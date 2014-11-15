#include "room_display.hpp"

using namespace bryte;

room_display::room_display ( ) :
     m_tilesheet ( nullptr, vector ( ),
                   rectangle ( 0, room::k_tile_height,
                               room::k_tile_width, 0 ) )
{

}

void room_display::change_tilesheet ( SDL_Surface* surface )
{
     m_tilesheet = clipped_sprite ( surface,
                                    vector ( ),
                                    rectangle ( 0, room::k_tile_height,
                                                room::k_tile_width, 0 ) );
}

void room_display::display ( const room& room, const camera& camera,
                             SDL_Surface* back_buffer )
{
     display_tiles ( room, camera, back_buffer );
}

void room_display::display_tiles ( const room& room, const camera& camera,
                                   SDL_Surface* back_buffer )
{
     // loop over the entire room, optimize later!
     for ( vector_base_type x = 0; x < room.width ( ); ++x ) {
          for ( vector_base_type y = 0; y < room.height ( ); ++y ) {

               auto tile_id = room.get_tile ( vector ( x, y ) );
               
               // build the clip using the tile id
               auto& tile_clip = m_tilesheet.clip ( );
               tile_clip.set_left ( tile_id * room::k_tile_width );
               tile_clip.set_right ( tile_clip.left ( ) + room::k_tile_width );
               tile_clip.set_bottom ( 0 );
               tile_clip.set_top ( room::k_tile_height );

               // set the position rect and move it by the camera
               m_tilesheet.position ( ).set ( x * room::k_tile_width,
                                              y * room::k_tile_height );

               m_tilesheet.position ( ) -= camera.viewport ( ).bottom_left ( );

               // draw onto the back buffer
               m_tilesheet.blit_onto ( back_buffer );
          }
     }
}