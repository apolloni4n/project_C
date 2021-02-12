#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "constantes.h"
#include <SDL/SDL.h>
#include "SDL_ttf.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "fonctions.h"
void attendre(float temps);
void intialisation();
void cleanup();
void afficher_texte( int x, int y, int taille, char *ch, int R, int G,int B);
void cas1 ();
void cas2 ();
void cas3 ();
void cas4 ();
void cas5 ();
void cas6 ();
void game();
void datesys(char ch[]);
void menu();
void ranking();

typedef uint8_t **     laby_t;         	 //tableau deux dimensions
typedef uint8_t *      mrow_t;
Uint32 SDL_Ticks(Uint32 interval, void * unused)
{
    SDL_Event tick;
    memset(&tick, 0, sizeof tick);
    tick.type = SDL_USEREVENT;
    SDL_PushEvent(&tick);
    return interval;
}
void SDL_DrawLine(SDL_Surface * ecran, uint32_t color, int x, int y, int x2, int y2)
{
    SDL_Rect r = {x, y, x2 - x + 1, y2 - y + 1};
    SDL_FillRect(ecran, &r, color);
}


laby_t alloc_laby(int w, int h)
{
    laby_t laby = (laby_t)calloc( 1, w * h + h * sizeof(laby));
    mrow_t mem  = (mrow_t) (laby + h);
    int    i;
    memset(mem, LABF_NSOE, w * h);
    for (i = 0; i < h; laby[i] = mem, mem += w, i ++);
    return laby;
}
/*fonction qui cree le labyrinthe aleatoirement et le résoud automatiquement*/
void jouer(){

Music("Ludovico Einaudi - Lady Labyrinth.mp3");
    // Initialisation des nombres aléatoires
    srand(time(NULL));
    //Définition des variables
    int w;                    // largeur du labyrinthe (verticale)
    int h;                    // Longeur du labyrinthe (horizontale)
    int t,i,j;                // Compteurs
    int xs, xs1, ys1, ys;     // Position de départ
    int xe, ye;               // Position de fin
    w =  40;
    h = 40;
    fprintf(stderr, "Generation du labyrinthe %dx%d...\n", w, h);
    // Sélection d'un point d'entrée et de sortie
    switch (rand() % 2) {
    case 0: ys = 0; xs = rand() % w; ye = h - 1; xe = rand() % w; i = LABF_NORD;  break;
    case 1: xs = 0; ys = rand() % h; xe = w - 1; ye = rand() % h; i = LABF_OUEST;break;

    }

    //Création du labyrinthe : matrice de cases
    laby_t laby  = alloc_laby(w, h);
    int *  stack = (int*)calloc( w * h, sizeof(stack));
    laby[ys][xs] &= ~i;
    laby[ys1][xs1] &= ~i;
    laby[ye][xe] &= ~opposite[i];
    stack[0] = MAKE_POS(xs, ys);
    //Génération des portes via algo de backtracking récursif.
    for (i = 0, j = 1; j > 0; i ++)
    {
        int x = stack[j - 1] & 0xffff;
        int y = stack[j - 1] >> 16;
        int f = 0;
        laby[y][x] |= LABF_VISIT;
        //On visite toutes les cellules avoisinantes de manière alélatoire
        while (f != LABF_NSOE)
        {
            int dir = 1 << (rand() % 4); //Choisi une direction parmi les 4 possibles
            int nx, ny;
            //Ajuste si on est déjà passé par là
            if (f & dir)
            {
                if ((f & (dir-1)) < dir-1) while (f & dir) dir >>= 1;
                else                       while (f & dir) dir <<= 1;
            }
            switch (dir) {
            case LABF_NORD:  nx = x; ny = y - 1; break;
            case LABF_EST:   ny = y; nx = x + 1; break;
            case LABF_SUD:   nx = x; ny = y + 1; break;
            case LABF_OUEST: ny = y; nx = x - 1; break;
            }
            //Dèjà visité ou visitable ?
            if (0 <= nx && nx < w && 0 <= ny && ny < h && ! (laby[ny][nx] & LABF_VISIT))
            {
                //Visite cette cellule, en créant un chemin de (x, y) à (nx, ny)
                laby[y][x] &= ~dir;
                laby[ny][nx] &= ~opposite[dir];
                stack[j++] = MAKE_POS(nx, ny);

                break;
            }
            else f |= dir;
        }
        //On a visité toutes les cellules avoisinante: backtrack
        if (f == LABF_NSOE) j --;
    }
    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    //===================Dessin=======================
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
    SDL_Surface * ecran = SDL_SetVideoMode(BLOC_W * w + 30, BLOC_H * h + 50, 32, SDL_HWSURFACE);
    SDL_WM_SetCaption("Labyrinthe", NULL);
    SDL_TimerID   timer;

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    //++++++++++++++Coloriage du fond++++++++++++++++++++
    uint32_t back = SDL_MapRGB(ecran->format, 0,   0, 0);
    uint32_t vert = SDL_MapRGB(ecran->format, 60, 197, 212);
    uint32_t white = SDL_MapRGB(ecran->format, 255, 255, 255);
    uint32_t dead = SDL_MapRGB(ecran->format, 183, 183, 183);
    SDL_FillRect(ecran, NULL, back);
    /*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

    //==================Affichage du labyrinthe=================
   SDL_Surface *background = NULL;
        background= IMG_Load("auroora.jpeg");
         SDL_Rect position;
        position.x=0;
            position.y=0;
        SDL_BlitSurface(background, NULL, ecran, &position);

    SDL_UpdateRect(ecran, 0, 0, 0, 0);
    for (j = 0; j < h; j++)
    {
        for (i = 0; i < w; i++)
        {
            int f  = laby[j][i];
            int x  = 10 + i * BLOC_W;
            int y  = 10 + j * BLOC_H;
            int x2 = x + BLOC_W - 1;
            int y2 = y + BLOC_H - 1;
            if (j > 0) f &= ~LABF_NORD;
            if (i > 0) f &= ~LABF_OUEST;
            laby[j][i] &= ~LABF_VISIT;
            // Placement de points aux angles des murs pour avoir un dessin continu
            if (f & LABF_NORD)  SDL_DrawLine(ecran, white, x,  y,  x2, y);
            if (f & LABF_EST)   SDL_DrawLine(ecran, white, x2, y,  x2, y2);
            if (f & LABF_SUD)   SDL_DrawLine(ecran, white, x,  y2, x2, y2);
            if (f & LABF_OUEST) SDL_DrawLine(ecran, white, x,  y,  x,  y2);
        }
    }


    timer = SDL_AddTimer(100, SDL_Ticks, NULL);
    //=====================Fin du dessin==============================
    //===================Parcours=====================================
    stack[0] = MAKE_POS(xs, ys);
    j = 1;
    while (! (xs == xe && ys == ye) && j > 0)
    {
        SDL_Event event;
        SDL_WaitEvent(&event);
        switch (event.type) {
        case SDL_KEYDOWN:
            if (event.key.state != SDL_PRESSED || event.key.keysym.sym != SDLK_ESCAPE)
                continue;
        case SDL_QUIT:
        Mix_PauseMusic();
        j = -1;
             continue;
        case SDL_USEREVENT:
            break;
        default:
            continue;
        }

        int x = stack[j - 1] & 0xffff;
        int y = stack[j - 1] >> 16;
        int f = laby[y][x] & ~LABF_VISIT;
        SDL_Rect path = {10 + x * BLOC_W, 10 + y * BLOC_H, BLOC_W, BLOC_H};
        if (f & LABF_NORD)  path.y += 2, path.h -= 2;
        if (f & LABF_OUEST) path.x += 2, path.w -= 2;
        if (f & LABF_SUD)   path.h -= 2;
        if (f & LABF_EST)   path.w -= 2;
        SDL_Surface*  bmp_image= IMG_Load("objectif1.bmp");
        SDL_FillRect(ecran, &path, white);

    SDL_UpdateRect(ecran, 0, 0, 0, 0);


        laby[y][x] |= LABF_VISIT;
        xs = x; ys = y;

        //Avance dans le labyrinthe
        while (f != LABF_NSOE)
        {
            int dir = 1 << (rand() % 4); //Choisi une direction parmi les 4 possibles
            int nx, ny;
            //Ajuste si on est déjà passé par là
            if (f & dir)
            {
                if ((f & (dir-1)) < dir-1) while (f & dir) dir >>= 1;
                else                       while (f & dir) dir <<= 1;
            }
            switch (dir) {
            case LABF_NORD:  nx = x; ny = y - 1; break;
            case LABF_EST:   ny = y; nx = x + 1; break;
            case LABF_SUD:   nx = x; ny = y + 1; break;
            case LABF_OUEST: ny = y; nx = x - 1; break;
            }

            //Dèjà visité ou visitable ?
            if (0 <= nx && nx < w && 0 <= ny && ny < h && ! (laby[ny][nx] & LABF_VISIT))
            {
                stack[j++] = MAKE_POS(nx, ny);
                break;
            }
            else f |= dir; t ++;
        }
        if (f == LABF_NSOE)
        {
            SDL_FillRect(ecran, &path, dead);            j --;
        }


   }

    SDL_RemoveTimer(timer);


    //====================Fin du programme============================
    if (j == 0)
    {
        fprintf(stderr, "LAbyrinthe résolu...\n" );
        pause();
    }

    SDL_FreeSurface(ecran);
    Mix_CloseAudio();
        Mix_PauseMusic();

    SDL_Surface *fenetre;
SDL_Init( SDL_INIT_VIDEO );
    fenetre = SDL_SetVideoMode(1024, 668, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);
    SDL_WM_SetCaption("Labyrinthe", NULL);
    IMG_Init(IMG_INIT_JPG|IMG_INIT_PNG|IMG_INIT_TIF);
   }

/*....................................................*/

void pause()
{
    int continuer = 1;
    SDL_Event event;
    while(continuer)
    {
        SDL_WaitEvent(&event);
        switch(event.type) {
        case SDL_KEYDOWN:
        case SDL_QUIT: continuer = 0;

        }
    }
    Mix_CloseAudio();

}



/* ......................................................*/


/* ....................................................*/


/* fonction qui cree de la musiue en arriere plan du jeu et du menu*/

void Music(char *music){
 Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,MIX_DEFAULT_CHANNELS,1024);
     Mix_Music *musique;
     musique = Mix_LoadMUS(music);
     Mix_VolumeMusic(MIX_MAX_VOLUME/3);
     Mix_PlayMusic(musique, -1);
     if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024) == -1)                                 //Initialisation de l'API Mixer
   {
      printf("%s", Mix_GetError());
   }}
/*...............................................*/
void settings()
{SDL_Surface* fenetre;

int sound=1;

SDL_Surface* t [7];
t[0]= IMG_Load("Settings/1.png");
t[1]= IMG_Load("Settings/9.png");
    SDL_Rect imagebg;
    imagebg.x = 0;
    imagebg.y = 0;
    if(sound==1)
    SDL_BlitSurface( t[0] ,NULL, fenetre, &imagebg );
    else
    SDL_BlitSurface( t[1] ,NULL, fenetre, &imagebg );
    SDL_Flip(fenetre);
    SDL_Event event;
    while(1)
    {
        while (SDL_PollEvent(&event))

        {
            if (event.type == SDL_QUIT)
            {
                exit(0);
            }
            if(event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    if (((event.button.x >= 75) && (event.button.x <= 160))&&((event.button.y >= 39) && (event.button.y <= 142)))
                    {
                        return ;
                    }
                    if(((event.button.x >= 409) && (event.button.x <= 619))&&((event.button.y >= 286) && (event.button.y <= 384))&&(sound==1))
                    {
                        SDL_BlitSurface( t[0] ,NULL, fenetre, &imagebg );
                        SDL_Flip(fenetre);
                        SDL_BlitSurface( t[1] ,NULL, fenetre, &imagebg );
                        SDL_Flip(fenetre);
                        Mix_CloseAudio();
                         Mix_PauseMusic();
                        sound=0;


                        }
                        else if (((event.button.x >= 409) && (event.button.x <= 619))&&((event.button.y >= 286) && (event.button.y <= 384))&&(sound==0))
                    {
                        SDL_BlitSurface( t[1] ,NULL, fenetre, &imagebg );
                        SDL_Flip(fenetre);
                        SDL_BlitSurface( t[0] ,NULL, fenetre, &imagebg );
                        SDL_Flip(fenetre);
                        Music("The Maze Runner Soundtrack - 01. The Maze Runner.mp3");
                        sound=1;
                        }
                    }
                }
            }
        }
}

/*............................................................*/
void about()
{SDL_Surface* fenetre;


    SDL_Surface* bg = IMG_Load("About/1.png");
    SDL_Rect imagebg;
    imagebg.x = 0;
    imagebg.y = 0;
    SDL_BlitSurface( bg ,NULL, fenetre, &imagebg );
    SDL_Event event;

    while(1)
    {
        while (SDL_PollEvent(&event))

        {
            if (event.type == SDL_QUIT)
            {
                exit(0);
            }
            if(event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    if (((event.button.x >= 75) && (event.button.x <= 160))&&((event.button.y >= 39) && (event.button.y <= 142)))
                    {
                        return ;
                    }
                }
            }
        }
        SDL_Flip(fenetre);
    }

}
/*...............................................................*/
void help()
{SDL_Surface* fenetre;

SDL_Surface* t [7];
int x=1;
t[0]= IMG_Load("Help/1.png");
t[1]= IMG_Load("Help/2.png");
t[2]= IMG_Load("Help/3.png");
    SDL_Rect imagebg;
    imagebg.x = 0;
    imagebg.y = 0;
    SDL_BlitSurface( t[0] ,NULL, fenetre, &imagebg );
    SDL_Flip(fenetre);
    SDL_Event event;

    while(1)
    {
        if(x>3)
            x=3;
        while (SDL_PollEvent(&event))

        {
            if (event.type == SDL_QUIT)
            {
                exit(0);
            }
            if(event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    if (((event.button.x >= 75) && (event.button.x <= 160))&&((event.button.y >= 39) && (event.button.y <= 142)))
                    {
                        if(x==1)
                            {
                                return ;
                    }
                        else
                        x-=1;
                    }
                    if (((event.button.x >= 880) && (event.button.x <= 968))&&((event.button.y >= 536) && (event.button.y <= 626)))
                    {
                        x+=1;
                    }
                    }
                }
            }
            switch(x)
                        {
                        case 1:
                            {
                                SDL_BlitSurface( t[0] ,NULL, fenetre, &imagebg );
                        SDL_Flip(fenetre);
                        break;
                            }
                            case 2:
                            {
                                SDL_BlitSurface( t[1] ,NULL, fenetre, &imagebg );
                        SDL_Flip(fenetre);
                        break;

                            }
                                                        case 3:
                            {
                                SDL_BlitSurface( t[2] ,NULL, fenetre, &imagebg );
                        SDL_Flip(fenetre);
                        break;
                        }
                        }
        }
}

/*.....................................................*/
void ranking()
{SDL_Surface* fenetre;

    SDL_Surface* bg = IMG_Load("Fichiers/Menu/rankingtable1.png");
    SDL_Rect imagebg;
    imagebg.x = 0;
    imagebg.y = 0;
    SDL_BlitSurface( bg ,NULL, fenetre, &imagebg );
    SDL_Event event;

    while(1)
    {
        while (SDL_PollEvent(&event))

        {
            if (event.type == SDL_QUIT)
            {
                exit(0);
            }
            if(event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    if (((event.button.x >= 75) && (event.button.x <= 160))&&((event.button.y >= 39) && (event.button.y <= 142)))
                    {
                        return;
                    }
                }
            }
        }
        SDL_Flip(fenetre);
    }

}
/*............................................................................................................................................................................................*/
void menu()
{
SDL_Surface* fenetre;
int sound=1;
SDL_Init( SDL_INIT_VIDEO );
    fenetre = SDL_SetVideoMode(1024, 668, 32, SDL_HWSURFACE|SDL_DOUBLEBUF);
    SDL_WM_SetCaption("Labyrinthe", NULL);
    IMG_Init(IMG_INIT_JPG|IMG_INIT_PNG|IMG_INIT_TIF);
    TTF_Init();
  Music("The Maze Runner Soundtrack - 01. The Maze Runner.mp3");

    int x=0;
    SDL_Event event;
    SDL_Surface* t [8];
    t[0]=IMG_Load("dmenu.png");
    t[1]=IMG_Load("play.png");
    t[2]=IMG_Load("ranking.png");
    t[3]=IMG_Load("help.png");
    t[4]=IMG_Load("about.png");
    t[5]=IMG_Load("Settings.png");
    t[6]=IMG_Load("Quit.png");

    SDL_Rect imagebg;
    imagebg.x = 0;
    imagebg.y = 0;
    SDL_BlitSurface( t[0] ,NULL, fenetre, &imagebg );
    SDL_Flip(fenetre);


    while(1)
    {
        while (SDL_PollEvent(&event))

        {
            if (event.type == SDL_QUIT)
            {
                exit(0);
            }
            if(event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    if (((event.button.x >= 11.5) && (event.button.x <= 267))&&((event.button.y >= 630) && (event.button.y <= 658)))
                    {
                        system("START fb.html");
                    }
                    if (((event.button.x >= 278) && (event.button.x <= 499))&&((event.button.y >= 630) && (event.button.y <= 658)))
                    {
                        system("START www.twitter.com");
                    }
                    if (((event.button.x >= 510) && (event.button.x <= 755))&&((event.button.y >= 630) && (event.button.y <= 658)))
                    {
                        system("START www.youtube.com");
                    }
                }
            }
            if(event.type == SDL_MOUSEMOTION)
            {
                    if((event.motion.x>78)&&(event.motion.x<156)&&(event.motion.y>296)&&(event.motion.y<375))
                    {
                    x=1;
                    }
                    if((event.motion.x>254)&&(event.motion.x<327)&&(event.motion.y>296)&&(event.motion.y<375))
                    {
                    x=2;
                    }

                    if((event.motion.x>418)&&(event.motion.x<490)&&(event.motion.y>296)&&(event.motion.y<375))
                    {
                    x=3;
                    }
                    if((event.motion.x>566)&&(event.motion.x<638)&&(event.motion.y>296)&&(event.motion.y<375))
                    {
                    x=4;
                    }
                    if((event.motion.x>734)&&(event.motion.x<806)&&(event.motion.y>296)&&(event.motion.y<375))
                    {
                    x=5;
                    }
                    if((event.motion.x>886)&&(event.motion.x<960)&&(event.motion.y>296)&&(event.motion.y<375))
                    {
                    x=6;
                    }

            }
            if(event.type == SDL_MOUSEBUTTONDOWN)
            {
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    switch (x)
                    {


                    case 1:
                    jouer();
                    break;
                    case 2:
                    help();
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
                if(event.key.keysym.sym == SDLK_RIGHT)
                {
                    if(x<6)
                        x=x+1;
                }
                if(event.key.keysym.sym == SDLK_LEFT)
                {
                    if(x>1)
                        x=x-1;
                }
                else if (((event.key.keysym.sym == SDLK_KP_ENTER)||(event.key.keysym.sym == SDLK_RETURN)))
                {
                    switch (x)
                    {

                    case 1:

                       jouer();

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

        SDL_BlitSurface( t[x] ,NULL, fenetre, &imagebg);
        //SDL_BlitSurface( t[7] ,NULL, fenetre, &imagebg);
        SDL_Flip(fenetre);

    }

}
/*................................................*/
void datesys(char ch[])
{
    time_t rawtime;
    struct tm * timeinfo;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    strcpy(ch, asctime (timeinfo));
}



























