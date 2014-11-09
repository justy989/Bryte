#ifndef BRYTE_CLIPPED_SPRITE_HPP
#define BRYTE_CLIPPED_SPRITE_HPP

#include "sprite.hpp"

namespace bryte
{
     class clipped_sprite : public sprite {
     public:

          clipped_sprite ( SDL_Surface* surface, vector position,
                           const rectangle& clip );

          void blit_onto ( SDL_Surface* destination ) final;

          inline rectangle& clip ( );

     private:

          rectangle m_clip;
     };

     inline rectangle& clipped_sprite::clip ( )
     {
          return m_clip;
     }
}

#endif