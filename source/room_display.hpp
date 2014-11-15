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
                         SDL_Surface* back_buffer );

     private:

          void display_tiles ( const room& room, const camera& camera,
                               SDL_Surface* back_buffer );

     private:
          
          clipped_sprite m_tilesheet;
     };
}

#endif