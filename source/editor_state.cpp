#include "editor_state.hpp"

using namespace bryte;

editor_state::editor_state ( surface_man& sman,
                             int window_width,
                             int window_height ) :
     m_room ( 20, 20 ),
     m_camera ( rectangle ( 0, 0,
                            sdl_window::k_back_buffer_width,
                            sdl_window::k_back_buffer_height),
                rectangle ( 0, 0,
                            m_room.width ( ) * room::k_tile_width, 
			             m_room.height ( ) * room::k_tile_height ) ),
     m_mode ( mode::tile ),
     m_tile_to_place ( 1 ),
     m_window_width ( window_width ),
     m_window_height ( window_height )
{
     auto* surface = sman.load ( "castle_tilesheet.bmp" );

     m_room_display.change_tilesheet ( surface );
}

void editor_state::update ( )
{

}

void editor_state::draw ( SDL_Surface* back_buffer )
{
     m_room_display.display ( m_room, m_camera, back_buffer );
}

void editor_state::handle_sdl_event ( const SDL_Event& sdl_event )
{
     handle_scroll ( sdl_event );
     handle_click ( sdl_event );
}

void editor_state::handle_scroll ( const SDL_Event& sdl_event )
{
     if ( sdl_event.type == SDL_KEYDOWN ) {
          if ( sdl_event.key.keysym.sym == SDLK_a ) {
               m_camera.move ( vector ( -k_scroll_speed, 0 ) );
          } else if ( sdl_event.key.keysym.sym == SDLK_d ) {
               m_camera.move ( vector ( k_scroll_speed, 0 ) );
          } else if ( sdl_event.key.keysym.sym == SDLK_w ) {
               m_camera.move ( vector ( 0, -k_scroll_speed ) );
          } else if ( sdl_event.key.keysym.sym == SDLK_s ) {
               m_camera.move ( vector ( 0, k_scroll_speed ) );
          }
     }
}

void editor_state::handle_click ( const SDL_Event& sdl_event )
{
     if ( sdl_event.type == SDL_MOUSEBUTTONDOWN ) {
          if ( sdl_event.button.button == SDL_BUTTON_LEFT ) {
               change_tile_at_screen_position ( sdl_event.button.x, sdl_event.button.y );
          }
     }
}

void editor_state::change_tile_at_screen_position ( int x, int y )
{
     // calculate the coordinate ratio on the window: 320 / 640 would be 0.5
     float window_x_pct = static_cast< float >( x ) / static_cast< float >( m_window_width );
     float window_y_pct = static_cast< float >( y ) / static_cast< float >( m_window_height );

     // convert it to the back buffer quantity: 0.5 * 256 = 128
     float screen_x = window_x_pct * static_cast< float >( sdl_window::k_back_buffer_width );
     float screen_y = window_y_pct * static_cast< float >( sdl_window::k_back_buffer_height );

     // find the world position by casting down to our vector type
     vector world_pos ( static_cast< vector_base_type >( screen_x ),
                        static_cast< vector_base_type >( screen_y ) );

     // offset by the camera
     //world_pos -= m_camera.viewport ( ).bottom_right ( );

     // find the tile index we need to set
     vector tile_index ( world_pos.x ( ) / room::k_tile_width,
                         world_pos.y ( ) / room::k_tile_width );

     m_room.set_tile ( tile_index, m_tile_to_place );
}
