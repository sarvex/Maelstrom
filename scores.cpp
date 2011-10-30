
/* 
   This file handles the cheat dialogs and the high score file
*/

#ifdef unix
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include <stdio.h>

#include "SDL_endian.h"

#include "Maelstrom_Globals.h"
#include "load.h"
#include "dialog.h"

#define MAELSTROM_SCORES	"Maelstrom-Scores"
#define NUM_SCORES		10		// Do not change this!

/* Everyone can write to scores file if defined to 0 */
#define SCORES_PERMMASK		0

Bool gNetScores = 0;
Scores hScores[NUM_SCORES];

void LoadScores(void)
{
	SDL_RWops *scores_src;
	int i;

	/* Try to load network scores, if we can */
	if ( gNetScores ) {
		if ( NetLoadScores() == 0 )
			return;
		else {
			mesg("Using local score file\n\n");
			gNetScores = 0;
		}
	}
	memset(&hScores, 0, sizeof(hScores));

	scores_src = PHYSFSRWOPS_openRead(MAELSTROM_SCORES);
	if ( scores_src != NULL ) {
		for ( i=0; i<NUM_SCORES; ++i ) {
			SDL_RWread(scores_src, hScores[i].name,
			           sizeof(hScores[i].name), 1);
			hScores[i].wave = SDL_ReadBE32(scores_src);
			hScores[i].score = SDL_ReadBE32(scores_src);
		}
		SDL_RWclose(scores_src);
	}
}

void SaveScores(void)
{
	SDL_RWops *scores_src;
	int i;
#ifdef unix
	int omask;
#endif

	/* Don't save network scores */
	if ( gNetScores )
		return;

#ifdef unix
	omask=umask(SCORES_PERMMASK);
#endif
	scores_src = PHYSFSRWOPS_openWrite(MAELSTROM_SCORES);
	if ( scores_src != NULL ) {
		for ( i=0; i<NUM_SCORES; ++i ) {
			SDL_RWwrite(scores_src, hScores[i].name,
			            sizeof(hScores[i].name), 1);
			SDL_WriteBE32(scores_src, hScores[i].wave);
			SDL_WriteBE32(scores_src, hScores[i].score);
		}
		SDL_RWclose(scores_src);
	} else {
		error("Warning: Couldn't save scores to %s\n",
						MAELSTROM_SCORES);
	}
#ifdef unix
	umask(omask);
#endif
}

/* Just show the high scores */
void PrintHighScores(void)
{
	int i;

	LoadScores();
	/* FIXME! -- Put all lines into a single formatted message */
	printf("Name			Score	Wave\n");
	for ( i=0; i<NUM_SCORES; ++i ) {
		printf("%-20s	%-3.1u	%u\n", hScores[i].name,
					hScores[i].score, hScores[i].wave);
	}
}

void ZapHighScores(UIDialog *dialog, int status)
{
	if (status) {
		memset(hScores, 0, sizeof(hScores));
		SaveScores();
		gLastHigh = -1;

		/* Fade the screen and redisplay scores */
		screen->FadeOut();
		Delay(SOUND_DELAY);
		sound->PlaySound(gExplosionSound, 5);
		gUpdateBuffer = true;
	}
}

