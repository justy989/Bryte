#ifndef BRYTE_MAP_DISPLAY_HPP
#define BRYTE_MAP_DISPLAY_HPP

#include "Map.hpp"
#include "Animation.hpp"

#include <SDL2/SDL.h>

class GameMemory;

namespace bryte
{
     struct MapDisplay {
     public:

          Void clear ( );

          Bool load_surfaces ( GameMemory& game_memory, const Char8* tilesheet_filepath,
                               const Char8* decorsheet_filepath, const Char8* lampsheet_filepath );
          Void unload_surfaces ( );

          Void render ( SDL_Surface* back_buffer, Map& map, Real32 camera_x, Real32 camera_y,
                        Bool invisibles );

          Void tick ( );

     public:

          static const Int32 c_lamp_frame_delay = 8;
          static const Int32 c_lamp_frame_count = 3;

     public:

          SDL_Surface* tilesheet;
          SDL_Surface* decorsheet;
          SDL_Surface* lampsheet;

          Animation lamp_animation;
     };

     extern "C" Void render_light ( SDL_Surface* back_buffer, Map& map, Real32 camera_x, Real32 camera_y );
}

#endif

