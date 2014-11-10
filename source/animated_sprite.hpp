/* animated_sprite: Holds info for drawing a clipped surface onto another that 
 *                  updates over time to draw different frames.
 */

#ifndef BRYTE_ANIMATED_SPRITE_HPP
#define BRYTE_ANIMATED_SPRITE_HPP

#include <SDL2/SDL.h>

#include "sprite.hpp"

namespace bryte
{
     class animated_sprite : public sprite {
     public:

          animated_sprite ( SDL_Surface* surface, vector position,
                            vector dimensions, vector frame,
                            vector_base_type row_frame_count,
                            vector_base_type updates_per_frame );

          void update ( );

          void blit_onto ( SDL_Surface* destination ) final;

          inline void set_row_frame_count ( vector_base_type frame_row_count );

          inline const vector& dimensions ( ) const;

          inline void set_row ( vector_base_type frame_row );
          inline void set_frame ( vector_base_type frame );

     private:

          vector m_dimensions;
          vector m_frame;

          vector_base_type m_row_frame_count;
          vector_base_type m_updates_per_frame;
          vector_base_type m_updates_since_frame;
     };

     inline void animated_sprite::set_row_frame_count ( vector_base_type frame_row_count )
     {
          m_row_frame_count = frame_row_count;
     }

     inline const vector& animated_sprite::dimensions ( ) const
     {
          return m_dimensions;
     }

     inline void animated_sprite::set_row ( vector_base_type frame_row )
     {
          m_frame.set_y ( frame_row );
     }

     inline void animated_sprite::set_frame ( vector_base_type frame )
     {
          m_frame.set_x ( frame );
     }
}

#endif
