#include "room_display.hpp"
#include "draw_utils.hpp"

#include <stdexcept>

using namespace bryte;

const vector_base_type room_display::k_orientation_clip_y[ rotation::count ] = {
     0,
     room::k_tile_height,
     room::k_tile_height * 2,
     room::k_tile_height * 3,
};

room_display::room_display ( ) :
     m_tilesheet ( nullptr, vector ( ),
                   rectangle ( 0, 0,
                               room::k_tile_width, room::k_tile_height ) )
{

}

room_display::~room_display ( )
{
     if ( m_orientation_tilesheet_surface ) {
          SDL_FreeSurface ( m_orientation_tilesheet_surface );
          m_orientation_tilesheet_surface = nullptr;
     }
}

void room_display::change_tilesheet ( SDL_Surface* surface )
{
     gen_orientation_tilesheet ( surface );
     build_orientations ( surface );

     m_tilesheet = clipped_sprite ( m_orientation_tilesheet_surface,
                                    vector ( ),
                                    rectangle ( 0, 0,
                                                room::k_tile_width,
                                                room::k_tile_height ) );
}

void room_display::display ( const room& room, const camera& camera,
                             vector view_offset, SDL_Surface* back_buffer )
{
     display_tiles ( room, camera, view_offset, back_buffer );
}

void room_display::display_tiles ( const room& room, const camera& camera,
                                   vector view_offset, SDL_Surface* back_buffer )
{
     // loop over the entire room, optimize later!
     for ( vector_base_type x = 0; x < room.width ( ); ++x ) {
          for ( vector_base_type y = 0; y < room.height ( ); ++y ) {

               auto& tile = room.get_tile ( vector ( x, y ) );
               auto id = tile.id;
               auto orientation = tile.orientation;

               // build the clip using the tile id
               auto& tile_clip = m_tilesheet.clip ( );
               tile_clip.set ( id * room::k_tile_width,
                               k_orientation_clip_y [ orientation ],
                               id * room::k_tile_width + room::k_tile_width,
                               k_orientation_clip_y [ orientation ] + room::k_tile_height );

               // set the position rect and move it by the camera
               m_tilesheet.position ( ).set ( x * room::k_tile_width,
                                              y * room::k_tile_height );

               m_tilesheet.position ( ) -= camera.viewport ( ).bottom_left ( );
               m_tilesheet.position ( ) += view_offset;

               // draw onto the back buffer
               m_tilesheet.blit_onto ( back_buffer );
          }
     }
}

void room_display::gen_orientation_tilesheet ( SDL_Surface* tilesheet )
{
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
     Uint32 r_mask = 0xff000000;
     Uint32 g_mask = 0x00ff0000;
     Uint32 b_mask = 0x0000ff00;
     Uint32 a_mask = 0x000000ff;
#else
     Uint32 r_mask = 0x000000ff;
     Uint32 g_mask = 0x0000ff00;
     Uint32 b_mask = 0x00ff0000;
     Uint32 a_mask = 0xff000000;
#endif

     m_orientation_tilesheet_surface = SDL_CreateRGBSurface ( 0,
                                                              tilesheet->w,
                                                              tilesheet->h * rotation::count,
                                                              32,
                                                              r_mask, g_mask,
                                                              b_mask, a_mask );

     if ( !m_orientation_tilesheet_surface ) {
          throw std::runtime_error ( std::string ( "SDL_CreateRGBSurface(): " ) +
                                     SDL_GetError ( ) );
     }
}

void room_display::build_orientations ( SDL_Surface* tilesheet )
{
     // duplicate for each of the orientations
     SDL_Rect src { 0, 0, tilesheet->w, tilesheet->h };
     SDL_Rect dst { 0, 0, 0, 0 };

     for ( ubyte i = 0; i < rotation::count; ++i ) {
          SDL_BlitSurface ( tilesheet, &src, m_orientation_tilesheet_surface, &dst );
          dst.y += tilesheet->h;
     }

     vector_base_type tile_count = tilesheet->w / room::k_tile_width;

     for ( vector_base_type i = 0; i < tile_count; ++i ) {
          rectangle tile ( i * room::k_tile_width,
                           room::k_tile_height,
                           i * room::k_tile_width + room::k_tile_width,
                           room::k_tile_height + room::k_tile_height );

          draw_utils::rotate_square_clockwise ( m_orientation_tilesheet_surface, tile );

          tile += vector ( 0, room::k_tile_height );

          draw_utils::rotate_square_clockwise ( m_orientation_tilesheet_surface, tile );
          draw_utils::rotate_square_clockwise ( m_orientation_tilesheet_surface, tile );

          tile += vector ( 0, room::k_tile_height );

          draw_utils::rotate_square_clockwise ( m_orientation_tilesheet_surface, tile );
          draw_utils::rotate_square_clockwise ( m_orientation_tilesheet_surface, tile );
          draw_utils::rotate_square_clockwise ( m_orientation_tilesheet_surface, tile );
     }
}

