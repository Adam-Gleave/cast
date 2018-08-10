// cast.cpp : Defines the entry point for the application.
//

#define SDL_MAIN_HANDLED
#define WIDTH 800
#define HEIGHT 600

#include "cast.h"
#include "camera.h"
#include "world.h"
#include "SDL.h"

Camera camera;
World world;

double time = 0.0f;
double prev_time = 0.0f;

bool toQuit(const SDL_Event& event) {
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
				quit = toQuit(event);
			}
		}

        for (int x = 0; x < WIDTH; x++) {
            double x_camera_space = 2.0f * x / WIDTH - 1.0f;
            double ray_x_direction = camera.xdir + x_camera_space*camera.xplane;
            double ray_y_direction = camera.ydir + x_camera_space*camera.yplane;

            // Position in the map
            int map_x = static_cast<int>(camera.xpos);
            int map_y = static_cast<int>(camera.ypos);

            // Direction to step
            int step_x = (ray_x_direction >= 0) ? 1 : -1;
            int step_y = (ray_y_direction >= 0) ? 1 : -1;

            // Distance from side to next corresponding side
            double delta_to_x = std::abs(1 / ray_x_direction);
            double delta_to_y = std::abs(1 / ray_y_direction);

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

            // Wall distance in camera direction
            double wall_dist = (hit_side == 0)
                ? wall_dist = (map_x - camera.xpos + (1 - step_x) / 2) / ray_x_direction
                : wall_dist = (map_y - camera.ypos + (1 - step_y) / 2) / ray_y_direction;
        
            int line_height = static_cast<int>(HEIGHT / wall_dist);

            int draw_start = -line_height/2 + HEIGHT/2;
            if (draw_start < 0) {
                draw_start = 0;
            }

            int draw_end = line_height/2 + HEIGHT/2;
            if (draw_end > HEIGHT) {
                draw_end = HEIGHT - 1;
            }

            struct Color {
                Uint8 r;
                Uint8 g;
                Uint8 b;

                Color(Uint8 r, Uint8 g, Uint8 b) : r(r), g(g), b(b) { }
            };

            Color color_x(200, 200, 200);
            Color color_y(140, 140, 140);

            if (hit_side == 0) {
                SDL_SetRenderDrawColor(renderer, color_x.r, color_x.g, color_x.b, 256);
            }
            else {
                SDL_SetRenderDrawColor(renderer, color_y.r, color_y.g, color_y.b, 256);
            }
            SDL_RenderDrawLine(renderer, x, draw_start, x, draw_end);
        }

        time = SDL_GetTicks();
        double frame_time = (time - prev_time)/1000.0;
        prev_time = time;
        double move_speed = frame_time * 5.0f;
        double rot_speed = frame_time * 3.0f;

        SDL_RenderPresent(renderer);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        const Uint8* pressed_keys = SDL_GetKeyboardState(nullptr);

        if (pressed_keys[SDL_GetScancodeFromKey(SDLK_UP)]) {
            if (!world.getMapAt(static_cast<int>(camera.xpos + camera.xdir * move_speed), static_cast<int>(camera.ypos))) {
                camera.xpos += camera.xdir * move_speed; 
            }
            if (!world.getMapAt(static_cast<int>(camera.xpos), static_cast<int>(camera.ypos + camera.ydir * move_speed))) {
                camera.ypos += camera.ydir * move_speed;
            }
        }
        else if (pressed_keys[SDL_GetScancodeFromKey(SDLK_DOWN)]) {
            if (!world.getMapAt(static_cast<int>(camera.xpos - camera.xdir * move_speed), static_cast<int>(camera.ypos))) {
                camera.xpos -= camera.xdir * move_speed; 
            }
            if (!world.getMapAt(static_cast<int>(camera.xpos), static_cast<int>(camera.ypos - camera.ydir * move_speed))) {
                camera.ypos -= camera.ydir * move_speed;
            }
        }
        else if (pressed_keys[SDL_GetScancodeFromKey(SDLK_LEFT)]) {
            double prev_xdir = camera.xdir;
		    camera.xdir = camera.xdir*cos(rot_speed) - camera.ydir*sin(rot_speed);
		    camera.ydir = prev_xdir*sin(rot_speed) + camera.ydir*cos(rot_speed);
		    double prev_xplane = camera.xplane;
		    camera.xplane = camera.xplane*cos(rot_speed) - camera.yplane*sin(rot_speed);
		    camera.yplane = prev_xplane*sin(rot_speed) + camera.yplane*cos(rot_speed);
        } 
        else if (pressed_keys[SDL_GetScancodeFromKey(SDLK_RIGHT)]) {
            double prev_xdir = camera.xdir;
		    camera.xdir = camera.xdir*cos(-rot_speed) - camera.ydir*sin(-rot_speed);
		    camera.ydir = prev_xdir*sin(-rot_speed) + camera.ydir*cos(-rot_speed);
		    double prev_xplane = camera.xplane;
		    camera.xplane = camera.xplane*cos(-rot_speed) - camera.yplane*sin(-rot_speed);
		    camera.yplane = prev_xplane*sin(-rot_speed) + camera.yplane*cos(-rot_speed);
        }
    }

	return 0;
}
