/*
    Maelstrom: Open Source version of the classic game by Ambrosia Software
    Copyright (C) 1997-2011  Sam Lantinga

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    slouken@libsdl.org
*/

#include "Maelstrom_Globals.h"
#include "load.h"
#include "object.h"
#include "player.h"
#include "netplay.h"
#include "make.h"
#include "game.h"
#include "../screenlib/UIElement.h"

// Global variables set in this file...
GameInfo gGameInfo;
int	gScore;
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

// Local functions used in the game module of Maelstrom
static void DoGameOver(void);

/* ----------------------------------------------------------------- */
/* -- Start a new game */

void NewGame(void)
{
	/* Make sure we have a valid player list */
	if ( CheckPlayers() < 0 )
		return;

	/* Start up the random number generator */
	SeedRandom(gGameInfo.seed);

	/* Send a "NEW_GAME" packet onto the network */
	if ( gNumPlayers > 1 ) {
		if ( gOurPlayer == 0 ) {
			if ( Send_NewGame() < 0)
				return;
		} else {
			if ( Await_NewGame() < 0 )
				return;
		}
	}

	ui->ShowPanel(PANEL_GAME);
#ifdef USE_TOUCHCONTROL
	ui->ShowPanel("touchcontrol");
#endif

	/* Play the game, dammit! */
	while (gGameOn) {
		ui->Draw();

		/* -- In case we were faded out in DoBonus() */
		screen->FadeIn();

		/* Timing handling -- Delay the FRAME_DELAY */
		if ( ! gGameInfo.turbo ) {
			DelayFrame();
		}
	}
	
#ifdef USE_TOUCHCONTROL
	ui->HidePanel("touchcontrol");
#endif
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

	m_showingBonus = false;

	/* Initialize our panel variables */
	m_score = m_panel->GetElement<UIElement>("score");
	m_shield = m_panel->GetElement<UIElement>("shield");
	m_wave = m_panel->GetElement<UIElement>("wave");
	m_lives = m_panel->GetElement<UIElement>("lives");
	m_bonus = m_panel->GetElement<UIElement>("bonus");

	for (i = 0; (unsigned)i < SDL_arraysize(m_multiplier); ++i) {
		sprintf(name, "multiplier%d", 2+i);
		m_multiplier[i] = m_panel->GetElement<UIElement>(name);
	}

	m_autofire = m_panel->GetElement<UIElement>("autofire");
	m_airbrakes = m_panel->GetElement<UIElement>("airbrakes");
	m_lucky = m_panel->GetElement<UIElement>("lucky");
	m_triplefire = m_panel->GetElement<UIElement>("triplefire");
	m_longfire = m_panel->GetElement<UIElement>("longfire");

	m_multiplayerCaption = m_panel->GetElement<UIElement>("multiplayer_caption");
	m_multiplayerColor = m_panel->GetElement<UIElement>("multiplayer_color");
	m_fragsLabel = m_panel->GetElement<UIElement>("frags_label");
	m_frags = m_panel->GetElement<UIElement>("frags");

	return true;
}

void
GamePanelDelegate::OnShow()
{
	int i;

	/* Initialize some game variables */
	gGameOn = 1;
	gPaused = 0;
	gWave = gGameInfo.wave - 1;
	for ( i=gNumPlayers; i--; )
		gPlayers[i]->NewGame(gGameInfo.lives, gGameInfo.deathMatch);
	gLastStar = STAR_DELAY;
	gLastDrawn = 0L;
	gNumSprites = 0;

	if ( gNumPlayers > 1 ) {
		if (m_multiplayerCaption) {
			m_multiplayerCaption->Show();
		}
		if (m_multiplayerColor) {
			m_multiplayerColor->Show();
		}
	} else {
		if (m_multiplayerCaption) {
			m_multiplayerCaption->Hide();
		}
		if (m_multiplayerColor) {
			m_multiplayerColor->Hide();
		}
	}
	if ( gGameInfo.deathMatch ) {
		if (m_fragsLabel) {
			m_fragsLabel->Show();
		}
		if (m_frags) {
			m_frags->Show();
		}
	} else {
		if (m_fragsLabel) {
			m_fragsLabel->Hide();
		}
		if (m_frags) {
			m_frags->Hide();
		}
	}

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
	int i, j;

	/* -- Read in keyboard input for our ship */
	HandleEvents(0);

	if ( m_showingBonus ) {
		return;
	}

	/* -- Send Sync! signal to all players, and handle keyboard. */
	if ( SyncNetwork() < 0 ) {
		if ( gPaused & ~0x1 ) {
			/* One of the other players is minimized and may not
			   be able to send packets (iOS), so don't abort yet.
			*/
			return;
		}
		error("Game aborted!\n");
		gGameOn = 0;
		return;
	}
	OBJ_LOOP(i, gNumPlayers)
		gPlayers[i]->HandleKeys();

	if ( gPaused ) {
		return;
	}

	/* -- Play the boom sounds */
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

	/* -- Do all hit detection */
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
		if ( gGameInfo.deathMatch ) {
			OBJ_LOOP(i, gNumPlayers) {
				if ( i == j )	// Don't shoot ourselves. :)
					continue;
				(void) gPlayers[i]->HitBy(gPlayers[j]);
			}
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

	/* -- Move all of the sprites */
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

	DoHousekeeping();
}

void
GamePanelDelegate::OnDraw()
{
	int i;

	/* Draw the status frame */
	DrawStatus(false);

	if ( m_showingBonus ) {
		return;
	}

	/* -- Draw the star field */
	for ( i=0; i<MAX_STARS; ++i ) {
		screen->DrawPoint(gTheStars[i]->xCoord, 
			gTheStars[i]->yCoord, gTheStars[i]->color);
	}

	/* -- Blit all the sprites */
	OBJ_LOOP(i, gNumSprites)
		gSprites[i]->BlitSprite();
	OBJ_LOOP(i, gNumPlayers)
		gPlayers[i]->BlitSprite();

	/* -- Show the player dots */
	if ( gNumPlayers > 1 ) {
		OBJ_LOOP(i, gNumPlayers)
			gPlayers[i]->ShowDot();
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
	int i;
	char numbuf[128];

/* -- Draw the status display */

	if (first && gWave == 1) {
		OBJ_LOOP(i, gNumPlayers) {
			lastLife[i] = lastScores[i] = 0;
		}
	}

	if ( gNumPlayers > 1 ) {
		char caption[BUFSIZ];

		sprintf(caption, "You are player %d --- displaying player %d",
					gOurPlayer+1, gDisplayed+1);
		if (m_multiplayerCaption) {
			m_multiplayerCaption->SetText(caption);
		}

		/* Fill in the color by the frag count */
		if (m_multiplayerColor) {
			m_multiplayerColor->SetColor(TheShip->Color());
		}

		sprintf(numbuf, "%-3.1d", TheShip->GetFrags());
		if (m_frags) {
			m_frags->SetText(numbuf);
		}
	}

	int fact = ((SHIELD_WIDTH - 2) * TheShip->GetShieldLevel()) / MAX_SHIELD;
	if (m_shield) {
		m_shield->SetWidth(fact);
	}
	
	MultFactor = TheShip->GetBonusMult();
	for (i = 0; (unsigned)i < SDL_arraysize(m_multiplier); ++i) {
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
/* -- Do some housekeeping! */

void
GamePanelDelegate::DoHousekeeping()
{
	int i;

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

	/* -- Make sure someone is still playing... */
	bool PlayersLeft;
	for ( i=0; i < gNumPlayers; ++i ) {
		if ( gPlayers[i]->Kicking() ) {
			PlayersLeft = true;
			break;
		}
	}
	if ( !PlayersLeft ) {
		gGameOn = 0;
	}

}	/* -- DoHousekeeping */

/* ----------------------------------------------------------------- */
/* -- Do the bonus display */

void
GamePanelDelegate::DoBonus()
{
	UIPanel *panel;
	UIElement *image;
	UIElement *label;
	UIElement *bonus;
	UIElement *score;
	int i;
	char numbuf[128];

	/* -- Now do the bonus */
	sound->HaltSound();

	panel = ui->GetPanel(PANEL_BONUS);
	if (!panel) {
		return;
	}
	panel->HideAll();

	/* -- Set the wave completed message */
	label = panel->GetElement<UIElement>("wave");
	if (label) {
		sprintf(numbuf, "Wave %d completed.", gWave);
		label->SetText(numbuf);
		label->Show();
	}
	label = panel->GetElement<UIElement>("bonus_label");
	if (label) {
		label->Show();
	}
	label = panel->GetElement<UIElement>("score_label");
	if (label) {
		label->Show();
	}
		
	m_showingBonus = true;

	/* Fade out */
	screen->FadeOut();

	ui->ShowPanel(PANEL_BONUS);
	ui->Draw();

	/* Fade in */
	screen->FadeIn();
	while ( sound->Playing() )
		Delay(SOUND_DELAY);

	/* -- Count the score down */

	bonus = panel->GetElement<UIElement>("bonus");
	score = panel->GetElement<UIElement>("score");
	OBJ_LOOP(i, gNumPlayers) {
		if ( i != gOurPlayer ) {
			gPlayers[i]->MultBonus();
			continue;
		}

		if (OurShip->GetBonusMult() != 1) {
			if (bonus) {
				sprintf(numbuf, "%-5.1d", OurShip->GetBonus());
				bonus->SetText(numbuf);
				bonus->Show();
			}
			bonus = panel->GetElement<UIElement>("multiplied_bonus");

			OurShip->MultBonus();
			Delay(SOUND_DELAY);
			sound->PlaySound(gMultiplier, 5);

			sprintf(numbuf, "multiplier%d", OurShip->GetBonusMult());
			image = panel->GetElement<UIElement>(numbuf);
			if (image) {
				image->Show();
			}

			ui->Draw();
			Delay(60);
		}
	}
	Delay(SOUND_DELAY);
	sound->PlaySound(gFunk, 5);

	if (bonus) {
		sprintf(numbuf, "%-5.1d", OurShip->GetBonus());
		bonus->SetText(numbuf);
		bonus->Show();
	}
	if (score) {
		sprintf(numbuf, "%-5.1d", OurShip->GetScore());
		score->SetText(numbuf);
		score->Show();
	}
	ui->Draw();
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
	
			if (bonus) {
				sprintf(numbuf, "%-5.1d", OurShip->GetBonus());
				bonus->SetText(numbuf);
			}
			if (score) {
				sprintf(numbuf, "%-5.1d", OurShip->GetScore());
				score->SetText(numbuf);
			}

			ui->Draw();
		}
	}
	while ( sound->Playing() )
		Delay(SOUND_DELAY);
	HandleEvents(10);

	/* -- Draw the "next wave" message */
	label = panel->GetElement<UIElement>("next");
	if (label) {
		sprintf(numbuf, "Prepare for Wave %d...", gWave+1);
		label->SetText(numbuf);
		label->Show();
	}
	ui->Draw();
	HandleEvents(100);

	ui->HidePanel(PANEL_BONUS);

	m_showingBonus = false;

	/* Fade out and prepare for drawing the next wave */
	screen->FadeOut();
	screen->Clear();

}	/* -- DoBonus */

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

	if (gWave != (gGameInfo.wave - 1))
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
	UIElement *label;
	SDL_Event event;
	int which = -1, i;
	char handle[MAX_NAMELEN+1];
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
	if ( gGameInfo.deathMatch )
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
			label = panel->GetElement<UIElement>(name);
			if (!label) {
				continue;
			}
			if (gGameInfo.deathMatch) {
				sprintf(num1, "%7d", final[i].Score);
				sprintf(num2, "%3d", final[i].Frags);
				sprintf(buffer, "Player %d: %s Points, %s Frags", final[i].Player, num1, num2);
			} else {
				sprintf(num1, "%7d", final[i].Score);
				sprintf(buffer, "Player %d: %s Points", final[i].Player, num1);
			}
			label->SetText(buffer);
			label->Show();
		}
	}

	ui->ShowPanel(PANEL_GAMEOVER);

	/* -- Wait for the game over sound */
	while( sound->Playing() )
		Delay(SOUND_DELAY);

	/* -- See if they got a high score */
	gScore = OurShip->GetScore();
	LoadScores();
	for ( i = 0; i<10; ++i ) {
		if ( gScore > (int)hScores[i].score ) {
			which = i;
			break;
		}
	}

	/* -- They got a high score! */
	gLastHigh = which;

	if ((which != -1) && (gNumPlayers == 1) &&
			(gGameInfo.wave == 1) && (gGameInfo.lives == 3)) {
		sound->PlaySound(gBonusShot, 5);

		/* -- Let them enter their name */
		const char *text = NULL;
		label = panel->GetElement<UIElement>("name_label");
		if (label) {
			label->Show();
		}
		label = panel->GetElement<UIElement>("name");
		if (label) {
			text = label->GetText();
			label->Show();
		}
		ui->Draw();

		/* Get the previously used handle, if possible */
		if (text) {
			SDL_strlcpy(handle, text, sizeof(handle));
		} else {
			*handle = '\0';
		}
		chars_in_handle = SDL_strlen(handle);

		while ( screen->PollEvent(&event) ) /* Loop, flushing events */;
		screen->EnableTextInput();
		while ( label && !done ) {
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
						}
						break;
					default:
						break;
				}
			} else if ( event.type == SDL_TEXTINPUT ) {
				/* FIXME: No true UNICODE support in font */
				key = event.text.text[0];
				if (key >= ' ' && key <= '~') {
					if ( chars_in_handle < MAX_NAMELEN ) {
						sound->PlaySound(gShotSound, 5);
						handle[chars_in_handle++] = key;
						handle[chars_in_handle] = '\0';
					} else
						sound->PlaySound(gBonk, 5);
				}
			}
			if (label ) {
				label->SetText(handle);
			}
			ui->Draw();
		}
		screen->DisableTextInput();

		if (*handle) {
			for ( i = 8; i >= which ; --i ) {
				hScores[i + 1].score = hScores[i].score;
				hScores[i + 1].wave = hScores[i].wave;
				strcpy(hScores[i+1].name, hScores[i].name);
			}
			hScores[which].wave = gWave;
			hScores[which].score = OurShip->GetScore();
			strcpy(hScores[which].name, handle);
		}

		sound->HaltSound();
		sound->PlaySound(gGotPrize, 6);
		SaveScores();
	} else
	if ( gNumPlayers > 1 )	/* Let them watch their ranking */
		SDL_Delay(3000);

	while ( sound->Playing() )
		Delay(SOUND_DELAY);
	HandleEvents(0);

	ui->HidePanel(PANEL_GAMEOVER);

}	/* -- DoGameOver */

