// cast.cpp : Defines the entry point for the application.
//

#define SDL_MAIN_HANDLED

#include "cast.h"
#include "../include/SDL.h"

const int WIDTH = 800;
const int HEIGHT = 600;

int main()
{
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window* window = SDL_CreateWindow("cast", SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	while (true);

	return 0;
}
