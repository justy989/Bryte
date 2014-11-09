#include "sprite.hpp"

using namespace bryte;

sprite::sprite ( SDL_Surface* surface, vector position ) :
     m_surface ( surface ),
     m_position ( position )
{

}

void sprite::blit_onto ( SDL_Surface* destination )
{
     SDL_Rect src_rect { 0, 0, m_surface->w, m_surface->h };
     SDL_Rect dst_rect { m_position.x (), m_position.y (), 0, 0 };

     SDL_BlitSurface ( m_surface, &src_rect, destination, &dst_rect );
}
