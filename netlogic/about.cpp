
#include "Maelstrom_Globals.h"
#include "object.h"
#include "about.h"


AboutPanelDelegate::AboutPanelDelegate()
{
	numsprites = 0;
}

AboutPanelDelegate::~AboutPanelDelegate()
{
	assert(numsprites == 0);
}

void
AboutPanelDelegate::OnShow()
{
	int x, y, off;

	gNoDelay = 0;

	x = (80) * SCALE_FACTOR;
	y = (136) * SCALE_FACTOR;
	off = 39 * SCALE_FACTOR;

	objects[numsprites++] = 
		new Object(x, y, 0, 0, gPlayerShip, 1);
	y += off;
	objects[numsprites++] = 
		new Object(x, y, 0, 0, gPrize, 2);
	y += off;
	objects[numsprites++] = 
		new Object(x, y, 0, 0, gBonusBlit, 2);
	y += off;
	objects[numsprites++] = 
		new Object(x, y, 0, 0, gMult[3], 1);
	y += off;
	objects[numsprites++] = 
		new Object(x, y, 0, 0, gDamagedShip, 1);
	y += off;

	/* -- Now for the second column */
	x = (340) * SCALE_FACTOR;
	y = (136) * SCALE_FACTOR;
	off = 39 * SCALE_FACTOR;

	objects[numsprites++] = 
		new Object(x, y, 0, 0, gRock1R, 1);
	y += off;
	objects[numsprites++] = 
		new Object(x, y, 0, 0, gSteelRoidR, 1);
	y += off;
	objects[numsprites++] = 
		new Object(x, y, 0, 0, gNova, 4);
	y += off;
	objects[numsprites++] = 
		new Object(x, y, 0, 0, gMineBlitL, 1);
	y += off;
	objects[numsprites++] = 
		new Object(x, y, 0, 0, gVortexBlit, 3);
	y += off;
	objects[numsprites++] = 
		new Object(x, y, 0, 0, gEnemyShip, 1);
	y += off;

}

void
AboutPanelDelegate::OnHide()
{
	int i;

	for ( i=0; i<numsprites; ++i )
		delete objects[i];
	numsprites = 0;
}

void
AboutPanelDelegate::OnDraw()
{
	int i;

	for ( i=0; i<numsprites; ++i ) {
		objects[i]->Move(0);
		objects[i]->BlitSprite();
	}
}
