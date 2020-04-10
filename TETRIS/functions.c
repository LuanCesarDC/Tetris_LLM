#include "llm_tetris.h"
#include "llm_defs.h"
#include "llm_tetris_menu.h"
#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>

//##################################### Init Functions #####################################

SDL_Window * createWindow(){
	SDL_Window * win = SDL_CreateWindow("LLM_Tetris", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
				     GAME_WINDOW_WIDTH, GAME_WINDOW_HEIGHT, SDL_WINDOW_OPENGL);
	if(win == NULL) printf("Error creating window\n");
	return win;
}

SDL_Renderer * createRenderer(SDL_Window * win){
	SDL_Renderer * rend = SDL_CreateRenderer(win, -1, RENDER_FLAGS);
	if(rend == NULL) printf("Error creating renderer\n");
	return rend;
}

piece * setPieceMatrix(){
	int i, j, k, l = 0;
	piece * p = NULL;
	p = (piece *) calloc(7, sizeof(piece));

	char c[1000];
	FILE * fptr = fopen("pieces", "r");
	fscanf(fptr, "%[^\n]", c);

	for(i = 0;i<7;i++){
		for(j = 0;j<4;j++){
			for(k = 0;k<5;k++){
				for(l = 0;l<5;l++){
					if(c[100*i+25*j+5*k+l] == '1') p[i].mat[j][k][l] = 1;
					if(c[100*i+25*j+5*k+l] == '0') p[i].mat[j][k][l] = 0;
				}
			}
		}
	}
		
	fclose(fptr);
	return p;
}

int ** initBackgroundMatrix(){
	int i, j;
	int ** m = (int **) calloc(ROWS, sizeof(int *));
	for(i = 0;i<ROWS;i++){
		m[i] = (int *)calloc(COLS, sizeof(int *));
	}

	for(i = 0;i<ROWS;i++){
		for(j = 0;j<COLS;j++){
			if(j >= COLS-2 || j <= 1 ) m[i][j] = 4;
			if(i >= ROWS - 4) m[i][j] = 3;
		}
	}

	if(m == NULL) return NULL;

	return m;
}

void clearBackgroundMatrix(int ** bg_matrix){
	int i, j;
	for(i = 0;i<ROWS-4;i++){
		for(j = 2;j<COLS-2;j++){
			bg_matrix[i][j] = 0;
		}
	}
}


int setBackgroundMatrix(int **matrix, int * i, int * j, piece * p, int rot, int * pieceType, int * p_left, int * p_right, int * inc, float * incx, int *p_space){
	int l, m;
	int alert = 0;

	/*
	 * this part of the function goes through the whole background matrix and checks if the piece
	 * would overwrite a fixed block or a floor block in the next frame.
	 * ("1" represents the movable block, "2" represents the fixed block and "3" represents the floor).
	 * if the alert equals 1, the function realizes that it cannot draw the block at this position,
	 * ie it has to go to the next block
	*/
	for(l = *i;l<*i+5;l++){
			for(m = *j;m<*j+5;m++){
				if( p[*pieceType].mat[rot][l-*i][m-*j] == 1 && (matrix[l][m] == 3 || matrix[l][m] == 2)) {
					alert = 1;
					*inc = 0;
					break;
				}
			}
			if(*inc == 0) break;
	}

	/*
	 * This part of the function checks if the Left button has pressed, and if so, it checks if the piece would
	 * overwrite a "2" or a "4" block, if so, it sets p_left to 0 to prevent the movement
	 * ("4" is a wall block.)
	*/
	if(*p_left){
		for(l = *i;l<*i+5;l++){
			for(m = *j;m<*j+3;m++){
				if( p[*pieceType].mat[rot][l-*i][m-*j] == 1 && (matrix[l][m-1] == 4 || matrix[l][m-1] == 2)) {
					*p_left = 0;
					break;
				}
			}
			if(!p_left) break;
		}
	}

	
	/*
	 * do the same as the part above (p_left), but for p_right.
	*/
	if(*p_right){
		for(l = *i;l<*i+5;l++){
			for(m = *j+2;m<*j+5;m++){
				if( p[*pieceType].mat[rot][l-*i][m-*j] == 1 && (matrix[l][m+1] == 4 || matrix[l][m+1] == 2)) {
					*p_right = 0;
					break;
				}
			}
			if(!p_right) break;
		}
	}

	/*
	 * If the first part of the function sets the alert to 1, goes through the whole background matrix and sets the
	 * "1" pieces to "2". After that, sets the i and j (Y = i+incx) at the initial position and randomizes another piece.
     * If alert = 0, it clears all blocks "1" from the backgroud matrix in preparation for the block to be repositioned
	 * to the next frame. After that, after that it sets the location of the next Piece in the matrix to "1".
	 */
	if(alert == 1){	
		*p_space = 0;	
		for(l = 0;l<ROWS;l++){
			for(m = 0;m<COLS;m++){
					if(matrix[l][m] == 1) 
					matrix[l][m] = 2;
			}
		}
		*i = POS_INIT_Y;
		*j = POS_INIT_X;
		//*pieceType = rand()%7;
		*incx = 0;

	}else{
		for(l = 0;l<ROWS;l++){
			for(m = 0;m<COLS;m++){
					if(matrix[l][m] == 1) 
					matrix[l][m] = 0;
			}
		}

		for(l = *i;l<*i+5;l++){
			for(m = *j;m<*j+5;m++){
				if(matrix[l][m] == 0)
					matrix[l][m] = p[*pieceType].mat[rot][l-*i][m-*j];
			}
		}
	}


	
	return alert;
}

void piecesPresent(SDL_Rect * dest, SDL_Renderer * rend, SDL_Texture * tex, SDL_Texture * score_tex, int ** bg_matrix, TTF_Font * font, int score, int nextPiece){
	int i, j;
	char str[50];
	SDL_Surface * surf = NULL, * text = NULL, * next = NULL;
	SDL_Texture * tex2 = NULL, * tex_next = NULL;
	SDL_Rect score_dest, next_dest;
	SDL_Color color_text = {255, 255, 255};

	sprintf(str, "Score: %d", score);

	switch(nextPiece){
		case 0:
			next = IMG_Load("srcs/miniatures/p0.png");
			break;
		case 1:
			next = IMG_Load("srcs/miniatures/p1.png");
			break;
		case 2:
			next = IMG_Load("srcs/miniatures/p2.png");
			break;
		case 3:
			next = IMG_Load("srcs/miniatures/p3.png");
			break;
		case 4:
			next = IMG_Load("srcs/miniatures/p4.png");
			break;
		case 5:
			next = IMG_Load("srcs/miniatures/p5.png");
			break;
		case 6:
			next = IMG_Load("srcs/miniatures/p6.png");
			break;
	}

	
	
	score_dest.w = 100;
	score_dest.h = 100;
	score_dest.x = 395;
	score_dest.y = 20;

	next_dest.w = 80;
	next_dest.h = 80;
	next_dest.x = 395;
	next_dest.y = 200;


	tex_next = SDL_CreateTextureFromSurface(rend, next);
	text = TTF_RenderText_Solid(font, str, color_text);

	surf = IMG_Load("srcs/tiles/t20.png");

	score_tex = SDL_CreateTextureFromSurface(rend, text);
	tex2 = SDL_CreateTextureFromSurface(rend, surf);

	SDL_RenderCopy(rend, score_tex, NULL, &score_dest);
	SDL_RenderCopy(rend, tex_next, NULL, &next_dest);

	for(i = 0;i<ROWS;i++){
			for(j = 0;j<COLS-1;j++){
				if(bg_matrix[i][j] == 1 || bg_matrix[i][j] == 2){
					dest[i+j].w = PIX;
					dest[i+j].h = PIX;
					dest[i+j].x = -ORIGIN_X*PIX + (j+1)*PIX;
					dest[i+j].y = -ORIGIN_Y*PIX + i*PIX;
					SDL_RenderCopy(rend, tex, NULL, &dest[i+j]);
				}
				if(bg_matrix[i][j] == 4 || bg_matrix[i][j] == 3){
					dest[i+j].w = PIX;
					dest[i+j].h = PIX;
					dest[i+j].x = -ORIGIN_X*PIX + (j+1)*PIX;
					dest[i+j].y = -ORIGIN_Y*PIX + i*PIX;
					SDL_RenderCopy(rend, tex2, NULL, &dest[i+j]);
				}
			}
		}
		SDL_RenderPresent(rend);

		SDL_FreeSurface(surf);
		SDL_FreeSurface(text);
		SDL_FreeSurface(next);
		SDL_DestroyTexture(tex_next);
		SDL_DestroyTexture(tex2);
}

int checkRotation(int ** matrix, int i, int j, piece * p, int rot, int * pieceType){
	int l, m;
	int rot_temp;

	if(rot == 3) rot_temp = 0;
	else rot_temp = rot+1;

	for(l = 0;l<5;l++){
			for(m = 0;m<5;m++){
				if( p[*pieceType].mat[rot_temp][l][m] == 1 && (matrix[l+i][m+j] == 4 || matrix[l+i][m+j] == 2)) {
					return rot;
				}
			}
		}

	return rot_temp;
}

void removeLine(int ** matrix, int * score){
	int p = 0, ** mat_aux = NULL, n, * intPointer;
	int i, j;

	mat_aux = (int **) calloc(ROWS, sizeof(int *));
	for(i = 0;i<ROWS;i++){
		mat_aux[i] = (int *)calloc(COLS, sizeof(int *));
	}


	for(i = 0;i<ROWS;i++){
		for(j = 0;j<COLS;j++){
			mat_aux[i][j] = matrix[i][j];
		}
	}

	for(i = 0;i<ROWS;i++){
		for(j = 0;j<COLS;j++){
			if(matrix[i][j] == 2)
				p++;
		}
		if(p == 10){
			for(j = 2;j<COLS-2;j++){
				mat_aux[i][j] = 5;
			}
			
		}
		p = 0;
	}
	n = adjustMatrix(mat_aux);
	*score += 100*n*n;
	
	for(i = 0;i<ROWS;i++){
		for(j = 0;j<COLS;j++){
			matrix[i][j] = mat_aux[i][j];
		}
	}
	for(i = 0;i<ROWS;i++){
	    intPointer = mat_aux[i];
	    free(intPointer);
	}
}

int adjustMatrix(int ** matrix_copy){
	int i, j, l, n = 0;
	
	for(i = 0;i<ROWS;i++){
		for(j = 0;j<COLS;j++){
			if(matrix_copy[i][j] == 5){
				n++;
				while(swapLine(matrix_copy, i) && i != 1){
					i--;
				}
				for(l = 0;l<COLS;l++){
					if(matrix_copy[i-1][l] == 5){ 
						matrix_copy[i-1][l] = 0;
					}
				}
				
			}
		}
	}

	for(i = 0;i<ROWS;i++){
		for(j = 0;j<COLS;j++){
	
		}
	}
			return n;
}

int swapLine(int ** mat_aux, int i){
	int v[COLS], v2[COLS], j, p = 0;

	for(j = 0;j<COLS;j++){
		v[j] = mat_aux[i][j];
		v2[j] = mat_aux[i-1][j];
		if(v2[j] == 2) p = 1;
	}

	for(j = 0;j<COLS;j++){
		mat_aux[i][j] = v2[j];
		mat_aux[i-1][j] = v[j];
	}

	return p;

}

int checkGameOver(int ** matrix){
	int i, j;
	for(i = 0;i<ROWS;i++){
			for(j = 0;j<COLS;j++){
				if(matrix[i][j] == 2 && i == 2 && (j > 2 && j < COLS-2))
					return 1;
			}
		}
	return 0;
}

void checkEvent(int * p_left, int * p_right, int * p_space, int * p_r, int * rotCheck, int * close){
	SDL_Event event;
	while(SDL_PollEvent(&event)){
				switch(event.type){
					case SDL_QUIT:
						*close = 1;
						break;
					case SDL_KEYDOWN:
						if(EVENT_CODE == SDL_SCANCODE_LEFT) *p_left = 1;
						if(EVENT_CODE == SDL_SCANCODE_RIGHT) *p_right = 1;
						if(EVENT_CODE == SDL_SCANCODE_SPACE) *p_space = 1;
						if(EVENT_CODE == SDL_SCANCODE_R && *rotCheck != 1){
							*p_r = 1;
							*rotCheck = 1;
						}
						break;
					case SDL_KEYUP:
						if(EVENT_CODE == SDL_SCANCODE_LEFT) *p_left = 0;
						if(EVENT_CODE == SDL_SCANCODE_RIGHT) *p_right = 0;
						if(EVENT_CODE == SDL_SCANCODE_SPACE) *p_space = 0;
						if(EVENT_CODE == SDL_SCANCODE_R){
							*p_r = 0;
							*rotCheck = 0;
						}
						break;
				}
	}	
}

int gameOverMenu(SDL_Window * win, SDL_Renderer * rend, int score){
	int close = 2;
	char str[50];
	
	sprintf(str, "Score: %d", score);	

	TTF_Font * font = TTF_OpenFont("srcs/fonts/game_over.ttf", 150);
	SDL_Color color_text = {15, 170, 20};
	SDL_Surface * surf = IMG_Load("srcs/menus/game_over.png");
	SDL_Texture * tex = SDL_CreateTextureFromSurface(rend, surf);
	SDL_Surface * text = TTF_RenderText_Solid(font, str, color_text);
	SDL_Texture * tex_text = SDL_CreateTextureFromSurface(rend, text);

	SDL_Rect dest_text;

	dest_text.w = 180;
	dest_text.h = 180;
	dest_text.x = 195;
	dest_text.y = 120;
	
	


	SDL_RenderClear(rend);
	SDL_RenderCopy(rend, tex, NULL, NULL);
	SDL_RenderCopy(rend, tex_text, NULL, &dest_text);
	SDL_RenderPresent(rend);

	while(close == 2){
		close = gameOver();
	}

	SDL_FreeSurface(surf);
	SDL_DestroyTexture(tex);

	return close;
	
}

int gameOver(){
	SDL_Event event;
	while(SDL_PollEvent(&event)){
		switch(event.type){
			case SDL_QUIT:
				return 1;
				break;
			case SDL_KEYDOWN:
				if(EVENT_CODE == SDL_SCANCODE_ESCAPE)
					return 1;
				if(EVENT_CODE == SDL_SCANCODE_RETURN)
					return 0;
		}
	}
	return 2;
}

int initMenu(SDL_Window * win, SDL_Renderer * rend){
	
	SDL_Surface * surf = IMG_Load("srcs/menus/init.png");
	SDL_Texture * tex = SDL_CreateTextureFromSurface(rend, surf);

	SDL_RenderClear(rend);
	SDL_RenderCopy(rend, tex, NULL, NULL);
	SDL_RenderPresent(rend);

	SDL_Event event;
	while(SDL_PollEvent(&event)){
		switch(event.type){
			case SDL_QUIT:
				return 0;
				break;
			case SDL_KEYDOWN:
				if(EVENT_CODE == SDL_SCANCODE_ESCAPE)
					return 0;
				if(EVENT_CODE == SDL_SCANCODE_E)
					return 3;
				if(EVENT_CODE == SDL_SCANCODE_M)
					return 4;
				if(EVENT_CODE == SDL_SCANCODE_H)
					return 5;
				if(EVENT_CODE == SDL_SCANCODE_SPACE)
					return 2;
				if(EVENT_CODE == SDL_SCANCODE_RETURN)
					return 1;
		}
	}
	SDL_FreeSurface(surf);
	SDL_DestroyTexture(tex);
	
	return 6;

}

int creditsMenu(SDL_Window * win, SDL_Renderer * rend){

	SDL_Surface * surf = IMG_Load("srcs/menus/credits.png");
	SDL_Texture * tex = SDL_CreateTextureFromSurface(rend, surf);

	SDL_RenderClear(rend);
	SDL_RenderCopy(rend, tex, NULL, NULL);
	SDL_RenderPresent(rend);
	
	SDL_Event event;
	while(SDL_PollEvent(&event)){
		switch(event.type){
			case SDL_QUIT:
				return 0;
				break;
			case SDL_KEYDOWN:
				if(EVENT_CODE == SDL_SCANCODE_Q)
					return 6;
		}
	}

	SDL_FreeSurface(surf);
	SDL_DestroyTexture(tex);
	
	return 2;

}





