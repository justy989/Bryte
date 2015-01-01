/* draw_utils: utilities for common drawing functionality
 */

#ifndef BRYTE_DRAW_UTILS_HPP
#define BRYTE_DRAW_UTILS_HPP

#include "types.hpp"

#include <SDL2/SDL.h>

namespace bryte
{
     class draw_utils {
     public:

          static void draw_border ( const rectangle& outline, Uint32 color, SDL_Surface* back_buffer );

          static void rotate_square_clockwise ( SDL_Surface* surface, const rectangle& square );

     private:

          // not constructable
          draw_utils ( );
     };
}

#endif
