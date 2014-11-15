#include "clipped_sprite.hpp"

using namespace bryte;

clipped_sprite::clipped_sprite ( SDL_Surface* surface, vector position,
                                 const rectangle& clip ) :
    sprite ( surface, position ),
    m_clip ( clip )
{

}

void clipped_sprite::blit_onto ( SDL_Surface* destination )
{
     SDL_Rect src_rect { m_clip.left ( ), m_clip.bottom ( ),
                         m_clip.width ( ), m_clip.height ( ) };
     SDL_Rect dst_rect { m_position.x ( ), m_position.y ( ), 0, 0 };

     SDL_BlitSurface ( m_surface, &src_rect, destination, &dst_rect );
}
