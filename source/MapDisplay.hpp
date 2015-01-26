#ifndef BRYTE_MAP_DISPLAY_HPP
#define BRYTE_MAP_DISPLAY_HPP

#include "Map.hpp"

#include <SDL2/SDL.h>

namespace bryte
{
     struct MapDisplay {

          Void render ( SDL_Surface* back_buffer, Map& map, Real32 camera_x, Real32 camera_y );

          SDL_Surface* tilesheet;
          SDL_Surface* decorsheet;
          SDL_Surface* lampsheet;
     };

     extern "C" Void render_light ( SDL_Surface* back_buffer, Map& map, Real32 camera_x, Real32 camera_y );
}

#endif

