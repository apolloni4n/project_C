#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "constantes.h"
#include <SDL/SDL_mixer.h>
#include <SDL/SDL.h>
#include "SDL_ttf.h"
#include <SDL_image.h>
#include "SDL_mixer.h"

typedef uint8_t **laby_t;	//tableau deux dimensions
typedef uint8_t * mrow_t;

SDL_Surface * fenetre;
void ranking();
void menu();
void pause();
 int   pausedTicks = 0;
int startTicks=0;

void pause()
{
	int continuer = 1;
	SDL_Event event;
	while (continuer)
	{
		SDL_WaitEvent(&event);
		switch (event.type)
		{
			case SDL_KEYDOWN:
			case SDL_QUIT:
				continuer = 0;
		}
	}

        pausedTicks = SDL_GetTicks() - startTicks;

	Mix_CloseAudio();

}

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
laby_t alloc_laby(int w, int h)
{
	laby_t laby = (laby_t) calloc(1, w *h + h* sizeof(laby));
	mrow_t mem = (mrow_t)(laby + h);
	int i;
	memset(mem, LABF_NSOE, w *h);
	for (i = 0; i < h; laby[i] = mem, mem += w, i++);
	return laby;
}
void SDL_DrawLine(SDL_Surface *ecran, uint32_t color, int x, int y, int x2, int y2)
{
	SDL_Rect r = { x,
		y,
		x2 - x + 1,
		y2 - y + 1
	};
	SDL_FillRect(ecran, &r, color);
}

/* Fonction de callback (sera appelée toutes les 100 ms) */
Uint32 SDL_Ticks(Uint32 interval, void *unused)
{
	SDL_Event tick;
	memset(&tick, 0, sizeof tick);
	tick.type = SDL_USEREVENT;
	SDL_PushEvent(&tick);
	return interval;
}
void Music(char *music)
{
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024);
	Mix_Music * musique;
	musique = Mix_LoadMUS(music);
	Mix_VolumeMusic(MIX_MAX_VOLUME / 3);
	Mix_PlayMusic(musique, -1);
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024) == -1)	//Initialisation de l'API Mixer
	{
		printf("%s", Mix_GetError());
	}
}

void jouer_auto()
{
	// Initialisation des nombres aléatoires
	srand(time(NULL));
	//Définition des variables
	int w;	// largeur du labyrinthe (verticale)
	int h;	// Longeur du labyrinthe (horizontale)
	int t, i, j;	// Compteurs
	int xs, ys;	// Position de départ s=start, e=end
	int xe, ye, xe1, ye1;	// Position de fin
	w = 10;
	h = 10;
	fprintf(stderr, "Generation du labyrinthe %dx%d...\n", w, h);
	/*Sélection d'un point d'entrée et de sortie*/
	SDL_Rect path1;
	SDL_Surface *ecran = SDL_SetVideoMode(BLOC_W *w + 30, BLOC_H *h + 50, 32, SDL_HWSURFACE);
	SDL_Surface *out = IMG_Load("exit.jpg");

	switch (rand() % 2)
	{
		case 0:
			ys = 0;
			xs = rand() % w;
			ye = h - 1;
			xe1 = rand() % w;
			ye1 = h - 1;
			xe = rand() % w;
			i = LABF_NORD;
			break;
		case 1:
			xs = 0;
			ys = rand() % h;
			xe = w - 1;
			xe1 = w - 1;
			ye1 = rand() % h;
			ye = rand() % h;
			i = LABF_OUEST;
	}
	/*Création du labyrinthe : matrice de cases*/
	laby_t laby = alloc_laby(w, h);
	int *stack = (int*) calloc(w *h, sizeof(stack));
	laby[ys][xs] &= ~i;
	laby[ye1][xe1] &= ~opposite[i];
	laby[ye][xe] &= ~opposite[i];
	stack[0] = MAKE_POS(xs, ys);
	/*Génération des portes via algo de backtracking récursif.*/
	for (i = 0, j = 1; j > 0; i++)
	{
		int x = stack[j - 1] &0xffff;
		int y = stack[j - 1] >> 16;
		int f = 0;
		laby[y][x] |= LABF_VISIT;
		/*On visite toutes les cellules avoisinantes de manière alélatoire*/
		while (f != LABF_NSOE)
		{
			int dir = 1 << (rand() % 4); /*Choisi une direction parmi les 4 possibles*/
			int nx, ny;
			/*Ajuste si on est déjà passé par là*/
			if (f & dir)
			{
				if ((f &(dir - 1)) < dir - 1)
					while (f & dir) dir >>= 1;
				else
					while (f & dir) dir <<= 1;
			}
			switch (dir)
			{
				case LABF_NORD:
					nx = x;
					ny = y - 1;
					break;
				case LABF_EST:
					ny = y;
					nx = x + 1;
					break;
				case LABF_SUD:
					nx = x;
					ny = y + 1;
					break;
				case LABF_OUEST:
					ny = y;
					nx = x - 1;
					break;
			}
			/*On verifie si on a deja visité ou pas */
			if (0 <= nx && nx < w && 0 <= ny && ny < h && !(laby[ny][nx] &LABF_VISIT))
			{ 	/*Visite cette cellule, en créant un chemin de (x, y) à (nx, ny)*/
				laby[y][x] &= ~dir;
				laby[ny][nx] &= ~opposite[dir];
				stack[j++] = MAKE_POS(nx, ny);

				break;
			}
			else f |= dir;
		}
		/*On a visité toutes les cellules avoisinante: backtrack*/
		if (f == LABF_NSOE) j--;
	}
	/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
	/*Dessin*/
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	SDL_WM_SetCaption("Labyrinthe", NULL);
	SDL_TimerID timer;

	/*definition des couleurs du fond*/
	uint32_t back = SDL_MapRGB(ecran->format, 0, 0, 0);
	uint32_t move = SDL_MapRGB(ecran->format, 156, 133, 209);
	uint32_t vert = SDL_MapRGB(ecran->format, 60, 197, 212);
	uint32_t white = SDL_MapRGB(ecran->format, 255, 255, 255);
	uint32_t dead = SDL_MapRGB(ecran->format, 183, 183, 183);
	SDL_FillRect(ecran, NULL, back);
	/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

	/*Affichage du labyrinthe */
	SDL_Surface *background = NULL;
	background = IMG_Load("auroora.jpeg");
	SDL_Rect position;
	position.x = 0;
	position.y = 0;
	SDL_BlitSurface(background, NULL, ecran, &position);
	SDL_UpdateRect(ecran, 0, 0, 0, 0);
	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			int f = laby[j][i];
			int x = 10 + i * BLOC_W;
			int y = 10 + j * BLOC_H;
			int x2 = x + BLOC_W - 1;
			int y2 = y + BLOC_H - 1;
			if (j > 0) f &= ~LABF_NORD;
			if (i > 0) f &= ~LABF_OUEST;
			laby[j][i] &= ~LABF_VISIT;
			/*Placement de points aux angles des murs pour avoir un dessin continu du labyrinthe généré*/
			if (f & LABF_NORD) SDL_DrawLine(ecran, white, x, y, x2, y);
			if (f & LABF_EST) SDL_DrawLine(ecran, white, x2, y, x2, y2);
			if (f & LABF_SUD) SDL_DrawLine(ecran, white, x, y2, x2, y2);
			if (f & LABF_OUEST) SDL_DrawLine(ecran, white, x, y, x, y2);
		}
	}
SDL_Surface * ecran1=ecran;
	timer = SDL_AddTimer(100, SDL_Ticks, NULL);
		/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++1++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

	/*Résolution du Labyrinthe*/
	stack[0] = MAKE_POS(xs, ys); /*On met les portes de sortie dans le stack*/
	j = 1;

	while ( (!(xs == xe && ys == ye) && !(xs == xe1 && ys == ye1))    && j > 0)	// si on arrive pas à la sortie du labyrinthe
	{
		SDL_Event event;
		SDL_WaitEvent(&event);
		int l;
		switch (event.type)
		{
			case SDL_KEYDOWN:
				if (event.key.state != SDL_PRESSED || event.key.keysym.sym != SDLK_ESCAPE)
					continue;
			case SDL_QUIT:
				SDL_Init(SDL_INIT_VIDEO);
				fenetre = SDL_SetVideoMode(1024, 668, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
				SDL_WM_SetCaption("Labyrinthe", NULL);
				IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF);
				menu();

				Mix_PauseMusic();
				j = -1;
				continue;
			case SDL_USEREVENT:
				break;
			default:
				continue;
		}


		int x = stack[j - 1] &0xffff;
		int y = stack[j - 1] >> 16;
		int f = laby[y][x] &~LABF_VISIT;//contient la cellule non visitée
		SDL_Rect path = { 10 + x *BLOC_W,
			10 + y *BLOC_H,
			BLOC_W,
			BLOC_H
		};
		if (f & LABF_NORD) path.y += 2, path.h -= 2;// si la cellule du nord est non visitée , on la visite
		if (f & LABF_OUEST) path.x += 2, path.w -= 2;// si la cellule du ouest est non visitée , on la visite
		if (f & LABF_SUD) path.h -= 2;// si la cellule du sud est non visitée , on la visite
		if (f & LABF_EST) path.w -= 2;// si la cellule du est est non visitée , on la visite
		/*update the screen */
		SDL_FillRect(ecran, &path, move);
		SDL_UpdateRect(ecran, 0, 0, 0, 0);

		laby[y][x] |= LABF_VISIT;
		xs = x;
		ys = y;

		//Avance dans le labyrinthe
		while (f != LABF_NSOE)
		{
			int dir = 1 << (rand() % 4);	//Choisi une direction parmi les 4 possibles
			int nx, ny;
			//Ajuste si on est déjà passé par là
			if (f & dir)
			{
				if ((f &(dir - 1)) < dir - 1)
					while (f & dir) dir >>= 1;
				else
					while (f & dir) dir <<= 1;
			}
			switch (dir)
			{
				case LABF_NORD:
					nx = x;
					ny = y - 1;
					break;
				case LABF_EST:
					ny = y;
					nx = x + 1;
					break;
				case LABF_SUD:
					nx = x;
					ny = y + 1;
					break;
				case LABF_OUEST:
					ny = y;
					nx = x - 1;
					break;
			}

			//Dèjà visité ou visitable ?
			if (0 <= nx && nx < w && 0 <= ny && ny < h && !(laby[ny][nx] &LABF_VISIT))
			{
				stack[j++] = MAKE_POS(nx, ny);
				break;
			}
			else f |= dir;
			t++;
		}
		if (f == LABF_NSOE)
		{
			SDL_FillRect(ecran, &path, dead);
			j--;
		}
	}

	background = IMG_Load("auroora.jpeg");
	position.x = 0;
	position.y = 0;
	SDL_BlitSurface(background, NULL, ecran1, &position);
	SDL_UpdateRect(ecran1, 0, 0, 0, 0);
	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			int f = laby[j][i];
			int x = 10 + i * BLOC_W;
			int y = 10 + j * BLOC_H;
			int x2 = x + BLOC_W - 1;
			int y2 = y + BLOC_H - 1;
			if (j > 0) f &= ~LABF_NORD;
			if (i > 0) f &= ~LABF_OUEST;
			laby[j][i] &= ~LABF_VISIT;
			/*Placement de points aux angles des murs pour avoir un dessin continu du labyrinthe généré*/
			if (f & LABF_NORD) SDL_DrawLine(ecran, white, x, y, x2, y);
			if (f & LABF_EST) SDL_DrawLine(ecran, white, x2, y, x2, y2);
			if (f & LABF_SUD) SDL_DrawLine(ecran, white, x, y2, x2, y2);
			if (f & LABF_OUEST) SDL_DrawLine(ecran, white, x, y, x, y2);
		}
}

	stack[0] = MAKE_POS(xs , ys); /*On met les portes de sortie dans le stack*/
	j = 1;
SDL_Surface *texte = NULL;
		TTF_Font *police = NULL;
		police = TTF_OpenFont("arial.ttf", 35);
		SDL_Color vert1= { 60, 197, 212 };
		texte = TTF_RenderText_Blended(police, "sortie N1", vert1);
		SDL_Rect position2;
		position2.x = 1;
		position2.y = h-2;
		SDL_BlitSurface(texte, NULL, ecran1 , &position); /*Blit du texte */
		SDL_Flip(ecran1);
		TTF_CloseFont(police); /*Doit être avant TTF_Quit() */
		TTF_Quit();
			/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++22222222++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
if(xs==xe && ys==ye){
	while ( !(xs == xe1 && ys == ye1) && j > 0)	// si on arrive pas à la sortie du labyrinthe
	{
		SDL_Event event;
		SDL_WaitEvent(&event);
		int l;
		switch (event.type)
		{
			case SDL_KEYDOWN:
				if (event.key.state != SDL_PRESSED || event.key.keysym.sym != SDLK_ESCAPE)
					continue;
			case SDL_QUIT:
				SDL_Init(SDL_INIT_VIDEO);
				fenetre = SDL_SetVideoMode(1024, 668, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
				SDL_WM_SetCaption("Labyrinthe", NULL);
				IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF);
				menu();
				Mix_PauseMusic();
				j = -1;
				continue;
			case SDL_USEREVENT:
				break;
			default:
				continue;
		}


		int x = stack[j - 1] &0xffff;
		int y = stack[j - 1] >> 16;
		int f = laby[y][x] &~LABF_VISIT;//contient la cellule non visitée
		SDL_Rect path = { 10 + x *BLOC_W,
			10 + y *BLOC_H,
			BLOC_W,
			BLOC_H
		};
		if (f & LABF_NORD) path.y += 2, path.h -= 2;// si la cellule du nord est non visitée , on la visite
		if (f & LABF_OUEST) path.x += 2, path.w -= 2;// si la cellule du ouest est non visitée , on la visite
		if (f & LABF_SUD) path.h -= 2;// si la cellule du sud est non visitée , on la visite
		if (f & LABF_EST) path.w -= 2;// si la cellule du est est non visitée , on la visite
		/*update the screen */
		SDL_FillRect(ecran, &path, vert  );
		SDL_UpdateRect(ecran, 0, 0, 0, 0);

		laby[y][x] |= LABF_VISIT;
		xs = x;
		ys = y;

		//Avance dans le labyrinthe
		while (f != LABF_NSOE)
		{
			int dir = 1 << (rand() % 4);	//Choisi une direction parmi les 4 possibles
			int nx, ny;
			//Ajuste si on est déjà passé par là
			if (f & dir)
			{
				if ((f &(dir - 1)) < dir - 1)
					while (f & dir) dir >>= 1;
				else
					while (f & dir) dir <<= 1;
			}
			switch (dir)
			{
				case LABF_NORD:
					nx = x;
					ny = y - 1;
					break;
				case LABF_EST:
					ny = y;
					nx = x + 1;
					break;
				case LABF_SUD:
					nx = x;
					ny = y + 1;
					break;
				case LABF_OUEST:
					ny = y;
					nx = x - 1;
					break;
			}

			//Dèjà visité ou visitable ?
			if (0 <= nx && nx < w && 0 <= ny && ny < h && !(laby[ny][nx] &LABF_VISIT))
			{
				stack[j++] = MAKE_POS(nx, ny);
				break;
			}
			else f |= dir;
			t++;
		}
		if (f == LABF_NSOE)
		{
			SDL_FillRect(ecran, &path, dead);
			j--;
		}
	}
	}
	else {while ( !(xs == xe && ys == ye) && j > 0)	// si on arrive pas à la sortie du labyrinthe
	{
		SDL_Event event;
		SDL_WaitEvent(&event);
		int l;
		switch (event.type)
		{
			case SDL_KEYDOWN:
				if (event.key.state != SDL_PRESSED || event.key.keysym.sym != SDLK_ESCAPE)
					continue;
			case SDL_QUIT:
				SDL_Init(SDL_INIT_VIDEO);
				fenetre = SDL_SetVideoMode(1024, 668, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
				SDL_WM_SetCaption("Labyrinthe", NULL);
				IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF);
				menu();
				Mix_PauseMusic();
				j = -1;
				continue;
			case SDL_USEREVENT:
				break;
			default:
				continue;
		}


		int x = stack[j - 1] &0xffff;
		int y = stack[j - 1] >> 16;
		int f = laby[y][x] &~LABF_VISIT;//contient la cellule non visitée
		SDL_Rect path = { 10 + x *BLOC_W,
			10 + y *BLOC_H,
			BLOC_W,
			BLOC_H
		};
		if (f & LABF_NORD) path.y += 2, path.h -= 2;// si la cellule du nord est non visitée , on la visite
		if (f & LABF_OUEST) path.x += 2, path.w -= 2;// si la cellule du ouest est non visitée , on la visite
		if (f & LABF_SUD) path.h -= 2;// si la cellule du sud est non visitée , on la visite
		if (f & LABF_EST) path.w -= 2;// si la cellule du est est non visitée , on la visite
		/*update the screen */
		SDL_FillRect(ecran, &path, vert  );
		SDL_UpdateRect(ecran, 0, 0, 0, 0);

		laby[y][x] |= LABF_VISIT;
		xs = x;
		ys = y;

		//Avance dans le labyrinthe
		while (f != LABF_NSOE)
		{
			int dir = 1 << (rand() % 4);	//Choisi une direction parmi les 4 possibles
			int nx, ny;
			//Ajuste si on est déjà passé par là
			if (f & dir)
			{
				if ((f &(dir - 1)) < dir - 1)
					while (f & dir) dir >>= 1;
				else
					while (f & dir) dir <<= 1;
			}
			switch (dir)
			{
				case LABF_NORD:
					nx = x;
					ny = y - 1;
					break;
				case LABF_EST:
					ny = y;
					nx = x + 1;
					break;
				case LABF_SUD:
					nx = x;
					ny = y + 1;
					break;
				case LABF_OUEST:
					ny = y;
					nx = x - 1;
					break;
			}

			//Dèjà visité ou visitable ?
			if (0 <= nx && nx < w && 0 <= ny && ny < h && !(laby[ny][nx] &LABF_VISIT))
			{
				stack[j++] = MAKE_POS(nx, ny);
				break;
			}
			else f |= dir;
			t++;
		}
		if (f == LABF_NSOE)
		{
			SDL_FillRect(ecran, &path, dead);
			j--;
		}
	}}
	SDL_Surface *fenetre1 = NULL;
	SDL_Init(SDL_INIT_VIDEO);
	fenetre1 = SDL_SetVideoMode(1024, 668, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
	SDL_WM_SetCaption("Labyrinthe", NULL);
	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF);
	SDL_Surface *won = IMG_Load("youwon.jpeg");
	SDL_Rect image;
	image.x = 0;
	image.y = 0;
	SDL_BlitSurface(won, NULL, fenetre1, &image);
	SDL_Flip(fenetre1);
	SDL_UpdateRect(fenetre1, 0, 0, 0, 0);
	Music("Clapping Sound Effects.mp3");
	SDL_Delay(1000);
    Mix_PauseMusic();

	SDL_RemoveTimer(timer);
	//====================Fin du programme============================
	if (j == 0)
	{
		fprintf(stderr, "Labyrinthe résolu...\n");
		pause();
	}
	SDL_FreeSurface(ecran);
	Mix_CloseAudio();
	SDL_Surface *fenetre = NULL;
	SDL_Init(SDL_INIT_VIDEO);
	fenetre = SDL_SetVideoMode(1024, 668, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
	SDL_WM_SetCaption("Labyrinthe", NULL);
	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF);
	menu();
}

struct jr
{
	int id;
	char nom[50];
	char prn[50];
	int nbr;
	char score[50];
	char best[50];
	char dte[300];
};
//===============================================Jeu_Manuelle===========================================
void jouer_manuelle()
{
	// Initialisation des nombres aléatoires
	srand(time(NULL));
	//Définition des variables
	int w;	// largeur du labyrinthe (verticale)
	int h;	// Longeur du labyrinthe (horizontale)
	int t, i, j;	// Compteurs
	int xs, ys;	// Position de départ s=start, e=end
	int xe, ye, xe1, ye1;	// Position de fin
	w = 30;
	h = 30;
	fprintf(stderr, "Generation du labyrinthe %dx%d...\n", w, h);
	/*Sélection d'un point d'entrée et de sortie*/
	SDL_Rect path1;
	SDL_Surface *ecran = SDL_SetVideoMode(BLOC_W *w + 30, BLOC_H *h + 50, 32, SDL_HWSURFACE);
	SDL_Surface *out = IMG_Load("exit.jpg");

	switch (rand() % 2)
	{
		case 0:
			ys = 0;
			xs = rand() % w;
			ye = h - 1;
			xe1 = rand() % w;
			ye1 = h - 1;
			xe = rand() % w;
			i = LABF_NORD;
			break;
		case 1:
			xs = 0;
			ys = rand() % h;
			xe = w - 1;
			xe1 = w - 1;
			ye1 = rand() % h;
			ye = rand() % h;
			i = LABF_OUEST;
	}
	/*Création du labyrinthe : matrice de cases*/
	laby_t laby = alloc_laby(w, h);
	int *stack = (int*) calloc(w *h, sizeof(stack));
	laby[ys][xs] &= ~i;
	laby[ye1][xe1] &= ~opposite[i];
	laby[ye][xe] &= ~opposite[i];
	stack[0] = MAKE_POS(xs, ys);
	/*Génération des portes via algo de backtracking récursif.*/
	for (i = 0, j = 1; j > 0; i++)
	{
		int x = stack[j - 1] &0xffff;
		int y = stack[j - 1] >> 16;
		int f = 0;
		laby[y][x] |= LABF_VISIT;
		SDL_Surface * ecran1=ecran;
	SDL_bool quit = SDL_FALSE;
	SDL_bool mouse_active = SDL_FALSE;
	SDL_bool mouse_hover = SDL_FALSE;
	SDL_EnableKeyRepeat(100, 100);
	int k=0;
	while (!quit)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{

			switch (event.type)
			{
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym)
					{
						case SDLK_w:
						case SDLK_UP:
k=1;

							break;
						case SDLK_s:
						case SDLK_DOWN:
k=2;

							break;
						case SDLK_a:
						case SDLK_LEFT:
k=3;


							break;
						case SDLK_d:
						case SDLK_RIGHT:
k=4;


							break;
					}
					break;
				case SDL_QUIT:
					quit = SDL_TRUE;
					break;

				default:
					continue;
			}
		}
	}
		/*On visite toutes les cellules avoisinantes de manière alélatoire*/
		while (f != LABF_NSOE)
		{
			int nx, ny;
			int dir;
			if(k==1) dir=1;
			if(k==2) dir=4;
			if(k==3) dir=8;
			if(k==4) dir=2;

			switch (dir)
			{
				case LABF_NORD:
					nx = x;
					ny = y - 1;
					break;
				case LABF_EST:
					ny = y;
					nx = x + 1;
					break;
				case LABF_SUD:
					nx = x;
					ny = y + 1;
					break;
				case LABF_OUEST:
					ny = y;
					nx = x - 1;
					break;
			}

			/*On verifie si on a deja visité ou pas */
			if (0 <= nx && nx < w && 0 <= ny && ny < h && !(laby[ny][nx] &LABF_VISIT))
			{ 	/*Visite cette cellule, en créant un chemin de (x, y) à (nx, ny)*/
				laby[y][x] &= ~dir;
				laby[ny][nx] &= ~opposite[dir];
				stack[j++] = MAKE_POS(nx, ny);

				break;
			}
			else f |= dir;
		}
		/*On a visité toutes les cellules avoisinante: backtrack*/
		if (f == LABF_NSOE) j--;
	}
	/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
	/*Dessin*/
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	SDL_WM_SetCaption("Labyrinthe", NULL);
	SDL_TimerID timer;

	/*definition des couleurs du fond*/
	uint32_t back = SDL_MapRGB(ecran->format, 0, 0, 0);
	uint32_t move = SDL_MapRGB(ecran->format, 156, 133, 209);
	uint32_t vert = SDL_MapRGB(ecran->format, 60, 197, 212);
	uint32_t white = SDL_MapRGB(ecran->format, 255, 255, 255);
	uint32_t dead = SDL_MapRGB(ecran->format, 183, 183, 183);
	SDL_FillRect(ecran, NULL, back);
	/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

	/*Affichage du labyrinthe */
	SDL_Surface *background = NULL;
	background = IMG_Load("auroora.jpeg");
	SDL_Rect position;
	position.x = 0;
	position.y = 0;
	SDL_BlitSurface(background, NULL, ecran, &position);
	SDL_UpdateRect(ecran, 0, 0, 0, 0);
	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			int f = laby[j][i];
			int x = 10 + i * BLOC_W;
			int y = 10 + j * BLOC_H;
			int x2 = x + BLOC_W - 1;
			int y2 = y + BLOC_H - 1;
			if (j > 0) f &= ~LABF_NORD;
			if (i > 0) f &= ~LABF_OUEST;
			laby[j][i] &= ~LABF_VISIT;
			/*Placement de points aux angles des murs pour avoir un dessin continu du labyrinthe généré*/
			if (f & LABF_NORD) SDL_DrawLine(ecran, white, x, y, x2, y);
			if (f & LABF_EST) SDL_DrawLine(ecran, white, x2, y, x2, y2);
			if (f & LABF_SUD) SDL_DrawLine(ecran, white, x, y2, x2, y2);
			if (f & LABF_OUEST) SDL_DrawLine(ecran, white, x, y, x, y2);
		}
	}

	/*Résolution du Labyrinthe*/
	stack[0] = MAKE_POS(xs, ys); /*On met les portes de sortie dans le stack*/
	j = 1;

	while ( !(xs == xe && ys == ye)    && j > 0)	// si on arrive pas à la sortie du labyrinthe
	{
		SDL_Event event;
		SDL_WaitEvent(&event);
		int l;
		switch (event.type)
		{
			case SDL_KEYDOWN:
				if (event.key.state != SDL_PRESSED || event.key.keysym.sym != SDLK_ESCAPE)
					continue;
			case SDL_QUIT:
				SDL_Init(SDL_INIT_VIDEO);
				fenetre = SDL_SetVideoMode(1024, 668, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
				SDL_WM_SetCaption("Labyrinthe", NULL);
				IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF);
				menu();

				Mix_PauseMusic();
				j = -1;
				continue;
			case SDL_USEREVENT:
				break;
			default:
				continue;
		}


		int x = stack[j - 1] &0xffff;
		int y = stack[j - 1] >> 16;
		int f = laby[y][x] &~LABF_VISIT;//contient la cellule non visitée
		SDL_Rect path = { 10 + x *BLOC_W,
			10 + y *BLOC_H,
			BLOC_W,
			BLOC_H
		};
	if (f & LABF_NORD) path.y += 2, path.h -= 2;// si la cellule du nord est non visitée , on la visite
		if (f & LABF_OUEST) path.x += 2, path.w -= 2;// si la cellule du ouest est non visitée , on la visite
		if (f & LABF_SUD) path.h -= 2;// si la cellule du sud est non visitée , on la visite
		if (f & LABF_EST) path.w -= 2;// si la cellule du est est non visitée , on la visite

		/*update the screen */
		SDL_FillRect(ecran, &path, move);
		SDL_UpdateRect(ecran, 0, 0, 0, 0);

		laby[y][x] |= LABF_VISIT;
		xs = x;
		ys = y;

		//Avance dans le labyrinthe
		while (f != LABF_NSOE)
		{int k;
			int nx, ny;
			int dir;
			if(k==1) dir=1;
			if(k==2) dir=4;
			if(k==3) dir=8;
			if(k==4) dir=2;

			switch (dir)
			{
				case LABF_NORD:
					nx = x;
					ny = y - 1;
					break;
				case LABF_EST:
					ny = y;
					nx = x + 1;
					break;
				case LABF_SUD:
					nx = x;
					ny = y + 1;
					break;
				case LABF_OUEST:
					ny = y;
					nx = x - 1;
					break;
			}

			//Dèjà visité ou visitable ?
			if (0 <= nx && nx < w && 0 <= ny && ny < h && !(laby[ny][nx] &LABF_VISIT))
			{
				stack[j++] = MAKE_POS(nx, ny);
				break;
			}
			else f |= dir;
			t++;
		}
		if (f == LABF_NSOE)
		{
			SDL_FillRect(ecran, &path, dead);
			j--;
		}
	}
SDL_Surface * ecran1=ecran;

	background = IMG_Load("auroora.jpeg");
	position.x = 0;
	position.y = 0;
	SDL_BlitSurface(background, NULL, ecran1, &position);
	SDL_UpdateRect(ecran1, 0, 0, 0, 0);
	for (j = 0; j < h; j++)
	{
		for (i = 0; i < w; i++)
		{
			int f = laby[j][i];
			int x = 10 + i * BLOC_W;
			int y = 10 + j * BLOC_H;
			int x2 = x + BLOC_W - 1;
			int y2 = y + BLOC_H - 1;
			if (j > 0) f &= ~LABF_NORD;
			if (i > 0) f &= ~LABF_OUEST;
			laby[j][i] &= ~LABF_VISIT;
			/*Placement de points aux angles des murs pour avoir un dessin continu du labyrinthe généré*/
			if (f & LABF_NORD) SDL_DrawLine(ecran, white, x, y, x2, y);
			if (f & LABF_EST) SDL_DrawLine(ecran, white, x2, y, x2, y2);
			if (f & LABF_SUD) SDL_DrawLine(ecran, white, x, y2, x2, y2);
			if (f & LABF_OUEST) SDL_DrawLine(ecran, white, x, y, x, y2);
		}
	}
	SDL_Surface *texte = NULL;
		TTF_Font *police = NULL;
		police = TTF_OpenFont("arial.ttf", 35);
		SDL_Color couleurNoire = { 0, 0, 0 };
		texte = TTF_RenderText_Blended(police, "sortie N1", couleurNoire);
		SDL_Rect position2;
		position2.x = w-2;
		position2.y = h-2;
		SDL_BlitSurface(texte, NULL, ecran1 , &position); /*Blit du texte */
		SDL_Flip(ecran1);
		TTF_CloseFont(police); /*Doit être avant TTF_Quit() */

		TTF_Quit();

	stack[0] = MAKE_POS(xs , ys); /*On met les portes de sortie dans le stack*/
	j = 1;

	while ( !(xs == xe1 && ys == ye1) && j > 0)	// si on arrive pas à la sortie du labyrinthe
	{
		SDL_Event event;
		SDL_WaitEvent(&event);
		int l;
		switch (event.type)
		{
			case SDL_KEYDOWN:
				if (event.key.state != SDL_PRESSED || event.key.keysym.sym != SDLK_ESCAPE)
					continue;
			case SDL_QUIT:
				SDL_Init(SDL_INIT_VIDEO);
				fenetre = SDL_SetVideoMode(1024, 668, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
				SDL_WM_SetCaption("Labyrinthe", NULL);
				IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF);
				menu();
				Mix_PauseMusic();
				j = -1;
				continue;
			case SDL_USEREVENT:
				break;
			default:
				continue;
		}


		int x = stack[j - 1] &0xffff;
		int y = stack[j - 1] >> 16;
		int f = laby[y][x] &~LABF_VISIT;//contient la cellule non visitée
		SDL_Rect path = { 10 + x *BLOC_W,
			10 + y *BLOC_H,
			BLOC_W,
			BLOC_H
		};
	if (f & LABF_NORD) path.y += 2, path.h -= 2;// si la cellule du nord est non visitée , on la visite
		if (f & LABF_OUEST) path.x += 2, path.w -= 2;// si la cellule du ouest est non visitée , on la visite
		if (f & LABF_SUD) path.h -= 2;// si la cellule du sud est non visitée , on la visite
		if (f & LABF_EST) path.w -= 2;// si la cellule du est est non visitée , on la visite

		/*update the screen */
		SDL_FillRect(ecran, &path, vert  );
		SDL_UpdateRect(ecran, 0, 0, 0, 0);

		laby[y][x] |= LABF_VISIT;
		xs = x;
		ys = y;

		//Avance dans le labyrinthe
		while (f != LABF_NSOE)
		{
			int dir = 1 << (rand() % 4);	//Choisi une direction parmi les 4 possibles
			int nx, ny;
			//Ajuste si on est déjà passé par là
			if (f & dir)
			{
				if ((f &(dir - 1)) < dir - 1)
					while (f & dir) dir >>= 1;
				else
					while (f & dir) dir <<= 1;
			}
			switch (dir)
			{
				case LABF_NORD:
					nx = x;
					ny = y - 1;
					break;
				case LABF_EST:
					ny = y;
					nx = x + 1;
					break;
				case LABF_SUD:
					nx = x;
					ny = y + 1;
					break;
				case LABF_OUEST:
					ny = y;
					nx = x - 1;
					break;
			}

			//Dèjà visité ou visitable ?
			if (0 <= nx && nx < w && 0 <= ny && ny < h && !(laby[ny][nx] &LABF_VISIT))
			{
				stack[j++] = MAKE_POS(nx, ny);
				break;
			}
			else f |= dir;
			t++;
		}
		if (f == LABF_NSOE)
		{
			SDL_FillRect(ecran, &path, dead);
			j--;
		}
	}
	SDL_Surface *fenetre1 = NULL;
	SDL_Init(SDL_INIT_VIDEO);
	fenetre1 = SDL_SetVideoMode(1024, 668, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
	SDL_WM_SetCaption("Labyrinthe", NULL);
	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF);
	SDL_Surface *won = IMG_Load("youwon.jpeg");
	SDL_Rect image;
	image.x = 0;
	image.y = 0;
	SDL_BlitSurface(won, NULL, fenetre1, &image);
	SDL_Flip(fenetre1);
	SDL_UpdateRect(fenetre1, 0, 0, 0, 0);
	Music("Clapping Sound Effects.mp3");
	SDL_Delay(1000);
    Mix_PauseMusic();

	SDL_RemoveTimer(timer);
	//====================Fin du programme============================
	if (j == 0)
	{
		fprintf(stderr, "Labyrinthe résolu...\n");
		pause();
	}
	SDL_FreeSurface(ecran);
	Mix_CloseAudio();
	SDL_Surface *fenetre = NULL;
	SDL_Init(SDL_INIT_VIDEO);
	fenetre = SDL_SetVideoMode(1024, 668, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
	SDL_WM_SetCaption("Labyrinthe", NULL);
	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF);
	menu();
}
void play()
{
	SDL_Surface *bg = IMG_Load("mode.jpeg");
	SDL_Rect imagebg;
	imagebg.x = 0;
	imagebg.y = 0;
	SDL_BlitSurface(bg, NULL, fenetre, &imagebg);
	SDL_Flip(fenetre);

	SDL_Event event;
	while (1)
	{
		while (SDL_PollEvent(&event))

		{

			if (event.type == SDL_QUIT)
			{
				exit(0);
			}
			if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					if (((event.button.x >= 75) && (event.button.x <= 160)) && ((event.button.y >= 39) && (event.button.y <= 142)))
					{
						return;
					}
				}
			}
			if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					if (((event.button.x >= 354) && (event.button.x <= 652)) && ((event.button.y >= 152) && (event.button.y <= 235)))
					{
						jouer_manuelle();
					}
					if (((event.button.x >= 354) && (event.button.x <= 652)) && ((event.button.y >= 320) && (event.button.y <= 403)))
					{int tempsActuel=0;

						jouer_auto();
						tempsActuel=SDL_GetTicks();

					}
				}
			}
		}
	}
}


void datesys(char ch[])
{
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strcpy(ch, asctime(timeinfo));
}

void ranking()
{
	SDL_Surface *bg = IMG_Load("fin.png");
	SDL_Rect imagebg;
	imagebg.x = 0;
	imagebg.y = 0;
	SDL_BlitSurface(bg, NULL, fenetre, &imagebg);
	SDL_Event event;
	while (1)
	{
		while (SDL_PollEvent(&event))

		{

			if (event.type == SDL_QUIT)
			{
				exit(0);
			}
			if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					if (((event.button.x >= 75) && (event.button.x <= 160)) && ((event.button.y >= 39) && (event.button.y <= 142)))
					{
						return;
					}
				}
			}
		}
		TTF_Init();
		if (TTF_Init() == -1)
		{
			fprintf(stderr, "Erreur d'initialisation de TTF_Init : %s\n", TTF_GetError());
			exit(EXIT_FAILURE);
		}
SDL_Surface *texte = NULL;
		TTF_Font *police = NULL;
		police = TTF_OpenFont("arial.ttf", 35);
		SDL_Color couleurNoire = { 0, 0, 0 };
		texte = TTF_RenderText_Blended(police, "tempsActuel", couleurNoire);
		SDL_Rect position;
		position.x = 645;
		position.y = 470;
		SDL_BlitSurface(texte, NULL, fenetre, &position); /*Blit du texte */
		SDL_Flip(fenetre);
		TTF_CloseFont(police); /*Doit être avant TTF_Quit() */

		TTF_Quit();
	}
}

void help()
{
	SDL_Surface *t[7];
	int x = 1;
	t[0] = IMG_Load("Help/1.png");
	t[1] = IMG_Load("Help/2.png");
	t[2] = IMG_Load("Help/3.png");
	SDL_Rect imagebg;
	imagebg.x = 0;
	imagebg.y = 0;
	SDL_BlitSurface(t[0], NULL, fenetre, &imagebg);
	SDL_Flip(fenetre);
	SDL_Event event;

	while (1)
	{
		if (x > 3)
			x = 3;
		while (SDL_PollEvent(&event))

		{

			if (event.type == SDL_QUIT)
			{
				exit(0);
			}
			if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					if (((event.button.x >= 75) && (event.button.x <= 160)) && ((event.button.y >= 39) && (event.button.y <= 142)))
					{
						if (x == 1)
						{
							return;
						}
						else
							x -= 1;
					}
					if (((event.button.x >= 880) && (event.button.x <= 968)) && ((event.button.y >= 536) && (event.button.y <= 626)))
					{
						x += 1;
					}
				}
			}
		}
		switch (x)
		{
			case 1:
				{
					SDL_BlitSurface(t[0], NULL, fenetre, &imagebg);
					SDL_Flip(fenetre);
					break;
				}
			case 2:
				{
					SDL_BlitSurface(t[1], NULL, fenetre, &imagebg);
					SDL_Flip(fenetre);
					break;
				}
			case 3:
				{
					SDL_BlitSurface(t[2], NULL, fenetre, &imagebg);
					SDL_Flip(fenetre);
					break;
				}
		}
	}
}
void about()
{

	SDL_Surface *bg = IMG_Load("About/1.jpeg");
	SDL_Rect imagebg;
	imagebg.x = 0;
	imagebg.y = 0;
	SDL_BlitSurface(bg, NULL, fenetre, &imagebg);
	SDL_Event event;

	while (1)
	{
		while (SDL_PollEvent(&event))

		{

			if (event.type == SDL_QUIT)
			{
				exit(0);
			}
			if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					if (((event.button.x >= 75) && (event.button.x <= 160)) && ((event.button.y >= 39) && (event.button.y <= 142)))
					{
						return;
					}
				}
			}
		}
		SDL_Flip(fenetre);
	}
}
void settings()
{
	int sound = 1;
	SDL_Surface *t[7];
	t[0] = IMG_Load("Settings/1.png");
	t[1] = IMG_Load("Settings/9.png");
	SDL_Rect imagebg;
	imagebg.x = 0;
	imagebg.y = 0;
	if (sound == 1)
		SDL_BlitSurface(t[0], NULL, fenetre, &imagebg);
	else
		SDL_BlitSurface(t[1], NULL, fenetre, &imagebg);
	SDL_Flip(fenetre);
	SDL_Event event;
	while (1)
	{
		while (SDL_PollEvent(&event))

		{

			if (event.type == SDL_QUIT)
			{
				exit(0);
			}
			if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					if (((event.button.x >= 75) && (event.button.x <= 160)) && ((event.button.y >= 39) && (event.button.y <= 142)))
					{
						return;
					}
					if (((event.button.x >= 409) && (event.button.x <= 619)) && ((event.button.y >= 286) && (event.button.y <= 384)) && (sound == 1))
					{
						SDL_BlitSurface(t[0], NULL, fenetre, &imagebg);
						SDL_Flip(fenetre);
						SDL_BlitSurface(t[1], NULL, fenetre, &imagebg);
						SDL_Flip(fenetre);
						Mix_PauseMusic();
						sound = 0;
					}
					else if (((event.button.x >= 409) && (event.button.x <= 619)) && ((event.button.y >= 286) && (event.button.y <= 384)) && (sound == 0))
					{
						SDL_BlitSurface(t[1], NULL, fenetre, &imagebg);
						SDL_Flip(fenetre);
						SDL_BlitSurface(t[0], NULL, fenetre, &imagebg);
						SDL_Flip(fenetre);
						Mix_ResumeMusic();
						sound = 1;
					}
				}
			}
		}
	}
}
void menu()
{int sound;
if(sound==1){

	Music("Ludovico Einaudi - Fly (Visualizer).mp3");}

	int x = 0;
	SDL_Event event;
	SDL_Surface *t[8];
	t[0] = IMG_Load("dmenu.jpeg");
	t[1] = IMG_Load("play.jpeg");
	t[2] = IMG_Load("ranking.jpeg");
	t[3] = IMG_Load("help.jpeg");
	t[4] = IMG_Load("about.jpeg");
	t[5] = IMG_Load("Settings.jpeg");
	t[6] = IMG_Load("Quit.jpeg");

	SDL_Rect imagebg;
	imagebg.x = 0;
	imagebg.y = 0;
	SDL_BlitSurface(t[0], NULL, fenetre, &imagebg);
	SDL_Flip(fenetre);

	while (1)
	{
		while (SDL_PollEvent(&event))

		{

			if (event.type == SDL_QUIT)
			{
				exit(0);
			}
			if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					if (((event.button.x >= 11.5) && (event.button.x <= 267)) && ((event.button.y >= 630) && (event.button.y <= 658)))
					{
						system("START fb.html");
					}
					if (((event.button.x >= 278) && (event.button.x <= 499)) && ((event.button.y >= 630) && (event.button.y <= 658)))
					{
						system("START www.twitter.com");
					}
					if (((event.button.x >= 510) && (event.button.x <= 755)) && ((event.button.y >= 630) && (event.button.y <= 658)))
					{
						system("START www.youtube.com");
					}
				}
			}
			if (event.type == SDL_MOUSEMOTION)
			{
				if ((event.motion.x > 78) && (event.motion.x < 156) && (event.motion.y > 296) && (event.motion.y < 375))
				{
					x = 1;
				}
				if ((event.motion.x > 254) && (event.motion.x < 327) && (event.motion.y > 296) && (event.motion.y < 375))
				{
					x = 2;
				}

				if ((event.motion.x > 418) && (event.motion.x < 490) && (event.motion.y > 296) && (event.motion.y < 375))
				{
					x = 3;
				}
				if ((event.motion.x > 566) && (event.motion.x < 638) && (event.motion.y > 296) && (event.motion.y < 375))
				{
					x = 4;
				}
				if ((event.motion.x > 734) && (event.motion.x < 806) && (event.motion.y > 296) && (event.motion.y < 375))
				{
					x = 5;
				}
				if ((event.motion.x > 886) && (event.motion.x < 960) && (event.motion.y > 296) && (event.motion.y < 375))
				{
					x = 6;
				}
			}
			if (event.type == SDL_MOUSEBUTTONDOWN)
			{
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					switch (x)
					{

						case 1:
							play();
							break;
						case 2:
							ranking();
							break;
						case 3:
							help();
							break;
						case 4:
							about();
							break;
						case 5:
							settings();
							break;
						case 6:
							exit(0);
							break;
					}
				}
			}
			if (event.type == SDL_KEYDOWN)
			{
				if (event.key.keysym.sym == SDLK_RIGHT)
				{
					if (x < 6)
						x = x + 1;
				}
				if (event.key.keysym.sym == SDLK_LEFT)
				{
					if (x > 1)
						x = x - 1;
				}
				else if (((event.key.keysym.sym == SDLK_KP_ENTER) || (event.key.keysym.sym == SDLK_RETURN)))
				{
					switch (x)
					{

						case 1:

							play();

							break;
						case 2:
							help();
							break;
						case 3:
							about();
							break;
						case 4:
							settings();
							break;
						case 5:
							exit(0);
							break;
					}
				}
			}
		}

		SDL_BlitSurface(t[x], NULL, fenetre, &imagebg);
		//SDL_BlitSurface(t[7] ,NULL, fenetre, &imagebg);
		SDL_Flip(fenetre);
	}
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

int main(int argc, char *argv[])
{
	int sound;
	Music("Ludovico Einaudi - Fly (Visualizer).mp3");
	SDL_Init(SDL_INIT_VIDEO);
	fenetre = SDL_SetVideoMode(1024, 668, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
	SDL_WM_SetCaption("Labyrinthe", NULL);
	IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF);

	menu();

	return 0;
}

