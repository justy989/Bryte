/* room_display: draw the different layers of the room
 */

#ifndef BRYTE_ROOM_DISPLAY_HPP
#define BRYTE_ROOM_DISPLAY_HPP

#include "room.hpp"
#include "camera.hpp"

#include "clipped_sprite.hpp"

namespace bryte
{
     class room_display {
     public:

          room_display ( );

          void change_tilesheet ( SDL_Surface* surface );

          void display ( const room& room, const camera& camera,
                         vector view_offset, SDL_Surface* back_buffer );

     private:

          void display_tiles ( const room& room, const camera& camera,
                               vector view_offset, SDL_Surface* back_buffer );

          void gen_orientation_tilesheet ( SDL_Surface* tilesheet );
          void build_orientations ( SDL_Surface* tilesheet );

     private:

          static const vector_base_type k_orientation_clip_y[ rotation::count ];

     private:

          clipped_sprite m_tilesheet;

          SDL_Surface* m_orientation_tilesheet_surface;
     };
}

#endif
