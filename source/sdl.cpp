#include "sdl.hpp"

#include <SDL2/SDL.h>

using namespace bryte;

sdl::sdl ( )
{
     SDL_Init ( SDL_INIT_VIDEO | SDL_INIT_EVENTS );
}

sdl::~sdl ( )
{
     SDL_Quit ( );
}
