
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL/SDL.h>
/**
* Game functions headers
*/

typedef uint8_t **laby_t;	//tableau deux dimensions
typedef uint8_t * mrow_t;
/*************************** Functions ********************************/

void pause();
laby_t alloc_laby(int w, int h);
void SDL_DrawLine(SDL_Surface *ecran, uint32_t color, int x, int y, int x2, int y2);
Uint32 SDL_Ticks(Uint32 interval, void *unused);
void Music(char *music);
void jouer_auto();
void menu();
void help();
void about();
void settings();
void ranking();
