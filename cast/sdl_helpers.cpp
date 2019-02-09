#include "sdl_helpers.h"

void SDLDeleter::operator()(SDL_Texture* texture)
{
    SDL_DestroyTexture(texture);
}

void SDLDeleter::operator()(SDL_Renderer* renderer)
{
    SDL_DestroyRenderer(renderer);
}

void SDLDeleter::operator()(SDL_Window* window)
{
    SDL_DestroyWindow(window);
}
