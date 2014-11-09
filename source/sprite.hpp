#ifndef BRYTE_SPRITE_HPP
#define BRYTE_SPRITE_HPP

#include <SDL2\SDL.h>

#include "types.hpp"

namespace bryte
{
     class sprite {
     public:

          sprite ( SDL_Surface* surface, vector position );

          virtual void blit_onto ( SDL_Surface* destination );

          inline vector& position ( );

     protected:

          SDL_Surface* m_surface;

          vector m_position;
     };

     inline vector& sprite::position ( )
     {
          return m_position;
     }
}

#endif
