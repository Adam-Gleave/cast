#define SDL_MAIN_HANDLED

#include "cast.h"
#include "camera.h"
#include "sdl_helpers.h"
#include <memory>
#include <sstream>

using namespace std;

const int WIDTH = 800;
const int HEIGHT = 600;

bool to_quit(const SDL_Event& event) 
{
    bool def = false;

    switch (event.key.keysym.sym) {
        case SDLK_ESCAPE:
            return true;
    }

    return def;
}

void key_up(Camera* camera, World* world, double move_speed)
{
    if (!world->getMapAt(static_cast<int>(camera->xpos + camera->xdir * move_speed), static_cast<int>(camera->ypos))) 
    {
        camera->xpos += camera->xdir * move_speed; 
    }

    if (!world->getMapAt(static_cast<int>(camera->xpos), static_cast<int>(camera->ypos + camera->ydir * move_speed))) 
    {
        camera->ypos += camera->ydir * move_speed;
    }
}

void key_down(Camera* camera, World* world, double move_speed)
{
    if (!world->getMapAt(static_cast<int>(camera->xpos - camera->xdir * move_speed), static_cast<int>(camera->ypos))) 
    {
        camera->xpos -= camera->xdir * move_speed; 
    }

    if (!world->getMapAt(static_cast<int>(camera->xpos), static_cast<int>(camera->ypos - camera->ydir * move_speed))) 
    {
        camera->ypos -= camera->ydir * move_speed;
    }   
}

void key_left(Camera* camera, World* world, double rot_speed)
{
    double prev_xdir = camera->xdir;
    camera->xdir = camera->xdir*cos(rot_speed) - camera->ydir*sin(rot_speed);
    camera->ydir = prev_xdir*sin(rot_speed) + camera->ydir*cos(rot_speed);
    double prev_xplane = camera->xplane;
    camera->xplane = camera->xplane*cos(rot_speed) - camera->yplane*sin(rot_speed);
    camera->yplane = prev_xplane*sin(rot_speed) + camera->yplane*cos(rot_speed);  
}

void key_right(Camera* camera, World* world, double rot_speed)
{
    double prev_xdir = camera->xdir;
    camera->xdir = camera->xdir*cos(-rot_speed) - camera->ydir*sin(-rot_speed);
    camera->ydir = prev_xdir*sin(-rot_speed) + camera->ydir*cos(-rot_speed);
    double prev_xplane = camera->xplane;
    camera->xplane = camera->xplane*cos(-rot_speed) - camera->yplane*sin(-rot_speed);
    camera->yplane = prev_xplane*sin(-rot_speed) + camera->yplane*cos(-rot_speed);
}

void handle_keys(const Uint8* keys, Camera* camera, World* world, double move_speed, double rot_speed)
{
    if (keys[SDL_GetScancodeFromKey(SDLK_UP)]) 
    {
        key_up(camera, world, move_speed);
    }

    if (keys[SDL_GetScancodeFromKey(SDLK_DOWN)]) 
    {
        key_down(camera, world, move_speed);
    }

    if (keys[SDL_GetScancodeFromKey(SDLK_LEFT)]) 
    {
        key_left(camera, world, rot_speed);
    } 

    if (keys[SDL_GetScancodeFromKey(SDLK_RIGHT)]) 
    {
        key_right(camera, world, rot_speed);
    }
}

int event_loop(World* world, Camera* camera, SDL_Event& event,
        SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* screen_texture) 
{
    // Pixel buffer
    vector<unsigned char> screen_pixels(WIDTH * HEIGHT * 4, 0);

    // Persistent variables
    const Uint8* keys = nullptr;
    bool quit = false;
    double time = 0.0f;
    double prev_time = 0.0f;

    // Main event/render loop
    while (!quit) 
    {
        while (SDL_PollEvent(&event)) 
        {
            SDL_Delay(5);
            keys = SDL_GetKeyboardState(0);

            if (event.type == SDL_QUIT) 
            {
                quit = true;
            }
            else if (event.type == SDL_KEYDOWN) 
            {
                quit = to_quit(event);
            }
        }

        // Render camera view into pixel buffer
        screen_pixels = camera->render_buffer(WIDTH, HEIGHT);

        // Keep a consistent move and rotation speed
        time = SDL_GetTicks();
        double frame_time = (time - prev_time)/1000.0;
        prev_time = time;
        double move_speed = frame_time * 3.0f;
        double rot_speed = frame_time * 3.0f;

        // Place pixel buffer into SDL_Texture and render
        SDL_UpdateTexture(screen_texture, NULL, &screen_pixels[0], WIDTH*4);
        SDL_RenderCopy(renderer, screen_texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        memset(&screen_pixels[0], 0, screen_pixels.size()*sizeof(screen_pixels[0]));

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Handle key presses
        const Uint8* keys = SDL_GetKeyboardState(nullptr);
        handle_keys(keys, camera, world, move_speed, rot_speed);
    }

    return 0;
}

int main() 
{
    // Initialise world data
    auto world = make_shared<World>();
    auto camera = make_unique<Camera>(world, WIDTH, HEIGHT);

    // Initialise SDL
    SDL_Init(SDL_INIT_EVERYTHING);
    atexit(SDL_Quit);
    SDL_Event event;

    auto window = unique_ptr<SDL_Window, SDLDeleter>(SDL_CreateWindow("cast", SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN));
    auto renderer = unique_ptr<SDL_Renderer, SDLDeleter>(SDL_CreateRenderer(window.get(), -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));
    auto screen_texture = unique_ptr<SDL_Texture, SDLDeleter>(SDL_CreateTexture(renderer.get(), SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT));

    return event_loop(world.get(), camera.get(), event, window.get(), renderer.get(), screen_texture.get());
}
