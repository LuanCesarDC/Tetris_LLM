#ifndef _LLM_TETRIS_H_
#define _LLM_TETRIS_H_

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_ttf.h"
#include "SDL2/SDL_timer.h"


typedef struct {
	int pos;
	int mat[4][5][5];
} piece;



/*
 * Creates a SDL_Window with defaults already defined by llm_defs.h.
 */
SDL_Window * createWindow();

/*
 * Creates a SDL_Renderer with defaults already defined by llm_tetris.h.
 * @param * win is a SDL_Window from SDL2 library, that will create
 * an interactive window.
 */
SDL_Renderer * createRenderer(SDL_Window * win);

/*
 * Gets the "pieces" file, and sets it to a pointer.
*/
piece * setPieceMatrix();

/*
 * Clean the Background Matrix to init a new game.
 */
void clearBackgroundMatrix(int ** bg_matrix);

/*
 *This function prepares an matrix (ROWS, COLS) that will be used as a grid pixel (30x30) in the game backend.
*/
int ** initBackgroundMatrix();

/*
 *This is the most complex function of the game.
 *it makes almost every interaction with the backend pixel grid.
 *Controls wheter or not the piece can be moved or rotated
 * @param ** matrix is the backend matrix that will simulate a 30x30 pixel grid
 * @param i is the Y coordinate of the current block int the matrix
 * @param j is the X coordinate of the current block int the matrix
 * @param * p is the pointer for the "piece" vector that was created by setPieceMatrix() function
 * @param rot is the variable that contains de current rotation of the piece (each piece has four rotations)
 * @param * pieceType is a pointer for the current piece type
 * @param * p_left is a pointer that tells you if the left arrow button has been pressed
 * @param * p_right is a pointer that tells you if the right arrow button has been pressed
 * @param * inc is a pointer to the "inc" variable, that contains the number of pixels the piece
 *has already dropped
 * @param * incx is a pointer to the "incx" variable, that contains the distance between the x coordinate
 *of the piece and the x coordinate of the origin
 * @param * p_space is a pointer that tells you if the space bar has been pressed
 */
int setBackgroundMatrix(int ** matrix, int * i, int * j, piece * p, int rot, int *pieceType, int * p_left, int * p_right, int * inc, float * incx, int * p_space);


/*
 * Controls the SDL_Rect's vector that will be used as destination for each game block.
 * @param * dest is a pointer to a SDL_Rect which will give the destination for each piece in the background grid
 * @param * rend is a renderer created by the createRenderer() function
 * @param * tex is a texture that will be rendered in the window (background image and pieces)
 * @param ** bg_matrix is the backend matrix that simulates a 30x30 pixel grid (each newPixel is a 30x30 real pixel)
 */
void piecesPresent(SDL_Rect * dest, SDL_Renderer * rend, SDL_Texture * tex, SDL_Texture * score_tex, int ** bg_matrix, TTF_Font * font, int score, int nextPiece);

/*
 * Checks wheter or not the piece can rotate, and returns the next rotation if it is allowed, if not, returns current rotation
 * @param ** matrix is the backend matrix that simulates a 30x30 pixel grid (each newPixel is a 30x30 real pixel)
 * @param * rend is a renderer created by the createRenderer() function
 * @param * tex is a texture that will be rendered in the window (background image and pieces)
 * @param * pieceType is a pointer for the current piece type
 */
int checkRotation(int **matrix, int i, int j, piece * p, int rot, int * pieceType);


/*
 * When a line is completed with blocks, this function removes them.
 * In addition, the score is incremented here
 * @param ** matrix is the backend matrix that simulates a 30x30 pixel grid (each newPixel is a 30x30 real pixel)
 * @param * score is the score of the game
 */
void removeLine(int ** matrix, int * score);

/*
 * Checks whether the requirements for changing lines have been met
 * @param ** mat_copy is a copy of the background matrix
 */
int adjustMatrix(int ** matrix_copy);

/*
 *exchange content between one line of the background matrix and the one above it
 *@param ** mat_aux is a copy of the background matrix
 *@param i is the Y coordinate where occours the exchange
 */
int swapLine(int ** mat_aux, int i);

int checkGameOver(int ** matrix);

void checkEvent(int * p_left, int * p_right, int * p_space, int * p_r, int * rotCheck, int * close);

/*
//MENU

_Bool menuGetStatus(struct menu m);
_Bool menuGetLevel_menu(struct menu m);
_Bool menuGetCredits(struct menu m);

void menuSetStatus(struct menu *m, _Bool ss);
void menuSetLevel_menu(struct menu *m, _Bool lvv);
void menuSetCredits(struct menu *m, _Bool yy);
*/




#endif //_LLM_TETRIS_H_
