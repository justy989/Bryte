#ifndef BRYTE_MAP_DISPLAY_HPP
#define BRYTE_MAP_DISPLAY_HPP

#include "Map.hpp"

#include <SDL2/SDL.h>

namespace bryte
{
     extern "C" Void render_map ( SDL_Surface* back_buffer, SDL_Surface* tilesheet, Map& map,
                                  Real32 camera_x, Real32 camera_y );

     extern "C" Void render_map_decor ( SDL_Surface* back_buffer, SDL_Surface* decorsheet, Map& map,
                                        Real32 camera_x, Real32 camera_y );

     extern "C" Void render_map_lamps ( SDL_Surface* back_buffer, SDL_Surface* lampsheet, Map& map,
                                        Real32 camera_x, Real32 camera_y );

     extern "C" Void render_map_exits ( SDL_Surface* back_buffer, Map& map,
                                        Real32 camera_x, Real32 camera_y );

     extern "C" Void render_light ( SDL_Surface* back_buffer, Map& map, Real32 camera_x, Real32 camera_y );
}

#endif

