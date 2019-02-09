#include "SDL.h"

struct SDLDeleter
{
    void operator()(SDL_Texture* texture);
    void operator()(SDL_Renderer* renderer);
    void operator()(SDL_Window* window);
};
