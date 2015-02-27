#ifndef BRYTE_PROJECTILE_DISPLAY_HPP
#define BRYTE_PROJECTILE_DISPLAY_HPP

#include "Projectile.hpp"
#include "Animation.hpp"

#include <SDL2/SDL.h>

class GameMemory;

namespace bryte
{
     class Map;

     struct ProjectileDisplay {
     public:

          Bool load_surfaces ( GameMemory& game_memory );
          Void unload_surfaces ( );

          Void tick ( );

          Void render ( SDL_Surface* back_buffer, const Projectile& projectile,
                        Real32 camera_x, Real32 camera_y );

     public:

          static const Int32 c_frame_delay = 5;

          static const Int32 c_frame_count = 3;

     public:

          SDL_Surface* arrow_sheet;
          SDL_Surface* goo_sheet;

          Animation animation;
     };
}

#endif

