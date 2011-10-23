#ifdef COMPUTE_VELTABLE
#include <math.h>
#endif
#include <signal.h>
#include <stdlib.h>

#include "Maelstrom_Globals.h"
#include "load.h"
#include "colortable.h"
#include "fastrand.h"
#include "UIElements.h"
#include "screenlib/UIElement.h"


// Global variables set in this file...
Sound    *sound = NULL;
FontServ *fontserv = NULL;
MFont    *fonts[NUM_FONTS];
FrameBuf *screen = NULL;
UIManager *ui = NULL;

Sint32	gLastHigh;
Uint32	gLastDrawn;
int     gNumSprites;
Rect	gScrnRect;
SDL_Rect gClipRect;
int	gStatusLine;
int	gTop, gLeft, gBottom, gRight;
MPoint	gShotOrigins[SHIP_FRAMES];
MPoint	gThrustOrigins[SHIP_FRAMES];
MPoint	gVelocityTable[SHIP_FRAMES];
StarPtr	gTheStars[MAX_STARS];
Uint32	gStarColors[20];

/* -- The blit'ers we use */
BlitPtr	gRock1R, gRock2R, gRock3R, gDamagedShip;
BlitPtr	gRock1L, gRock2L, gRock3L, gShipExplosion;
BlitPtr	gPlayerShip, gExplosion, gNova, gEnemyShip, gEnemyShip2;
BlitPtr	gMult[4], gSteelRoidL;
BlitPtr	gSteelRoidR, gPrize, gBonusBlit, gPointBlit;
BlitPtr	gVortexBlit, gMineBlitL, gMineBlitR, gShieldBlit;
BlitPtr	gThrust1, gThrust2, gShrapnel1, gShrapnel2;

/* -- The prize CICN's */

SDL_Texture *gAutoFireIcon, *gAirBrakesIcon, *gMult2Icon, *gMult3Icon;
SDL_Texture *gMult4Icon, *gMult5Icon, *gLuckOfTheIrishIcon, *gLongFireIcon;
SDL_Texture *gTripleFireIcon, *gKeyIcon, *gShieldIcon;

// Local functions used in this file.
static void DrawLoadBar(int stage);
static int InitSprites(void);
static int LoadBlits(Mac_Resource *spriteres);
static int LoadCICNS(void);
static void BackwardsSprite(BlitPtr *theBlit, BlitPtr oldBlit);
static int LoadSprite(Mac_Resource *spriteres,
			BlitPtr *theBlit, int baseID, int numFrames);
static int LoadSmallSprite(Mac_Resource *spriteres,
			BlitPtr *theBlit, int baseID, int numFrames);


/* Put up an Ambrosia Software splash screen */
void DoSplash(void)
{
}

/* ----------------------------------------------------------------- */
/* -- Draw a loading status bar */

#define	MAX_BAR	26

static void DrawLoadBar(int stage)
{
	UIElement *progress;
	int fact;
	const int FULL_WIDTH = 196;

	progress = ui->GetCurrentPanel()->GetElement("progress");
	if (progress) {
		fact = (FULL_WIDTH * stage) / MAX_BAR;
		progress->SetWidth(fact);
	}
	ui->Draw();
}	/* -- DrawLoadBar */


/* ----------------------------------------------------------------- */
/* -- Set a star */

void SetStar(int which)
{
	gTheStars[which]->xCoord = gClipRect.x+FastRandom(gClipRect.w);
	gTheStars[which]->yCoord = gClipRect.y+FastRandom(gClipRect.h);
	gTheStars[which]->color = gStarColors[FastRandom(20)];
}	/* -- SetStar */

/* ----------------------------------------------------------------- */
/* -- Initialize the stars */

static void InitStars(void)
{
	int index;

	/* Map star pixel colors to new colormap */
	gStarColors[0] = screen->MapRGB(colors[gGammaCorrect][0xEB].r,
					colors[gGammaCorrect][0xEB].g,
					colors[gGammaCorrect][0xEB].b);
	gStarColors[1] = screen->MapRGB(colors[gGammaCorrect][0xEC].r,
					colors[gGammaCorrect][0xEC].g,
					colors[gGammaCorrect][0xEC].b);
	gStarColors[2] = screen->MapRGB(colors[gGammaCorrect][0xED].r,
					colors[gGammaCorrect][0xED].g,
					colors[gGammaCorrect][0xED].b);
	gStarColors[3] = screen->MapRGB(colors[gGammaCorrect][0xEE].r,
					colors[gGammaCorrect][0xEE].g,
					colors[gGammaCorrect][0xEE].b);
	gStarColors[4] = screen->MapRGB(colors[gGammaCorrect][0xEF].r,
					colors[gGammaCorrect][0xEF].g,
					colors[gGammaCorrect][0xEF].b);
	gStarColors[5] = screen->MapRGB(colors[gGammaCorrect][0xD9].r,
					colors[gGammaCorrect][0xD9].g,
					colors[gGammaCorrect][0xD9].b);
	gStarColors[6] = screen->MapRGB(colors[gGammaCorrect][0xDA].r,
					colors[gGammaCorrect][0xDA].g,
					colors[gGammaCorrect][0xDA].b);
	gStarColors[7] = screen->MapRGB(colors[gGammaCorrect][0xDB].r,
					colors[gGammaCorrect][0xDB].g,
					colors[gGammaCorrect][0xDB].b);
	gStarColors[8] = screen->MapRGB(colors[gGammaCorrect][0xDC].r,
					colors[gGammaCorrect][0xDC].g,
					colors[gGammaCorrect][0xDC].b);
	gStarColors[9] = screen->MapRGB(colors[gGammaCorrect][0xDD].r,
					colors[gGammaCorrect][0xDD].g,
					colors[gGammaCorrect][0xDD].b);
	gStarColors[10] = screen->MapRGB(colors[gGammaCorrect][0xE4].r,
					colors[gGammaCorrect][0xE4].g,
					colors[gGammaCorrect][0xE4].b);
	gStarColors[11] = screen->MapRGB(colors[gGammaCorrect][0xE5].r,
					colors[gGammaCorrect][0xE5].g,
					colors[gGammaCorrect][0xE5].b);
	gStarColors[12] = screen->MapRGB(colors[gGammaCorrect][0xE6].r,
					colors[gGammaCorrect][0xE6].g,
					colors[gGammaCorrect][0xE6].b);
	gStarColors[13] = screen->MapRGB(colors[gGammaCorrect][0xE7].r,
					colors[gGammaCorrect][0xE7].g,
					colors[gGammaCorrect][0xE7].b);
	gStarColors[14] = screen->MapRGB(colors[gGammaCorrect][0xE8].r,
					colors[gGammaCorrect][0xE8].g,
					colors[gGammaCorrect][0xE8].b);
	gStarColors[15] = screen->MapRGB(colors[gGammaCorrect][0xF7].r,
					colors[gGammaCorrect][0xF7].g,
					colors[gGammaCorrect][0xF7].b);
	gStarColors[16] = screen->MapRGB(colors[gGammaCorrect][0xF8].r,
					colors[gGammaCorrect][0xF8].g,
					colors[gGammaCorrect][0xF8].b);
	gStarColors[17] = screen->MapRGB(colors[gGammaCorrect][0xF9].r,
					colors[gGammaCorrect][0xF9].g,
					colors[gGammaCorrect][0xF9].b);
	gStarColors[18] = screen->MapRGB(colors[gGammaCorrect][0xFA].r,
					colors[gGammaCorrect][0xFA].g,
					colors[gGammaCorrect][0xFA].b);
	gStarColors[19] = screen->MapRGB(colors[gGammaCorrect][0xFB].r,
					colors[gGammaCorrect][0xFB].g,
					colors[gGammaCorrect][0xFB].b);

	for (index = 0; index < MAX_STARS; index++) {
		gTheStars[index] = new Star;
		gTheStars[index]->color = 0L;
		SetStar(index);
	}
}	/* -- InitStars */


/* ----------------------------------------------------------------- */
/* -- Build the ship's velocity table */

static void InitShots(void)
{
	int xx = 30;

	/* Load the shot images */
	gPlayerShot = screen->LoadImage(SHOT_SIZE,SHOT_SIZE,gPlayerShotColors);
	gEnemyShot = screen->LoadImage(SHOT_SIZE, SHOT_SIZE, gEnemyShotColors);

	/* Now setup the shot origin table */

	gShotOrigins[0].h = 15 * SCALE_FACTOR;
	gShotOrigins[0].v = 12 * SCALE_FACTOR;

	gShotOrigins[1].h = 16 * SCALE_FACTOR;
	gShotOrigins[1].v = 12 * SCALE_FACTOR;

	gShotOrigins[2].h = 18 * SCALE_FACTOR;
	gShotOrigins[2].v = 12 * SCALE_FACTOR;

	gShotOrigins[3].h = 21 * SCALE_FACTOR;
	gShotOrigins[3].v = 12 * SCALE_FACTOR;

	gShotOrigins[4].h = xx * SCALE_FACTOR;
	gShotOrigins[4].v = xx * SCALE_FACTOR;

	gShotOrigins[5].h = xx * SCALE_FACTOR;
	gShotOrigins[5].v = xx * SCALE_FACTOR;

	gShotOrigins[6].h = xx * SCALE_FACTOR;
	gShotOrigins[6].v = xx * SCALE_FACTOR;

	gShotOrigins[7].h = xx * SCALE_FACTOR;
	gShotOrigins[7].v = xx * SCALE_FACTOR;

	gShotOrigins[8].h = xx * SCALE_FACTOR;
	gShotOrigins[8].v = xx * SCALE_FACTOR;

	gShotOrigins[9].h = xx * SCALE_FACTOR;
	gShotOrigins[9].v = xx * SCALE_FACTOR;

	gShotOrigins[10].h = xx * SCALE_FACTOR;
	gShotOrigins[10].v = xx * SCALE_FACTOR;

	gShotOrigins[11].h = xx * SCALE_FACTOR;
	gShotOrigins[11].v = xx * SCALE_FACTOR;

	gShotOrigins[12].h = xx * SCALE_FACTOR;
	gShotOrigins[12].v = xx * SCALE_FACTOR;

	gShotOrigins[13].h = xx * SCALE_FACTOR;
	gShotOrigins[13].v = xx * SCALE_FACTOR;

	gShotOrigins[14].h = xx * SCALE_FACTOR;
	gShotOrigins[14].v = xx * SCALE_FACTOR;

	gShotOrigins[15].h = xx * SCALE_FACTOR;
	gShotOrigins[15].v = xx * SCALE_FACTOR;

	gShotOrigins[16].h = xx * SCALE_FACTOR;
	gShotOrigins[16].v = xx * SCALE_FACTOR;

	gShotOrigins[17].h = xx * SCALE_FACTOR;
	gShotOrigins[17].v = xx * SCALE_FACTOR;

	gShotOrigins[18].h = xx * SCALE_FACTOR;
	gShotOrigins[18].v = xx * SCALE_FACTOR;

	gShotOrigins[19].h = xx * SCALE_FACTOR;
	gShotOrigins[19].v = xx * SCALE_FACTOR;

	gShotOrigins[20].h = xx * SCALE_FACTOR;
	gShotOrigins[20].v = xx * SCALE_FACTOR;

	gShotOrigins[21].h = xx * SCALE_FACTOR;
	gShotOrigins[21].v = xx * SCALE_FACTOR;

	gShotOrigins[22].h = xx * SCALE_FACTOR;
	gShotOrigins[22].v = xx * SCALE_FACTOR;

	gShotOrigins[23].h = xx * SCALE_FACTOR;
	gShotOrigins[23].v = xx * SCALE_FACTOR;

	gShotOrigins[24].h = xx * SCALE_FACTOR;
	gShotOrigins[24].v = xx * SCALE_FACTOR;

	gShotOrigins[25].h = xx * SCALE_FACTOR;
	gShotOrigins[25].v = xx * SCALE_FACTOR;

	gShotOrigins[26].h = xx * SCALE_FACTOR;
	gShotOrigins[26].v = xx * SCALE_FACTOR;

	gShotOrigins[27].h = xx * SCALE_FACTOR;
	gShotOrigins[27].v = xx * SCALE_FACTOR;

	gShotOrigins[28].h = xx * SCALE_FACTOR;
	gShotOrigins[28].v = xx * SCALE_FACTOR;

	gShotOrigins[29].h = xx * SCALE_FACTOR;
	gShotOrigins[29].v = xx * SCALE_FACTOR;

	gShotOrigins[30].h = xx * SCALE_FACTOR;
	gShotOrigins[30].v = xx * SCALE_FACTOR;

	gShotOrigins[31].h = xx * SCALE_FACTOR;
	gShotOrigins[31].v = xx * SCALE_FACTOR;

	gShotOrigins[32].h = xx * SCALE_FACTOR;
	gShotOrigins[32].v = xx * SCALE_FACTOR;

	gShotOrigins[33].h = xx * SCALE_FACTOR;
	gShotOrigins[33].v = xx * SCALE_FACTOR;

	gShotOrigins[34].h = xx * SCALE_FACTOR;
	gShotOrigins[34].v = xx * SCALE_FACTOR;

	gShotOrigins[35].h = xx * SCALE_FACTOR;
	gShotOrigins[35].v = xx * SCALE_FACTOR;

	gShotOrigins[36].h = xx * SCALE_FACTOR;
	gShotOrigins[36].v = xx * SCALE_FACTOR;

	gShotOrigins[37].h = xx * SCALE_FACTOR;
	gShotOrigins[37].v = xx * SCALE_FACTOR;

	gShotOrigins[38].h = xx * SCALE_FACTOR;
	gShotOrigins[38].v = xx * SCALE_FACTOR;

	gShotOrigins[39].h = xx * SCALE_FACTOR;
	gShotOrigins[39].v = xx * SCALE_FACTOR;

	gShotOrigins[40].h = xx * SCALE_FACTOR;
	gShotOrigins[40].v = xx * SCALE_FACTOR;

	gShotOrigins[41].h = xx * SCALE_FACTOR;
	gShotOrigins[41].v = xx * SCALE_FACTOR;

	gShotOrigins[42].h = xx * SCALE_FACTOR;
	gShotOrigins[42].v = xx * SCALE_FACTOR;

	gShotOrigins[43].h = xx * SCALE_FACTOR;
	gShotOrigins[43].v = xx * SCALE_FACTOR;

	gShotOrigins[44].h = xx * SCALE_FACTOR;
	gShotOrigins[44].v = xx * SCALE_FACTOR;

	gShotOrigins[45].h = xx * SCALE_FACTOR;
	gShotOrigins[45].v = xx * SCALE_FACTOR;

	gShotOrigins[46].h = xx * SCALE_FACTOR;
	gShotOrigins[46].v = xx * SCALE_FACTOR;

	gShotOrigins[47].h = xx * SCALE_FACTOR;
	gShotOrigins[47].v = xx * SCALE_FACTOR;

/* -- Now setup the thruster origin table */

	gThrustOrigins[0].h = 8 * SCALE_FACTOR;
	gThrustOrigins[0].v = 22 * SCALE_FACTOR;

	gThrustOrigins[1].h = 6 * SCALE_FACTOR;
	gThrustOrigins[1].v = 22 * SCALE_FACTOR;

	gThrustOrigins[2].h = 4 * SCALE_FACTOR;
	gThrustOrigins[2].v = 21 * SCALE_FACTOR;

	gThrustOrigins[3].h = 1 * SCALE_FACTOR;
	gThrustOrigins[3].v = 20 * SCALE_FACTOR;

	gThrustOrigins[4].h = 0 * SCALE_FACTOR;
	gThrustOrigins[4].v = 19 * SCALE_FACTOR;

	gThrustOrigins[5].h = -1 * SCALE_FACTOR;
	gThrustOrigins[5].v = 19 * SCALE_FACTOR;

	gThrustOrigins[6].h = -3 * SCALE_FACTOR;
	gThrustOrigins[6].v = 16 * SCALE_FACTOR;

	gThrustOrigins[7].h = -5 * SCALE_FACTOR;
	gThrustOrigins[7].v = 15 * SCALE_FACTOR;

	gThrustOrigins[8].h = -6 * SCALE_FACTOR;
	gThrustOrigins[8].v = 13 * SCALE_FACTOR;

	gThrustOrigins[9].h = -9 * SCALE_FACTOR;
	gThrustOrigins[9].v = 11 * SCALE_FACTOR;

	gThrustOrigins[10].h = -10 * SCALE_FACTOR;
	gThrustOrigins[10].v = 10 * SCALE_FACTOR;

	gThrustOrigins[11].h = -11 * SCALE_FACTOR;
	gThrustOrigins[11].v = 7 * SCALE_FACTOR;

	gThrustOrigins[12].h = -9 * SCALE_FACTOR;
	gThrustOrigins[12].v = 7 * SCALE_FACTOR;

	gThrustOrigins[13].h = -9 * SCALE_FACTOR;
	gThrustOrigins[13].v = 4 * SCALE_FACTOR;

	gThrustOrigins[14].h = -7 * SCALE_FACTOR;
	gThrustOrigins[14].v = 2 * SCALE_FACTOR;

	gThrustOrigins[15].h = -6 * SCALE_FACTOR;
	gThrustOrigins[15].v = 0 * SCALE_FACTOR;

	gThrustOrigins[16].h = -9 * SCALE_FACTOR;
	gThrustOrigins[16].v = 1 * SCALE_FACTOR;

	gThrustOrigins[17].h = -3 * SCALE_FACTOR;
	gThrustOrigins[17].v = -3 * SCALE_FACTOR;

	gThrustOrigins[18].h = -1 * SCALE_FACTOR;
	gThrustOrigins[18].v = -2 * SCALE_FACTOR;

	gThrustOrigins[19].h = 0 * SCALE_FACTOR;
	gThrustOrigins[19].v = -4 * SCALE_FACTOR;

	gThrustOrigins[20].h = 4 * SCALE_FACTOR;
	gThrustOrigins[20].v = -6 * SCALE_FACTOR;

	gThrustOrigins[21].h = 5 * SCALE_FACTOR;
	gThrustOrigins[21].v = -8 * SCALE_FACTOR;

	gThrustOrigins[22].h = 5 * SCALE_FACTOR;
	gThrustOrigins[22].v = -6 * SCALE_FACTOR;

	gThrustOrigins[23].h = 8 * SCALE_FACTOR;
	gThrustOrigins[23].v = -7 * SCALE_FACTOR;

	gThrustOrigins[24].h = 9 * SCALE_FACTOR;
	gThrustOrigins[24].v = -7 * SCALE_FACTOR;

	gThrustOrigins[25].h = 12 * SCALE_FACTOR;
	gThrustOrigins[25].v = -6 * SCALE_FACTOR;

	gThrustOrigins[26].h = 13 * SCALE_FACTOR;
	gThrustOrigins[26].v = -6 * SCALE_FACTOR;

	gThrustOrigins[27].h = 15 * SCALE_FACTOR;
	gThrustOrigins[27].v = -7 * SCALE_FACTOR;

	gThrustOrigins[28].h = 17 * SCALE_FACTOR;
	gThrustOrigins[28].v = -6 * SCALE_FACTOR;

	gThrustOrigins[29].h = 18 * SCALE_FACTOR;
	gThrustOrigins[29].v = -4 * SCALE_FACTOR;

	gThrustOrigins[30].h = 20 * SCALE_FACTOR;
	gThrustOrigins[30].v = -2 * SCALE_FACTOR;

	gThrustOrigins[31].h = 19 * SCALE_FACTOR;
	gThrustOrigins[31].v = -1 * SCALE_FACTOR;

	gThrustOrigins[32].h = 21 * SCALE_FACTOR;
	gThrustOrigins[32].v = 0 * SCALE_FACTOR;

	gThrustOrigins[33].h = 22 * SCALE_FACTOR;
	gThrustOrigins[33].v = 2 * SCALE_FACTOR;

	gThrustOrigins[34].h = 24 * SCALE_FACTOR;
	gThrustOrigins[34].v = 3 * SCALE_FACTOR;

	gThrustOrigins[35].h = 25 * SCALE_FACTOR;
	gThrustOrigins[35].v = 5 * SCALE_FACTOR;

	gThrustOrigins[36].h = 26 * SCALE_FACTOR;
	gThrustOrigins[36].v = 7 * SCALE_FACTOR;

	gThrustOrigins[37].h = 25 * SCALE_FACTOR;
	gThrustOrigins[37].v = 7 * SCALE_FACTOR;

	gThrustOrigins[38].h = 24 * SCALE_FACTOR;
	gThrustOrigins[38].v = 10 * SCALE_FACTOR;

	gThrustOrigins[39].h = 23 * SCALE_FACTOR;
	gThrustOrigins[39].v = 11 * SCALE_FACTOR;

	gThrustOrigins[40].h = 23 * SCALE_FACTOR;
	gThrustOrigins[40].v = 12 * SCALE_FACTOR;

	gThrustOrigins[41].h = 20 * SCALE_FACTOR;
	gThrustOrigins[41].v = 14 * SCALE_FACTOR;

	gThrustOrigins[42].h = 20 * SCALE_FACTOR;
	gThrustOrigins[42].v = 16 * SCALE_FACTOR;

	gThrustOrigins[43].h = 18 * SCALE_FACTOR;
	gThrustOrigins[43].v = 18 * SCALE_FACTOR;

	gThrustOrigins[44].h = 15 * SCALE_FACTOR;
	gThrustOrigins[44].v = 18 * SCALE_FACTOR;

	gThrustOrigins[45].h = 15 * SCALE_FACTOR;
	gThrustOrigins[45].v = 20 * SCALE_FACTOR;

	gThrustOrigins[46].h = 12 * SCALE_FACTOR;
	gThrustOrigins[46].v = 21 * SCALE_FACTOR;

	gThrustOrigins[47].h = 9 * SCALE_FACTOR;
	gThrustOrigins[47].v = 22 * SCALE_FACTOR;

}	/* -- InitShots */


/* ----------------------------------------------------------------- */
/* -- Build the ship's velocity table */

static void BuildVelocityTable(void)
{
#ifdef COMPUTE_VELTABLE
	/* Calculate the appropriate values */
	int	index;
	double	factor;
	double	ss;

	ss = SHIP_FRAMES;
	factor = (360.0 / ss);

	for (index = 0; index < SHIP_FRAMES; index++) {
		ss = index;
		ss = -(((ss * factor) * PI) / 180.0);
		gVelocityTable[index].h = (int)(sin(ss) * -8.0);
		gVelocityTable[index].v = (int)(cos(ss) * -8.0);
#ifdef PRINT_TABLE
		printf("\tgVelocityTable[%d].h = %d;\n", index,
						gVelocityTable[index].h);
		printf("\tgVelocityTable[%d].v = %d;\n", index,
						gVelocityTable[index].v);
#endif
	}
#else
	/* Because PI, sin() and cos() return _slightly_ different
	   values across architectures, we need to precompute our
	   velocity table -- make it standard across compilations. :)
	*/
	gVelocityTable[0].h = 0;
	gVelocityTable[0].v = -8;
	gVelocityTable[1].h = 1;
	gVelocityTable[1].v = -7;
	gVelocityTable[2].h = 2;
	gVelocityTable[2].v = -7;
	gVelocityTable[3].h = 3;
	gVelocityTable[3].v = -7;
	gVelocityTable[4].h = 4;
	gVelocityTable[4].v = -6;
	gVelocityTable[5].h = 4;
	gVelocityTable[5].v = -6;
	gVelocityTable[6].h = 5;
	gVelocityTable[6].v = -5;
	gVelocityTable[7].h = 6;
	gVelocityTable[7].v = -4;
	gVelocityTable[8].h = 6;
	gVelocityTable[8].v = -4;
	gVelocityTable[9].h = 7;
	gVelocityTable[9].v = -3;
	gVelocityTable[10].h = 7;
	gVelocityTable[10].v = -2;
	gVelocityTable[11].h = 7;
	gVelocityTable[11].v = -1;
	gVelocityTable[12].h = 8;
	gVelocityTable[12].v = 0;
	gVelocityTable[13].h = 7;
	gVelocityTable[13].v = 1;
	gVelocityTable[14].h = 7;
	gVelocityTable[14].v = 2;
	gVelocityTable[15].h = 7;
	gVelocityTable[15].v = 3;
	gVelocityTable[16].h = 6;
	gVelocityTable[16].v = 3;
	gVelocityTable[17].h = 6;
	gVelocityTable[17].v = 4;
	gVelocityTable[18].h = 5;
	gVelocityTable[18].v = 5;
	gVelocityTable[19].h = 4;
	gVelocityTable[19].v = 6;
	gVelocityTable[20].h = 3;
	gVelocityTable[20].v = 6;
	gVelocityTable[21].h = 3;
	gVelocityTable[21].v = 7;
	gVelocityTable[22].h = 2;
	gVelocityTable[22].v = 7;
	gVelocityTable[23].h = 1;
	gVelocityTable[23].v = 7;
	gVelocityTable[24].h = 0;
	gVelocityTable[24].v = 8;
	gVelocityTable[25].h = -1;
	gVelocityTable[25].v = 7;
	gVelocityTable[26].h = -2;
	gVelocityTable[26].v = 7;
	gVelocityTable[27].h = -3;
	gVelocityTable[27].v = 7;
	gVelocityTable[28].h = -4;
	gVelocityTable[28].v = 6;
	gVelocityTable[29].h = -4;
	gVelocityTable[29].v = 6;
	gVelocityTable[30].h = -5;
	gVelocityTable[30].v = 5;
	gVelocityTable[31].h = -6;
	gVelocityTable[31].v = 4;
	gVelocityTable[32].h = -6;
	gVelocityTable[32].v = 4;
	gVelocityTable[33].h = -7;
	gVelocityTable[33].v = 3;
	gVelocityTable[34].h = -7;
	gVelocityTable[34].v = 2;
	gVelocityTable[35].h = -7;
	gVelocityTable[35].v = 1;
	gVelocityTable[36].h = -8;
	gVelocityTable[36].v = 0;
	gVelocityTable[37].h = -7;
	gVelocityTable[37].v = -1;
	gVelocityTable[38].h = -7;
	gVelocityTable[38].v = -2;
	gVelocityTable[39].h = -7;
	gVelocityTable[39].v = -3;
	gVelocityTable[40].h = -6;
	gVelocityTable[40].v = -4;
	gVelocityTable[41].h = -6;
	gVelocityTable[41].v = -4;
	gVelocityTable[42].h = -5;
	gVelocityTable[42].v = -5;
	gVelocityTable[43].h = -4;
	gVelocityTable[43].v = -6;
	gVelocityTable[44].h = -4;
	gVelocityTable[44].v = -6;
	gVelocityTable[45].h = -3;
	gVelocityTable[45].v = -7;
	gVelocityTable[46].h = -2;
	gVelocityTable[46].v = -7;
	gVelocityTable[47].h = -1;
	gVelocityTable[47].v = -7;
#endif /* COMPUTE_VELTABLE */
}	/* -- BuildVelocityTable */


/* This function needs to be able to properly clean up from any stage
   of the program at any time, including itself if interrupted during
   cleanup.  *sigh*  reentrant multi-threading can be a pain. :)
*/
void CleanUp(void)
{
	int i;

	HaltLogic();
	if ( ui ) {
		delete ui;
		ui = NULL;
	}
	if ( fontserv ) {
		/* This will free the allocated fonts */
		delete fontserv;
		fontserv = NULL;
	}
	if ( screen ) {
		delete screen;
		screen = NULL;
	}
	if ( sound ) {
		delete sound;
		sound = NULL;
	}
	SaveControls();
	PHYSFS_deinit();
	SDL_Quit();
}

/* ----------------------------------------------------------------- */
/* -- Perform some initializations and report failure if we choke */
int DoInitializations(Uint32 window_flags, Uint32 render_flags)
{
	UIPanel *panel;
	Uint32 start;
	int i;
	SDL_Surface *icon;

	Uint32 init_flags = (SDL_INIT_VIDEO|SDL_INIT_AUDIO);
#ifdef SDL_INIT_JOYSTICK
	init_flags |= SDL_INIT_JOYSTICK;
#endif
	if ( SDL_Init(init_flags) < 0 ) {
		init_flags &= ~SDL_INIT_JOYSTICK;
		if ( SDL_Init(init_flags) < 0 ) {
			error("Couldn't initialize SDL: %s\n", SDL_GetError());
			return(-1);
		}
	}

	// -- Initialize some variables
	gLastHigh = -1;

	// -- Create our scores file
	LoadScores();

#ifdef SDL_INIT_JOYSTICK
	/* Initialize the first joystick */
	if ( SDL_NumJoysticks() > 0 ) {
		if ( SDL_JoystickOpen(0) == NULL ) {
			error("Warning: Couldn't open joystick '%s' : %s\n",
				SDL_JoystickName(0), SDL_GetError());
		}
	}
#endif

	/* Load the Maelstrom icon */
	icon = SDL_LoadBMP_RW(PHYSFSRWOPS_openRead("icon.bmp"), 1);
	if ( icon == NULL ) {
		error("Fatal: Couldn't load icon: %s\n", SDL_GetError());
		return(-1);
	}

	/* Initialize the screen */
	screen = new FrameBuf;
	if (screen->Init(SCREEN_WIDTH, SCREEN_HEIGHT, window_flags, render_flags,
					colors[gGammaCorrect], icon) < 0){
		error("Fatal: %s\n", screen->Error());
		return(-1);
	}
	screen->SetCaption("Maelstrom");
	screen->Clear();
	screen->Update();
	SDL_FreeSurface(icon);

	/* Load the Font Server and fonts */
	fontserv = new FontServ(screen, "Maelstrom Fonts");
	if ( fontserv->Error() ) {
		error("Fatal: %s\n", fontserv->Error());
		return(-1);
	}
	memset(fonts, 0, sizeof(fonts));
	fonts[CHICAGO_12] = fontserv->NewFont("Chicago", 12);
	fonts[GENEVA_9] = fontserv->NewFont("Geneva", 9);
	fonts[NEWYORK_14] = fontserv->NewFont("New York", 14);
	fonts[NEWYORK_18] = fontserv->NewFont("New York", 18);
	for ( i = 0; i < NUM_FONTS; ++i ) {
		if ( !fonts[i] ) {
			error("Fatal: Couldn't load fonts");
			return(-1);
		}
	}

	/* Create the UI manager */
	ui = new UIManager(screen, CreateMaelstromUIElement);
	ui->SetLoadPath("UI");

	/* Load the Sound Server and initialize sound */
	sound = new Sound("Maelstrom Sounds", gSoundLevel);
	if ( sound->Error() ) {
		error("Fatal: %s\n", sound->Error());
		return(-1);
	}

	/* -- We want to access the FULL screen! */
	SetRect(&gScrnRect, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	gStatusLine = (gScrnRect.bottom - gScrnRect.top - STATUS_HEIGHT);
	gScrnRect.bottom -= STATUS_HEIGHT;
	gTop = 0;
	gLeft = 0;
	gBottom = gScrnRect.bottom - gScrnRect.top;
	gRight = gScrnRect.right - gScrnRect.left;

	gClipRect.x = gLeft+SPRITES_WIDTH;
	gClipRect.y = gTop+SPRITES_WIDTH;
	gClipRect.w = gRight-gLeft-2*SPRITES_WIDTH;
	gClipRect.h = gBottom-gTop-2*SPRITES_WIDTH+STATUS_HEIGHT;
	screen->ClipBlit(&gClipRect);

	/* Do the Ambrosia Splash screen */
	screen->FadeOut();
	ui->LoadPanel(PANEL_SPLASH);
	ui->ShowPanel(PANEL_SPLASH);
	ui->Draw();

	start = SDL_GetTicks();

	/* Preload some of our data while the splash screen is up */

	/* -- Load in the prize CICN's */
	if ( LoadCICNS() < 0 )
		return(-1);

	/* -- Load the rest of the UI panels */
	if (!ui->LoadPanels()) {
		error("Couldn't load panels: %s\n", ui->Error());
		return(-1);
	}

	/* -- Create the stars array */
	InitStars();

	/* -- Set up the velocity tables */
	BuildVelocityTable();

	/* Wait for the splash time to finish, or a keypress */
	while ((SDL_GetTicks() - start) < 5000) {
		if ( DropEvents() ) {
			break;
		}
		SDL_Delay(100);
	}

	ui->DeletePanel(PANEL_SPLASH);

	/* -- Throw up our intro screen */
	ui->ShowPanel(PANEL_LOADING);
	sound->PlaySound(gPrizeAppears, 1);
	ui->Draw();

	/* -- Load in our sprites and other needed resources */
	{
		Mac_Resource spriteres("Maelstrom Sprites");

		if ( spriteres.Error() ) {
			error("%s\n", spriteres.Error());
			return(-1);
		}
		if ( LoadBlits(&spriteres) < 0 ) {
			return(-1);
		}
	}

	/* -- Create the shots array */
	InitShots();

	/* -- Initialize the sprite manager - after we load blits and shots! */
	if ( InitSprites() < 0 )
		return(-1);

	ui->DeletePanel(PANEL_LOADING);

	return(0);
}	/* -- DoInitializations */


/* ----------------------------------------------------------------- */
/* -- Load in the blits */

static int LoadBlits(Mac_Resource *spriteres)
{
	int stage = 1;

	DrawLoadBar(stage++);

/* -- Load in the thrusters */

	if ( LoadSmallSprite(spriteres, &gThrust1, 400, SHIP_FRAMES) < 0 )
		return(-1);
	DrawLoadBar(stage++);

	if ( LoadSmallSprite(spriteres, &gThrust2, 500, SHIP_FRAMES) < 0 )
		return(-1);
	DrawLoadBar(stage++);

/* -- Load in the player's ship */

	if ( LoadSprite(spriteres, &gPlayerShip, 200, SHIP_FRAMES) < 0 )
		return(-1);
	DrawLoadBar(stage++);

/* -- Load in the large rock */

	if ( LoadSprite(spriteres, &gRock1R, 500, 60) < 0 )
		return(-1);
	BackwardsSprite(&gRock1L, gRock1R);
	DrawLoadBar(stage++);

/* -- Load in the medium rock */

	if ( LoadSprite(spriteres, &gRock2R, 400, 40) < 0 )
		return(-1);
	BackwardsSprite(&gRock2L, gRock2R);
	DrawLoadBar(stage++);

/* -- Load in the small rock */

	if ( LoadSmallSprite(spriteres, &gRock3R, 300, 20) < 0 )
		return(-1);
	BackwardsSprite(&gRock3L, gRock3R);
	DrawLoadBar(stage++);

/* -- Load in the explosion */

	if ( LoadSprite(spriteres, &gExplosion, 600, 12) < 0 )
		return(-1);
	DrawLoadBar(stage++);

/* -- Load in the 2x multiplier */

	if ( LoadSprite(spriteres, &gMult[0], 2000, 1) < 0 )
		return(-1);
	DrawLoadBar(stage++);

/* -- Load in the 3x multiplier */

	if ( LoadSprite(spriteres, &gMult[1], 2002, 1) < 0 )
		return(-1);
	DrawLoadBar(stage++);

/* -- Load in the 4x multiplier */

	if ( LoadSprite(spriteres, &gMult[2], 2004, 1) < 0 )
		return(-1);
	DrawLoadBar(stage++);

/* -- Load in the 5x multiplier */

	if ( LoadSprite(spriteres, &gMult[3], 2006, 1) < 0 )
		return(-1);
	DrawLoadBar(stage++);

/* -- Load in the steel asteroid */

	if ( LoadSprite(spriteres, &gSteelRoidL, 700, 40) < 0 )
		return(-1);
	BackwardsSprite(&gSteelRoidR, gSteelRoidL);
	DrawLoadBar(stage++);

/* -- Load in the prize */

	if ( LoadSprite(spriteres, &gPrize, 800, 30) < 0 )
		return(-1);
	DrawLoadBar(stage++);

/* -- Load in the bonus */

	if ( LoadSprite(spriteres, &gBonusBlit, 900, 10) < 0 )
		return(-1);
	DrawLoadBar(stage++);

/* -- Load in the bonus */

	if ( LoadSprite(spriteres, &gPointBlit, 1000, 6) < 0 )
		return(-1);
	DrawLoadBar(stage++);

/* -- Load in the vortex */

	if ( LoadSprite(spriteres, &gVortexBlit, 1100, 10) < 0 )
		return(-1);
	DrawLoadBar(stage++);

/* -- Load in the homing mine */

	if ( LoadSprite(spriteres, &gMineBlitR, 1200, 40) < 0 )
		return(-1);
	BackwardsSprite(&gMineBlitL, gMineBlitR);
	DrawLoadBar(stage++);

/* -- Load in the shield */

	if ( LoadSprite(spriteres, &gShieldBlit, 1300, 2) < 0 )
		return(-1);
	DrawLoadBar(stage++);

/* -- Load in the nova */

	if ( LoadSprite(spriteres, &gNova, 1400, 18) < 0 )
		return(-1);
	DrawLoadBar(stage++);

/* -- Load in the ship explosion */

	if ( LoadSprite(spriteres, &gShipExplosion, 1500, 21) < 0 )
		return(-1);
	DrawLoadBar(stage++);

/* -- Load in the shrapnel */

	if ( LoadSprite(spriteres, &gShrapnel1, 1800, 50) < 0 )
		return(-1);
	DrawLoadBar(stage++);

	if ( LoadSprite(spriteres, &gShrapnel2, 1900, 42) < 0 )
		return(-1);
	DrawLoadBar(stage++);

/* -- Load in the damaged ship */

	if ( LoadSprite(spriteres, &gDamagedShip, 1600, 10) < 0 )
		return(-1);
	DrawLoadBar(stage++);

/* -- Load in the enemy ship */

	if ( LoadSprite(spriteres, &gEnemyShip, 1700, 40) < 0 )
		return(-1);
	DrawLoadBar(stage++);

/* -- Load in the enemy ship */

	if ( LoadSprite(spriteres, &gEnemyShip2, 2100, 40) < 0 )
		return(-1);
	DrawLoadBar(stage++);

	return(0);
}	/* -- LoadBlits */


/* -- Initialize our sprites */

int InitSprites(void)
{
	/* Initialize sprite variables */
	gNumSprites = 0;
	gLastDrawn = 0L;

	/* Initialize player sprites */
	return(InitPlayerSprites());
}	/* -- InitSprites */


/* ----------------------------------------------------------------- */
/* -- Load in the sprites we use */

static void BackwardsSprite(BlitPtr *theBlit, BlitPtr oldBlit)
{
	BlitPtr	aBlit;
	int index, nFrames;

	aBlit = new Blit;
	nFrames = oldBlit->numFrames;
	aBlit->numFrames = nFrames;
	aBlit->hitRect.left = oldBlit->hitRect.left;
	aBlit->hitRect.right = oldBlit->hitRect.right;
	aBlit->hitRect.top = oldBlit->hitRect.top;
	aBlit->hitRect.bottom = oldBlit->hitRect.bottom;
	aBlit->isSmall = oldBlit->isSmall;

	/* -- Reverse the sprite images */
	for (index = 0; index < aBlit->numFrames; index++) {
		aBlit->sprite[index] = oldBlit->sprite[nFrames - index - 1];
		aBlit->mask[index] = oldBlit->mask[nFrames - index - 1];
	}
	(*theBlit) = aBlit;
}	/* -- BackwardsSprite */


/* ----------------------------------------------------------------- */
/* -- Load in the sprites we use */

static int LoadSprite(Mac_Resource *spriteres,
			BlitPtr *theBlit, int baseID, int numFrames)
{
	Mac_ResData *S, *M;
	int	index;
	BlitPtr	aBlit;
	Uint32	offset;
	int	top, left, bottom, right;
	int	row, col;
	Uint8	*mask;

	aBlit = new Blit;
	aBlit->numFrames = numFrames;
	aBlit->isSmall = 0;

	left = 32;
	right = 0;
	top = 32;
	bottom = 0;

	/* -- Load in the image data */
	for (index = 0; index < numFrames; index++) {

		M = spriteres->Resource("ICN#", baseID+index);
		if ( M== NULL ) {
			error(
	"LoadSprite(%d+%d): Couldn't load ICN# resource!\n", baseID, index);
			return(-1);
		}
		mask = M->data+128;
		
		S = spriteres->Resource("icl8", baseID+index);
		if ( S == NULL ) {
			error(
	"LoadSprite(%d+%d): Couldn't load icl8 resource!\n", baseID, index);
			return(-1);
		}

		/* -- Figure out the hit rectangle */
		/* -- Do the top/left first */
		for ( row=0; row<32; ++row ) {
			for ( col=0; col<32; ++col ) {
				offset = (row*32)+col;
				if ((mask[offset/8]>>(7-(offset%8))) & 0x01){
					if ( row < top )
						top = row;
					if ( col < left )
						left = col;
				}
			}
		}
		for ( row=31; row>top; --row ) {
			for ( col=31; col>left; --col ) {
				offset = (row*32)+col;
				if ((mask[offset/8]>>(7-(offset%8))) & 0x01){
					if ( row > bottom )
						bottom = row;
					if ( col > right )
						right = col;
				}
			}
		}
		SetRect(&aBlit->hitRect, left, top, right, bottom);
				
		/* Load the image */
		aBlit->sprite[index] = screen->LoadImage(32, 32, S->data, mask);
		if ( aBlit->sprite[index] == NULL ) {
			error(
	"LoadSprite(%d+%d): Couldn't convert sprite image!\n", baseID, index);
			return(-1);
		}

		/* Create the bytemask */
		M->length = (M->length-128)*8;
		aBlit->mask[index] = new Uint8[M->length];
		for ( offset=0; offset<M->length; ++offset ) {
			aBlit->mask[index][offset] = 
				((mask[offset/8]>>(7-(offset%8)))&0x01);
		}
	}
	(*theBlit) = aBlit;
	return(0);
}	/* -- LoadSprite */


/* ----------------------------------------------------------------- */
/* -- Load in the prize CICN's */

static int LoadCICNS(void)
{
	if ( (gAutoFireIcon = GetCIcon(screen, 128)) == NULL )
		return(-1);
	if ( (gAirBrakesIcon = GetCIcon(screen, 129)) == NULL )
		return(-1);
	if ( (gMult2Icon = GetCIcon(screen, 130)) == NULL )
		return(-1);
	if ( (gMult3Icon = GetCIcon(screen, 131)) == NULL )
		return(-1);
	if ( (gMult4Icon = GetCIcon(screen, 132)) == NULL )
		return(-1);
	if ( (gMult5Icon = GetCIcon(screen, 134)) == NULL )
		return(-1);
	if ( (gLuckOfTheIrishIcon = GetCIcon(screen, 133)) == NULL )
		return(-1);
	if ( (gTripleFireIcon = GetCIcon(screen, 135)) == NULL )
		return(-1);
	if ( (gLongFireIcon = GetCIcon(screen, 136)) == NULL )
		return(-1);
	if ( (gShieldIcon = GetCIcon(screen, 137)) == NULL )
		return(-1);
	if ( (gKeyIcon = GetCIcon(screen, 100)) == NULL )
		return(-1);
	return(0);
}	/* -- LoadCICNS */


/* ----------------------------------------------------------------- */
/* -- Load in the sprites we use */

static int LoadSmallSprite(Mac_Resource *spriteres,
				BlitPtr *theBlit, int baseID, int numFrames)
{
	Mac_ResData *S, *M;
	int	index;
	BlitPtr	aBlit;
	Uint32	offset;
	int	top, left, bottom, right;
	int	row, col;
	Uint8	*mask;

	aBlit = new Blit;
	aBlit->numFrames = numFrames;
	aBlit->isSmall = 1;

	left = 16;
	right = 0;
	top = 16;
	bottom = 0;

	/* -- Load in the image data */

	for (index = 0; index < numFrames; index++) {

		M = spriteres->Resource("ics#", baseID+index);
		if ( M == NULL ) {
			error(
	"LoadSmallSprite(%d+%d): Couldn't load ics# resource!\n", baseID,index);
			return(-1);
		}
		mask = M->data+32;

		S = spriteres->Resource("ics8", baseID+index);
		if ( S == NULL ) {
			error(
	"LoadSmallSprite(%d+%d): Couldn't load ics8 resource!\n", baseID,index);
			return(-1);
		}

		/* -- Figure out the hit rectangle */
		/* -- Do the top/left first */
		for ( row=0; row<16; ++row ) {
			for ( col=0; col<16; ++col ) {
				offset = (row*16)+col;
				if ((mask[offset/8]>>(7-(offset%8))) & 0x01){
					if ( row < top )
						top = row;
					if ( col < left )
						left = col;
				}
			}
		}
		for ( row=15; row>top; --row ) {
			for ( col=15; col>left; --col ) {
				offset = (row*16)+col;
				if ((mask[offset/8]>>(7-(offset%8))) & 0x01){
					if ( row > bottom )
						bottom = row;
					if ( col > right )
						right = col;
				}
			}
		}
		SetRect(&aBlit->hitRect, left, top, right, bottom);

		/* Load the image */
		aBlit->sprite[index] = screen->LoadImage(16, 16, S->data, mask);
		if ( aBlit->sprite[index] == NULL ) {
			error(
	"LoadSprite(%d+%d): Couldn't convert sprite image!\n", baseID, index);
			return(-1);
		}

		/* Create the bytemask */
		M->length = (M->length-32)*8;
		aBlit->mask[index] = new Uint8[M->length];
		for ( offset=0; offset<M->length; ++offset ) {
			aBlit->mask[index][offset] = 
				((mask[offset/8]>>(7-(offset%8)))&0x01);
		}
	}
	(*theBlit) = aBlit;
	return(0);
}	/* -- LoadSmallSprite */

