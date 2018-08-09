// cast.cpp : Defines the entry point for the application.
//

#define SDL_MAIN_HANDLED

#include "cast.h"
#include "SDL.h"

const int WIDTH = 800;
const int HEIGHT = 600;

bool respondToKey(const SDL_Event& event) {
	bool def = false;

	switch (event.key.keysym.sym) {
		case SDLK_ESCAPE:
			return true;
	}
	return def;
}

int main() {
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Event event;

	SDL_Window* window = SDL_CreateWindow("cast", SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	bool quit = false;

	while (!quit) {
		const Uint8* keys;

		while (SDL_PollEvent(&event)) {
			SDL_Delay(5);
			keys = SDL_GetKeyboardState(0);

			if (event.type == SDL_QUIT) {
				quit = true;
			}
			else if (event.type == SDL_KEYDOWN) {
				quit = respondToKey(event);
			}
		}
	}

	return 0;
}
