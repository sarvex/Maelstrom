
#include "Maelstrom_Globals.h"
#include "object.h"
#include "player.h"
#include "netplay.h"
#include "make.h"
#include "load.h"
#include "game.h"
#include "../UIElementIcon.h"
#include "../UIElementLabel.h"


#ifdef MOVIE_SUPPORT
extern int gMovie;
static SDL_Rect gMovieRect;
int SelectMovieRect(void)
{
	SDL_Event event;
	SDL_Surface *saved;
	Uint32 white;
	int center_x, center_y;
	int width, height;

	/* Wait for initial button press */
	screen->ShowCursor();
	center_x = 0;
	center_y = 0;
	while ( ! center_x && ! center_y ) {
		screen->WaitEvent(&event);

		/* Check for escape key */
		if ( (event.type == SDL_KEYEVENT) && 
				(event.key.state == SDL_PRESSED) &&
				(event.key.keysym.sym == SDL_ESCAPE) ) {
			screen->HideCursor();
			return(0);
		}

		/* Wait for button press */
		if ( (event.type == SDL_MOUSEBUTTONEVENT) && 
				(event.button.state == SDL_PRESSED) ) {
			center_x = event.button.x;
			center_y = event.button.y;
			break;
		}
	}

	/* Save the screen */
	white = screen->MapRGB(0xFFFF, 0xFFFF, 0xFFFF);
	saved = screen->GrabArea(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	/* As the box moves... */
	width = height = 0;
	while ( 1 ) {
		win->GetEvent(&event);

		/* Check for escape key */
		if ( (event.type == SDL_KEYEVENT) && 
				(event.key.state == SDL_PRESSED) &&
				(event.key.keysym.sym == SDL_ESCAPE) ) {
			screen->QueueBlit(0, 0, saved, NOCLIP);
			screen->Update();
			screen->FreeImage(saved);
			win->HideCursor();
			return(0);
		}

		/* Check for ending button press */
		if ( event.type == ButtonPress ) {
			gMovieRect.x = center_x - width;
			gMovieRect.y = center_y - height;
			gMovieRect.w = 2*width;
			gMovieRect.h = 2*height;
			screen->QueueBlit(0, 0, saved, NOCLIP);
			screen->Update();
			screen->FreeImage(saved);
			win->HideCursor();
			return(1);
		}

		if ( event.type == MotionNotify ) {
			screen->QueueBlit(0, 0, saved, NOCLIP);
			screen->Update();
			width = abs(event.motion.x - center_x);
			height = abs(event.motion.y - center_y);
			screen->DrawRect(center_x-width, center_y-height,
						2*width, 2*height, white);
			screen->Update();
		}
	}
	/* NEVERREACHED */

}
#endif

// Global variables set in this file...
int	gGameOn;
int	gPaused;
int	gWave;
int	gBoomDelay;
int	gNextBoom;
int	gBoomPhase;
int	gNumRocks;
int	gLastStar;
int	gWhenDone;
int	gDisplayed;

int	gMultiplierShown;
int	gPrizeShown;
int	gBonusShown;
int	gWhenHoming;
int	gWhenGrav;
int	gWhenDamaged;
int	gWhenNova;
int	gShakeTime;
int	gFreezeTime;
Object *gEnemySprite;
int	gWhenEnemy;

// Local global variables;
static MFont *geneva;

// Local functions used in the game module of Maelstrom
static void DoGameOver(void);
static void DoBonus(void);

/* ----------------------------------------------------------------- */
/* -- Start a new game */

void NewGame(void)
{
	int i;

	/* Send a "NEW_GAME" packet onto the network */
	if ( gNumPlayers > 1 ) {
		if ( gOurPlayer == 0 ) {
			if ( Send_NewGame(&gStartLevel,&gStartLives,&gNoDelay)
									< 0)
				return;
		} else {
			if ( Await_NewGame(&gStartLevel,&gStartLives,&gNoDelay)
									< 0 )
				return;
		}
	}

	ui->ShowPanel(PANEL_GAME);

	/* Play the game, dammit! */
	while (gGameOn) {
		ui->Draw();

		/* Timing handling -- Delay the FRAME_DELAY */
		if ( ! gNoDelay ) {
			Uint32 ticks;
			while ( ((ticks=Ticks)-gLastDrawn) < FRAME_DELAY ) {
				SDL_Delay(1);
			}
			gLastDrawn = ticks;
		}
	}
	
	ui->HidePanel(PANEL_GAME);

/* -- Do the game over stuff */

	DoGameOver();

	ui->ShowPanel(PANEL_MAIN);
}	/* -- NewGame */

bool
GamePanelDelegate::OnLoad()
{
	int i;
	char name[32];

	/* Load the font and colors we use everywhere */
	geneva = fonts[GENEVA_9];

	/* Initialize our panel variables */
	m_score = m_panel->GetElement<UIElementLabel>("score");
	m_shield = m_panel->GetElement<UIElement>("shield");
	m_wave = m_panel->GetElement<UIElementLabel>("wave");
	m_lives = m_panel->GetElement<UIElementLabel>("lives");
	m_bonus = m_panel->GetElement<UIElementLabel>("bonus");

	for (i = 0; i < SDL_arraysize(m_multiplier); ++i) {
		sprintf(name, "multiplier%d", 2+i);
		m_multiplier[i] = m_panel->GetElement<UIElementIcon>(name);
	}

	m_autofire = m_panel->GetElement<UIElementIcon>("autofire");
	m_airbrakes = m_panel->GetElement<UIElementIcon>("airbrakes");
	m_lucky = m_panel->GetElement<UIElementIcon>("lucky");
	m_triplefire = m_panel->GetElement<UIElementIcon>("triplefire");
	m_longfire = m_panel->GetElement<UIElementIcon>("longfire");

	return true;
}

void
GamePanelDelegate::OnShow()
{
	int i;

	/* Initialize some game variables */
	gGameOn = 1;
	gPaused = 0;
	gWave = gStartLevel - 1;
	for ( i=gNumPlayers; i--; )
		gPlayers[i]->NewGame(gStartLives);
	gLastStar = STAR_DELAY;
	gLastDrawn = 0L;
	gNumSprites = 0;

	NextWave();
}

void
GamePanelDelegate::OnHide()
{
	/* -- Kill any existing sprites */
	while (gNumSprites > 0)
		delete gSprites[gNumSprites-1];

	sound->HaltSound();
}

void
GamePanelDelegate::OnTick()
{
	/* Don't do anything if we're paused */
	if ( gPaused ) {
		return;
	}

#ifdef MOVIE_SUPPORT
	if ( gMovie )
		screen->ScreenDump("MovieFrame", &gMovieRect);
#endif
	/* -- Maybe throw a multiplier up on the screen */
	if (gMultiplierShown && (--gMultiplierShown == 0) )
		MakeMultiplier();
	
	/* -- Maybe throw a prize(!) up on the screen */
	if (gPrizeShown && (--gPrizeShown == 0) )
		MakePrize();
	
	/* -- Maybe throw a bonus up on the screen */
	if (gBonusShown && (--gBonusShown == 0) )
		MakeBonus();

	/* -- Maybe make a nasty enemy fighter? */
	if (gWhenEnemy && (--gWhenEnemy == 0) )
		MakeEnemy();

	/* -- Maybe create a transcenfugal vortex */
	if (gWhenGrav && (--gWhenGrav == 0) )
		MakeGravity();
	
	/* -- Maybe create a recified space vehicle */
	if (gWhenDamaged && (--gWhenDamaged == 0) )
		MakeDamagedShip();
	
	/* -- Maybe create a autonominous tracking device */
	if (gWhenHoming && (--gWhenHoming == 0) )
		MakeHoming();
	
	/* -- Maybe make a supercranial destruction thang */
	if (gWhenNova && (--gWhenNova == 0) )
		MakeNova();

	/* -- Maybe create a new star ? */
	if ( --gLastStar == 0 ) {
		gLastStar = STAR_DELAY;
		SetStar(FastRandom(MAX_STARS));
	}
	
	/* -- Time for the next wave? */
	if (gNumRocks == 0) {
		if ( gWhenDone == 0 )
			gWhenDone = DEAD_DELAY;
		else if ( --gWhenDone == 0 )
			NextWave();
	}
}

void
GamePanelDelegate::OnDraw()
{
	int i, j, PlayersLeft;

	/* -- Draw the star field */
	for ( i=0; i<MAX_STARS; ++i ) {
		screen->DrawPoint(gTheStars[i]->xCoord, 
			gTheStars[i]->yCoord, gTheStars[i]->color);
	}

	/* Draw the status frame */
	DrawStatus(false);

	/* Read in keyboard input for our ship */
	HandleEvents(0);

	/* Send Sync! signal to all players, and handle keyboard. */
	if ( SyncNetwork() < 0 ) {
		error("Game aborted!\n");
		gGameOn = 0;
		return;
	}
	OBJ_LOOP(i, gNumPlayers)
		gPlayers[i]->HandleKeys();

	if ( gPaused > 0 )
		return;

	/* Play the boom sounds */
	if ( --gNextBoom == 0 ) {
		if ( gBoomPhase ) {
			sound->PlaySound(gBoom1, 0);
			gBoomPhase = 0;
		} else {
			sound->PlaySound(gBoom2, 0);
			gBoomPhase = 1;
		}
		gNextBoom = gBoomDelay;
	}

	/* Do all hit detection */
	OBJ_LOOP(j, gNumPlayers) {
		if ( ! gPlayers[j]->Alive() )
			continue;

		/* This loop looks funny because gNumSprites can change 
		   dynamically during the loop as sprites are killed/created.
		   This same logic is used whenever looping where sprites
		   might be destroyed.
		*/
		OBJ_LOOP(i, gNumSprites) {
			if ( gSprites[i]->HitBy(gPlayers[j]) < 0 ) {
				delete gSprites[i];
				gSprites[i] = gSprites[gNumSprites];
			}
		}
		OBJ_LOOP(i, gNumPlayers) {
			if ( i == j )	// Don't shoot ourselves. :)
				continue;
			(void) gPlayers[i]->HitBy(gPlayers[j]);
		}
	}
	if ( gEnemySprite ) {
		OBJ_LOOP(i, gNumPlayers) {
			if ( ! gPlayers[i]->Alive() )
				continue;
			(void) gPlayers[i]->HitBy(gEnemySprite);
		}
		OBJ_LOOP(i, gNumSprites) {
			if ( gSprites[i] == gEnemySprite )
				continue;
			if ( gSprites[i]->HitBy(gEnemySprite) < 0 ) {
				delete gSprites[i];
				gSprites[i] = gSprites[gNumSprites];
			}
		}
	}

	/* Handle all the shimmy and the shake. :-) */
	if ( gShakeTime && (gShakeTime-- > 0) ) {
		int shakeV;

		OBJ_LOOP(i, gNumPlayers) {
			shakeV = FastRandom(SHAKE_FACTOR);
			if ( ! gPlayers[i]->Alive() )
				continue;
			gPlayers[i]->Shake(FastRandom(SHAKE_FACTOR));
		}
		OBJ_LOOP(i, gNumSprites) {
			shakeV = FastRandom(SHAKE_FACTOR);
			gSprites[i]->Shake(FastRandom(SHAKE_FACTOR));
		}
	}

	/* Move all of the sprites */
	OBJ_LOOP(i, gNumPlayers)
		gPlayers[i]->Move(0);
	OBJ_LOOP(i, gNumSprites) {
		if ( gSprites[i]->Move(gFreezeTime) < 0 ) {
			delete gSprites[i];
			gSprites[i] = gSprites[gNumSprites];
		}
	}
	if ( gFreezeTime )
		--gFreezeTime;

	/* Now Blit them all again */
	OBJ_LOOP(i, gNumSprites)
		gSprites[i]->BlitSprite();
	OBJ_LOOP(i, gNumPlayers)
		gPlayers[i]->BlitSprite();

	/* Make sure someone is still playing... */
	for ( i=0, PlayersLeft=0; i < gNumPlayers; ++i ) {
		if ( gPlayers[i]->Kicking() )
			++PlayersLeft;
	}
	if ( gNumPlayers > 1 ) {
		OBJ_LOOP(i, gNumPlayers)
			gPlayers[i]->ShowDot();
	}
	if ( !PlayersLeft ) {
		gGameOn = 0;
	}
}

/* ----------------------------------------------------------------- */
/* -- Draw the status display */

void
GamePanelDelegate::DrawStatus(Bool first)
{
	static int lastScores[MAX_PLAYERS], lastLife[MAX_PLAYERS];
	int Score;
	int MultFactor;
	int i, x;
	char numbuf[128];

/* -- Draw the status display */

	if (first && gWave == 1) {
		OBJ_LOOP(i, gNumPlayers) {
			lastLife[i] = lastScores[i] = 0;
		}
	}

	/* Heh, DOOM style frag count */
	if ( gNumPlayers > 1 ) {
		x = 530;
		i = DrawText(x, gStatusLine+11, "Frags:", geneva,
				STYLE_BOLD, 30000>>8, 30000>>8, 0xFF);
		sprintf(numbuf, "%-3.1d", TheShip->GetFrags());
		DrawText(x+i+4, gStatusLine+11, numbuf, geneva, STYLE_BOLD, 0xFF, 0xFF, 0xFF);
	}

	if ( gNumPlayers > 1 ) {
		char caption[BUFSIZ];

		sprintf(caption, "You are player %d --- displaying player %d",
					gOurPlayer+1, gDisplayed+1);
		DrawText(SPRITES_WIDTH, 11, caption, geneva,
				STYLE_BOLD, 30000>>8, 30000>>8, 0xFF);

		/* Fill in the color by the frag count */
		screen->FillRect(518, gStatusLine+4, 4, 8, TheShip->Color());
	}

	int fact = ((SHIELD_WIDTH - 2) * TheShip->GetShieldLevel()) / MAX_SHIELD;
	if (m_shield) {
		m_shield->SetWidth(fact);
	}
	
	MultFactor = TheShip->GetBonusMult();
	for (i = 0; i < SDL_arraysize(m_multiplier); ++i) {
		if (!m_multiplier[i]) {
			continue;
		}
		if (MultFactor == 2+i) {
			m_multiplier[i]->Show();
		} else {
			m_multiplier[i]->Hide();
		}
	}

	if (m_autofire) {
		if ( TheShip->GetSpecial(MACHINE_GUNS) ) {
			m_autofire->Show();
		} else {
			m_autofire->Hide();
		}
	}
	if (m_airbrakes) {
		if ( TheShip->GetSpecial(AIR_BRAKES) ) {
			m_airbrakes->Show();
		} else {
			m_airbrakes->Hide();
		}
	}
	if (m_lucky) {
		if ( TheShip->GetSpecial(LUCKY_IRISH) ) {
			m_lucky->Show();
		} else {
			m_lucky->Hide();
		}
	}
	if (m_triplefire) {
		if ( TheShip->GetSpecial(TRIPLE_FIRE) ) {
			m_triplefire->Show();
		} else {
			m_triplefire->Hide();
		}
	}
	if (m_longfire) {
		if ( TheShip->GetSpecial(LONG_RANGE) ) {
			m_longfire->Show();
		} else {
			m_longfire->Hide();
		}
	}

	/* Check for everyone else's new lives */
	OBJ_LOOP(i, gNumPlayers) {
		Score = gPlayers[i]->GetScore();

		if ( i == gDisplayed && m_score ) {
			sprintf(numbuf, "%d", Score);
			m_score->SetText(numbuf);
		}

		if (lastScores[i] == Score)
			continue;

		/* -- See if they got a new life */
		lastScores[i] = Score;
		if ((Score - lastLife[i]) >= NEW_LIFE) {
			gPlayers[i]->IncrLives(1);
			lastLife[i] = (Score / NEW_LIFE) * NEW_LIFE;
			if ( i == gOurPlayer )
				sound->PlaySound(gNewLife, 5);
		}
	}

	if (m_wave) {
		sprintf(numbuf, "%d", gWave);
		m_wave->SetText(numbuf);
	}

	if (m_lives) {
		sprintf(numbuf, "%-3.1d", TheShip->GetLives());
		m_lives->SetText(numbuf);
	}

	if (m_bonus) {
		sprintf(numbuf, "%-7.1d", TheShip->GetBonus());
		m_bonus->SetText(numbuf);
	}

}	/* -- DrawStatus */


/* ----------------------------------------------------------------- */
/* -- Start the next wave! */

void
GamePanelDelegate::NextWave()
{
	int	index, x, y;
	int	NewRoids;
	short	temp;

	gEnemySprite = NULL;

	/* -- Initialize some variables */
	gDisplayed = gOurPlayer;
	gNumRocks = 0;
	gShakeTime = 0;
	gFreezeTime = 0;

	if (gWave != (gStartLevel - 1))
		DoBonus();

	gWave++;

	/* See about the Multiplier */
	if ( FastRandom(2) )
		gMultiplierShown = ((FastRandom(30) * 60)/FRAME_DELAY);
	else
		gMultiplierShown = 0;

	/* See about the Prize */
	if ( FastRandom(2) )
		gPrizeShown = ((FastRandom(30) * 60)/FRAME_DELAY);
	else
		gPrizeShown = 0;

	/* See about the Bonus */
	if ( FastRandom(2) )
		gBonusShown = ((FastRandom(30) * 60)/FRAME_DELAY);
	else
		gBonusShown = 0;

	/* See about the Gravity */
	if (FastRandom(10 + gWave) > 11)
		gWhenGrav = ((FastRandom(30) * 60)/FRAME_DELAY);
	else
		gWhenGrav = 0;

	/* See about the Nova */
	if (FastRandom(10 + gWave) > 13)
		gWhenNova = ((FastRandom(30) * 60)/FRAME_DELAY);
	else
		gWhenNova = 0;

	/* See about the Enemy */
	if (FastRandom(3) == 0)
		gWhenEnemy = ((FastRandom(30) * 60)/FRAME_DELAY);
	else
		gWhenEnemy = 0;

	/* See about the Damaged Ship */
	if (FastRandom(10) == 0)
		gWhenDamaged = ((FastRandom(60) * 60L)/FRAME_DELAY);
	else
		gWhenDamaged = 0;

	/* See about the Homing Mine */
	if (FastRandom(10 + gWave) > 12)
		gWhenHoming = ((FastRandom(60) * 60L)/FRAME_DELAY);
	else
		gWhenHoming = 0;

	temp = gWave / 4;
	if (temp < 1)
		temp = 1;

	NewRoids = FastRandom(temp) + (gWave / 5) + 3;

	/* -- Black the screen out and draw the wave */
	screen->Clear();

	/* -- Kill any existing sprites */
	while (gNumSprites > 0)
		delete gSprites[gNumSprites-1];

	/* -- Initialize some variables */
	gLastDrawn = 0L;
	gBoomDelay = (60/FRAME_DELAY);
	gNextBoom = gBoomDelay;
	gBoomPhase = 0;
	gWhenDone = 0;

	/* -- Create the ship's sprite */
	for ( index=gNumPlayers; index--; )
		gPlayers[index]->NewWave();
	DrawStatus(true);
	screen->Update();

	/* -- Create some asteroids */
	for (index = 0; index < NewRoids; index++) {
		int	randval;
	
		x = FastRandom(SCREEN_WIDTH) * SCALE_FACTOR;
		y = 0;
	
		randval = FastRandom(10);

		/* -- See what kind of asteroid to make */
		if (randval == 0)
			MakeSteelRoid(x, y);
		else
			MakeLargeRock(x, y);
	}

	screen->Fade();
}	/* -- NextWave */

/* ----------------------------------------------------------------- */
/* -- Do the game over display */

struct FinalScore {
	int Player;
	int Score;
	int Frags;
};

static int cmp_byscore(const void *A, const void *B)
{
	return(((struct FinalScore *)B)->Score-((struct FinalScore *)A)->Score);
}
static int cmp_byfrags(const void *A, const void *B)
{
	return(((struct FinalScore *)B)->Frags-((struct FinalScore *)A)->Frags);
}

static void DoGameOver(void)
{
	UIPanel *panel;
	UIElement *image;
	UIElementLabel *label;
	SDL_Event event;
	int which = -1, i;
	char handle[20];
	char key;
	int chars_in_handle = 0;
	Bool done = false;

	/* Get the final scoring */
	struct FinalScore *final = new struct FinalScore[gNumPlayers];
	for ( i=0; i<gNumPlayers; ++i ) {
		final[i].Player = i+1;
		final[i].Score = gPlayers[i]->GetScore();
		final[i].Frags = gPlayers[i]->GetFrags();
	}
	if ( gDeathMatch )
		qsort(final,gNumPlayers,sizeof(struct FinalScore),cmp_byfrags);
	else
		qsort(final,gNumPlayers,sizeof(struct FinalScore),cmp_byscore);

	panel = ui->GetPanel(PANEL_GAMEOVER);
	if (!panel) {
		return;
	}
	panel->HideAll();

	image = panel->GetElement<UIElement>("image");
	if (image) {
		image->Show();
	}

	/* Show the player ranking */
	if ( gNumPlayers > 1 ) {
		for ( i=0; i<gNumPlayers; ++i ) {
			char name[32];
			char buffer[BUFSIZ], num1[12], num2[12];

			sprintf(name, "rank%d", 1+i);
			label = panel->GetElement<UIElementLabel>(name);
			if (label) {
				sprintf(num1, "%7.1d", final[i].Score);
				sprintf(num2, "%3.1d", final[i].Frags);
				sprintf(buffer, "Player %d: %-.7s Points, %-.3s Frags", final[i].Player, num1, num2);
				label->SetText(buffer);
				label->Show();
			}
		}
	}

	ui->ShowPanel(PANEL_GAMEOVER);

	/* -- Wait for the game over sound */
	while( sound->Playing() )
		Delay(SOUND_DELAY);

	/* -- See if they got a high score */
	LoadScores();
	for ( i = 0; i<10; ++i ) {
		if ( OurShip->GetScore() > hScores[i].score ) {
			which = i;
			break;
		}
	}

	/* -- They got a high score! */
	gLastHigh = which;

	if ((which != -1) && (gStartLevel == 1) && (gStartLives == 3) &&
					(gNumPlayers == 1) && !gDeathMatch ) {
		sound->PlaySound(gBonusShot, 5);
		for ( i = 8; i >= which ; --i ) {
			hScores[i + 1].score = hScores[i].score;
			hScores[i + 1].wave = hScores[i].wave;
			strcpy(hScores[i+1].name, hScores[i].name);
		}

		/* -- Let them enter their name */
		chars_in_handle = 0;
		handle[0] = '\0';
		label = panel->GetElement<UIElementLabel>("name_label");
		if (label) {
			label->Show();
		}
		label = panel->GetElement<UIElementLabel>("name");
		if (label) {
			label->Show();
		}
		ui->Draw();

		while ( screen->PollEvent(&event) ) /* Loop, flushing events */;
		SDL_StartTextInput();
		while ( label && !done ) {
			bool updated = false;

			screen->WaitEvent(&event);

			if ( event.type == SDL_KEYUP ) {
				switch (event.key.keysym.sym) {
					case SDLK_RETURN:
						done = true;
						break;
					case SDLK_DELETE:
					case SDLK_BACKSPACE:
						if ( chars_in_handle ) {
							sound->PlaySound(gExplosionSound, 5);
							--chars_in_handle;
							handle[chars_in_handle] = '\0';
							label->SetText(handle);
						}
						break;
					default:
						break;
				}
			} else if ( event.type == SDL_TEXTINPUT ) {
				/* FIXME: No true UNICODE support in font */
				key = event.text.text[0];
				if (key >= ' ' && key <= '~') {
					if ( chars_in_handle < 15 ) {
						sound->PlaySound(gShotSound, 5);
						handle[chars_in_handle++] = key;
						handle[chars_in_handle] = '\0';
						label->SetText(handle);
					} else
						sound->PlaySound(gBonk, 5);
				}
			}
			ui->Draw();
		}
		SDL_StopTextInput();

		hScores[which].wave = gWave;
		hScores[which].score = OurShip->GetScore();
		strcpy(hScores[which].name, handle);

		sound->HaltSound();
		sound->PlaySound(gGotPrize, 6);
		if ( gNetScores )	// All time high!
			RegisterHighScore(hScores[which]);
		else
			SaveScores();
	} else
	if ( gNumPlayers > 1 )	/* Let them watch their ranking */
		SDL_Delay(3000);

	while ( sound->Playing() )
		Delay(SOUND_DELAY);
	HandleEvents(0);

	ui->HidePanel(PANEL_GAMEOVER);

}	/* -- DoGameOver */


/* ----------------------------------------------------------------- */
/* -- Do the bonus display */

static void DoBonus(void)
{
	int i, x, sw, xs, xt;
	char numbuf[128];

	//DrawStatus(false);
	screen->Update();

	/* -- Now do the bonus */
	sound->HaltSound();
	sound->PlaySound(gRiff, 6);

	/* Fade out */
	screen->Fade();

	/* -- Clear the screen */
	screen->Clear();
	

	/* -- Draw the wave completed message */
	sprintf(numbuf, "Wave %d completed.", gWave);
	sw = fontserv->TextWidth(numbuf, geneva, STYLE_BOLD);
	x = (SCREEN_WIDTH - sw) / 2;
	DrawText(x,  150, numbuf, geneva, STYLE_BOLD, 0xFF, 0xFF, 0x00);

	/* -- Draw the bonus */
	sw = fontserv->TextWidth("Bonus Score:     ", geneva, STYLE_BOLD);
	x = ((SCREEN_WIDTH - sw) / 2) - 20;
	DrawText(x, 200, "Bonus Score:     ", geneva, STYLE_BOLD,
						30000>>8, 30000>>8, 0xFF);
	xt = x+sw;

	/* -- Draw the score */
	sw = fontserv->TextWidth("Score:     ", geneva, STYLE_BOLD);
	x = ((SCREEN_WIDTH - sw) / 2) - 3;
	DrawText(x, 220, "Score:     ", geneva, STYLE_BOLD,
						30000>>8, 30000>>8, 0xFF);
	xs = x+sw;
	screen->Update();

	/* Fade in */
	screen->Fade();
	while ( sound->Playing() )
		Delay(SOUND_DELAY);

	/* -- Count the score down */
	x = xs;

	OBJ_LOOP(i, gNumPlayers) {
		if ( i != gOurPlayer ) {
			gPlayers[i]->MultBonus();
			continue;
		}

		if (OurShip->GetBonusMult() != 1) {
			SDL_Texture *sprite;

			sprintf(numbuf, "%-5.1d", OurShip->GetBonus());
			DrawText(x, 200, numbuf, geneva, STYLE_BOLD,
							0xFF, 0xFF, 0xFF);
			x += 75;
			OurShip->MultBonus();
			Delay(SOUND_DELAY);
			sound->PlaySound(gMultiplier, 5);
			sprite = gMult[OurShip->GetBonusMult()-2]->sprite[0];
			screen->QueueBlit(xs+34, 180, sprite);
			screen->Update();
			Delay(60);
		}
	}
	Delay(SOUND_DELAY);
	sound->PlaySound(gFunk, 5);

	sprintf(numbuf, "%-5.1d", OurShip->GetBonus());
	DrawText(x, 200, numbuf, geneva, STYLE_BOLD, 0xFF, 0xFF, 0xFF);
	sprintf(numbuf, "%-5.1d", OurShip->GetScore());
	DrawText(xt, 220, numbuf, geneva, STYLE_BOLD, 0xFF, 0xFF, 0xFF);
	screen->Update();
	Delay(60);

	/* -- Praise them or taunt them as the case may be */
	if (OurShip->GetBonus() == 0) {
		Delay(SOUND_DELAY);
		sound->PlaySound(gNoBonus, 5);
	}
	if (OurShip->GetBonus() > 10000) {
		Delay(SOUND_DELAY);
		sound->PlaySound(gPrettyGood, 5);
	}
	while ( sound->Playing() )
		Delay(SOUND_DELAY);

	/* -- Count the score down */
	OBJ_LOOP(i, gNumPlayers) {
		if ( i != gOurPlayer ) {
			while ( gPlayers[i]->GetBonus() > 500 ) {
				gPlayers[i]->IncrScore(500);
				gPlayers[i]->IncrBonus(-500);
			}
			continue;
		}

		while (OurShip->GetBonus() > 0) {
			while ( sound->Playing() )
				Delay(SOUND_DELAY);

			sound->PlaySound(gBonk, 5);
			if ( OurShip->GetBonus() >= 500 ) {
				OurShip->IncrScore(500);
				OurShip->IncrBonus(-500);
			} else {
				OurShip->IncrScore(OurShip->GetBonus());
				OurShip->IncrBonus(-OurShip->GetBonus());
			}
	
			sprintf(numbuf, "%-5.1d", OurShip->GetBonus());
			DrawText(x, 200, numbuf, geneva, STYLE_BOLD, 0xFF, 0xFF, 0xFF);
			sprintf(numbuf, "%-5.1d", OurShip->GetScore());
			DrawText(xt, 220, numbuf, geneva, STYLE_BOLD, 0xFF, 0xFF, 0xFF);

			//DrawStatus(false);
			screen->Update();
		}
	}
	while ( sound->Playing() )
		Delay(SOUND_DELAY);
	HandleEvents(10);

	/* -- Draw the "next wave" message */
	sprintf(numbuf, "Prepare for Wave %d...", gWave+1);
	sw = fontserv->TextWidth(numbuf, geneva, STYLE_BOLD);
	x = (SCREEN_WIDTH - sw)/2;
	DrawText(x, 259, numbuf, geneva, STYLE_BOLD, 0xFF, 0xFF, 0x00);
	screen->Update();
	HandleEvents(100);

	screen->Fade();
}	/* -- DoBonus */

