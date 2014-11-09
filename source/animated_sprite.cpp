#include "animated_sprite.hpp"

using namespace bryte;

animated_sprite::animated_sprite ( SDL_Surface* surface, vector position,
                                   vector dimensions, vector frame,
                                   vector_base_type row_frame_count,
                                   vector_base_type updates_per_frame ) :
     sprite ( surface, position ),
     m_dimensions ( dimensions ),
     m_frame ( frame ),
     m_row_frame_count ( row_frame_count ),
     m_updates_per_frame ( updates_per_frame ),
     m_updates_since_frame ( 0 )
{

}

void animated_sprite::update ( )
{
     m_updates_since_frame++;

     if ( m_updates_since_frame >= m_updates_per_frame ) {
          m_updates_since_frame = 0;
          m_frame.set_x ( ( m_frame.x ( ) + 1 ) % m_row_frame_count );
     }
}

void animated_sprite::blit_onto ( SDL_Surface* destination )
{
     SDL_Rect src_rect { m_frame.x ( ) * m_dimensions.x (),
                         m_frame.y ( ) * m_dimensions.y ( ),
                         m_dimensions.x ( ), m_dimensions.y ( ) };
     SDL_Rect dst_rect { m_position.x ( ), m_position.y ( ), 0, 0 };

     SDL_BlitSurface ( m_surface, &src_rect, destination, &dst_rect );
}
