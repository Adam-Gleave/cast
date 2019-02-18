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

void queue_key_events(EventQueue& queue)
{
    const Uint8* keys = SDL_GetKeyboardState(nullptr);

    if (keys[SDL_GetScancodeFromKey(SDLK_UP)])
    {
        queue.push(Event::KEY_UP);
    }

    if (keys[SDL_GetScancodeFromKey(SDLK_DOWN)])
    {
        queue.push(Event::KEY_DOWN);
    }

    if (keys[SDL_GetScancodeFromKey(SDLK_LEFT)])
    {
        queue.push(Event::KEY_LEFT);
    }

    if (keys[SDL_GetScancodeFromKey(SDLK_RIGHT)])
    {
        queue.push(Event::KEY_RIGHT);
    }
}

int event_loop(World* world, Camera* camera, SDL_Event& event,
        SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* screen_texture) 
{
    EventQueue event_queue;
    event_queue.register_handler(camera);

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
        camera->update_speeds(move_speed, rot_speed);

        // Place pixel buffer into SDL_Texture and render
        SDL_UpdateTexture(screen_texture, NULL, &screen_pixels[0], WIDTH*4);
        SDL_RenderCopy(renderer, screen_texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        memset(&screen_pixels[0], 0, screen_pixels.size()*sizeof(screen_pixels[0]));

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Handle key presses
        queue_key_events(event_queue);
        event_queue.check_and_execute();
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
