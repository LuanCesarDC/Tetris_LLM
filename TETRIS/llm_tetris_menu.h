#ifndef LLM_TETRIS_MENU_H
#define LLM_TETRIS_MENU_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_timer.h>

int gameOver();

int gameOverMenu(SDL_Window * win, SDL_Renderer * rend, int score);

int initMenu(SDL_Window * win, SDL_Renderer * rend);

int creditsMenu(SDL_Window * win, SDL_Renderer * rend);


#endif //LLM_TETRIS_MENU_H
