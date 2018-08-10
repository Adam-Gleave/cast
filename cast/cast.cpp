// cast.cpp : Defines the entry point for the application.
//

#define SDL_MAIN_HANDLED
#define WIDTH 800
#define HEIGHT 600

#include "cast.h"
#include "camera.h"
#include "world.h"
#include "SDL.h"

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

    Camera camera;
    World world;

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

        for (int x = 0; x < WIDTH; x++) {
            double x_camera_space = (2.0f*x / WIDTH) - 1.0f;
            double ray_x_direction = camera.xdir + x_camera_space*0.0f;
            double ray_y_direction = camera.ydir + x_camera_space*FOV;

            // Position in the map
            int map_x = static_cast<int>(camera.xpos);
            int map_y = static_cast<int>(camera.ypos);

            // Direction to step
            int step_x = (ray_x_direction >= 0) ? 1 : -1;
            int step_y = (ray_y_direction >= 0) ? 1 : -1;

            // Distance from side to next corresponding side
            double delta_to_x = std::abs(1 / camera.xdir);
            double delta_to_y = std::abs(1 / camera.ydir);

            // Distance from position to nearest side
            double dist_to_x = (ray_x_direction >= 0) 
                ? (map_x + 1.0f - camera.xpos) * delta_to_x 
                : (camera.xpos - map_x) * delta_to_x;
            double dist_to_y = (ray_y_direction >= 0)
                ? (map_y + 1.0f - camera.ypos) * delta_to_y
                : (camera.ypos - map_y) * delta_to_y;

            // Ray collision
            int hit = 0;
            int hit_side;

            // DDA Algorithm
            while (!hit) {
                // Advance ray to next intersection
                if (dist_to_x < dist_to_y) {
                    dist_to_x += delta_to_x;
                    map_x += step_x;
                    hit_side = 0;
                }
                else {
                    dist_to_y += delta_to_y;
                    map_y += step_y;
                    hit_side = 1;
                }

                // Check ray collision
                hit = world.getMapAt(map_x, map_y);
            }

            if (hit_side == 0) {

            }
            else {

            }
        }
    }

	return 0;
}
