#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "llm_tetris.h"
#include "llm_defs.h"
#include "llm_tetris_menu.h"
#include "SDL2/SDL.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>


int main(int argc, char *argv[]){
	SDL_Window * win = NULL;
	SDL_Renderer * rend = NULL;
	SDL_Surface * surf = NULL;
	SDL_Surface * bg = NULL;
	SDL_Surface * text = NULL;
	SDL_Texture * tex = NULL, * score_tex = NULL;
	SDL_Texture * tex_bg = NULL;
	SDL_Color color_text = {255, 255, 255};
	TTF_Font * font = NULL;
	//game * game = NULL;
	piece * p = NULL;
	SDL_Rect dest[ROWS*COLS];

	int ** bg_matrix = NULL, rot = 0, randPiece1, randPiece2,
	    inc = 0, counter = 0, alert = 0,
		close = 0, rotCheck = 0, vel = VEL,
		score = 0, color = 0, level = EASY, close_menu, option = 6, lv = EASY;

	int posx = POS_INIT_X, posy = POS_INIT_Y;
	float incx = 0;
	char tst[20];

//-----------------INITIALIZATION----------------------------//
	if(SDL_Init(SDL_INIT_VIDEO) != 0){
		printf("ERROR SDL_INIT: %s\n", SDL_GetError());
		return 1;
	}
	if(TTF_Init() != 0){
		printf("ERROR TTF_INIT: %s\n", SDL_GetError());
		return 1;
	}
	bg_matrix = initBackgroundMatrix();
	if(bg_matrix == NULL){
		printf("ERROR BG_MATRIX: %s\n", SDL_GetError());
		return 1;
	}
	win = createWindow();
	if(win == NULL){
		printf("ERROR WINDOW: %s\n", SDL_GetError());
		return 1;
	}
	rend = createRenderer(win);
	if(rend == NULL){
		printf("ERROR RENDERER: %s\n", SDL_GetError());
		return 1;
	}
	p = setPieceMatrix();
	if(p == NULL){
		printf("ERROR PIECE MATRIX: %s\n", SDL_GetError());
		return 1;
	}
	font = TTF_OpenFont("srcs/fonts/game_over.ttf", 100);
	if(font == NULL){
		printf("ERROR FONT: %s\n", SDL_GetError());
		return 1;
	}
//-----------------------------------------------------------//

	text = TTF_RenderText_Solid(font, "Score: 0", color_text);
	SDL_BlitSurface(text, NULL, bg, NULL);


	srand(time(0));
	randPiece1 = rand()%7;
	randPiece2 = rand()%7;
	color = rand()%19;
	sprintf(tst, "srcs/tiles/t%d.png", color+1);



	bg = IMG_Load("srcs/galaxy.png");
	//game = createGame(level);

	tex_bg = SDL_CreateTextureFromSurface(rend, bg);

	int p_left = 0;
	int p_right = 0;
	int p_space = 0;
	int p_r = 0;


	while(!close_menu){
		close = close_menu;
		clearBackgroundMatrix(bg_matrix);
		score = 0;

		while(option != 1 && option != 0){
			option = initMenu(win, rend);
			while(option == 2)
				option = creditsMenu(win, rend);

			if(option == 3){
				lv = EASY;
			}else if(option == 4){
				lv = MEDIUM;
			}else if(option == 5){
				lv = HARD;
			}
		}

		if(option == 0){
			close = 2;
		}

		level = lv;
		option = 6;
	//------------------------------ Game Loop ---------------------------- //
		while(close != 1 && close != 2){

			checkEvent(&p_left, &p_right, &p_space, &p_r, &rotCheck, &close);

			surf = IMG_Load(tst);
			tex = SDL_CreateTextureFromSurface(rend, surf);


			SDL_RenderClear(rend);
			SDL_RenderCopy(rend, tex_bg, NULL, NULL);
			counter++;

			piecesPresent(dest, rend, tex, score_tex, bg_matrix, font, score, randPiece2);

			alert = setBackgroundMatrix(bg_matrix, &posy, &posx, p, rot, &randPiece1, &p_left, &p_right, &inc, &incx, &p_space);

			if(alert == 1){
				randPiece1 = randPiece2;
				randPiece2 = rand()%7;
			}

			if(p_r){
				rot = checkRotation(bg_matrix, posy, posx, p, rot, &randPiece1);

				p_r = 0;
			}

			removeLine(bg_matrix, &score);

			if(counter >= FPS/level ){
				counter = 0;
				inc++;
			}
			if(p_left){
				vel = -VEL;
			}

			if(p_right){
				vel = VEL;
			}

			if(!p_right && !p_left)
				vel = 0;

			if(!p_space){
				level = lv;
			}else{
				level = 40;
			}

			incx += vel;

			posx = POS_INIT_X + (int)incx / 30;
			posy = POS_INIT_Y + inc;

			if(checkGameOver(bg_matrix)){
				close = 1;
			}

			SDL_Delay(1000/FPS);
		}
		// --------------------------End_Game_Loop------------------------------------- //
		if(close != 2)
			close_menu = gameOverMenu(win, rend, score);
		else
			close_menu = 1;
	}

	if(win != NULL) SDL_DestroyWindow(win);
	if(tex_bg != NULL) 	SDL_DestroyTexture(tex_bg);
	if(tex != NULL) 	SDL_DestroyTexture(tex);
	if(score_tex != NULL) 	SDL_DestroyTexture(score_tex);
	if(text != NULL) SDL_FreeSurface(text);

	free(p);
	free(font);
	SDL_DestroyRenderer(rend);
	SDL_DestroyTexture(tex);
	SDL_FreeSurface(surf);
	SDL_FreeSurface(bg);
	SDL_Quit();
	TTF_Quit();


	return 0;
}

