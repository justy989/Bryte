#include "editor_state.hpp"

#include "draw_utils.hpp"

using namespace bryte;

editor_state::editor_state ( surface_man& sman,
                             int window_width,
                             int window_height ) :
     m_window_width ( window_width ),
     m_window_height ( window_height ),
     m_room ( 20, 20 ),
     m_camera ( rectangle ( 0, 0,
                            sdl_window::k_back_buffer_width,
                            sdl_window::k_back_buffer_height ),
                rectangle ( 0, 0,
                            m_room.width ( ) * room::k_tile_width,
                            m_room.height ( ) * room::k_tile_height + 60 ) ),
     m_tilesheet ( sman.load ( "castle_tilesheet.bmp" ) ),
     m_mode ( mode::tile ),
     m_tile_index_to_place ( 0 ),
     m_tile_sprite_to_place ( nullptr, vector ( ),
                              rectangle ( 0, 0, 0, 0 ) ),
     m_max_tile_index ( ( m_tilesheet->w / room::k_tile_width ) - 1 ),
     m_ui_buttons_surface ( sman.load ( "editor_button_icons.bmp" ) ),
     m_tile_index_inc_btn ( m_ui_buttons_surface, vector ( 240, 223 ), rectangle ( 77, 0, 86, 10 ) ),
     m_tile_index_dec_btn ( m_ui_buttons_surface, vector ( 5, 223 ), rectangle ( 66, 0, 76, 10 ) ),
     m_map_area ( 0, k_top_border, sdl_window::k_back_buffer_width, k_bottom_border )
{
     m_room_display.change_tilesheet ( m_tilesheet );

     m_tile_sprite_to_place = clipped_sprite ( m_tilesheet, vector ( ),
                                               rectangle ( 0, 0, 0, 0 ) );

     update_tile_sprite_clip ( );
}

void editor_state::update ( )
{
     int mousex, mousey;

     auto mouse_state = SDL_GetMouseState ( &mousex, &mousey );

     // calculate the coordinate ratio on the window: 320 / 640 would be 0.5
     float window_x_pct = static_cast< float >( mousex ) / static_cast< float >( m_window_width );
     float window_y_pct = static_cast< float >( mousey ) / static_cast< float >( m_window_height );

     // convert it to the back buffer quantity: 0.5 * 256 = 128
     float screen_x = window_x_pct * static_cast< float >( sdl_window::k_back_buffer_width );
     float screen_y = window_y_pct * static_cast< float >( sdl_window::k_back_buffer_height );

     // find the world position by casting down to our vector type
     m_mouse.set ( static_cast< vector_base_type >( screen_x ),
                   static_cast< vector_base_type >( screen_y ) );

     m_tile_sprite_to_place.position ( ).set ( m_mouse );

     m_tile_index_inc_btn.update ( m_mouse, mouse_state & SDL_BUTTON ( SDL_BUTTON_LEFT ) );
     m_tile_index_dec_btn.update ( m_mouse, mouse_state & SDL_BUTTON ( SDL_BUTTON_LEFT ) );

     if ( m_tile_index_inc_btn.get_state ( ) == ui_button::state::pressed ) {
          increment_tile_index ( );
     }

     if ( m_tile_index_dec_btn.get_state ( ) == ui_button::state::pressed ) {
          decrement_tile_index ( );
     }

     update_tile_sprite_clip ( );
}

void editor_state::draw ( SDL_Surface* back_buffer )
{
     m_room_display.display ( m_room, m_camera, vector ( 0, k_top_border ), back_buffer );
     m_tile_sprite_to_place.blit_onto ( back_buffer );

     SDL_Rect rect { 0, 0, sdl_window::k_back_buffer_width, k_top_border };
     SDL_FillRect ( back_buffer, &rect, 0x000000 );

     rect = SDL_Rect { 0, k_bottom_border, 
                       sdl_window::k_back_buffer_width,
                       sdl_window::k_back_buffer_height - k_bottom_border };
     SDL_FillRect ( back_buffer, &rect, 0x000000 );

     // in tile mode, draw the tilesheet
     if ( m_mode == mode::tile ) {
          draw_tile_strip ( back_buffer );
     }

     m_tile_index_inc_btn.draw ( back_buffer );
     m_tile_index_dec_btn.draw ( back_buffer );
}

void editor_state::handle_sdl_event ( const SDL_Event& sdl_event )
{
     handle_scroll ( sdl_event );
     handle_click ( sdl_event );
     handle_change_selected ( sdl_event );
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

void editor_state::handle_change_selected ( const SDL_Event& sdl_event )
{
     if ( sdl_event.type == SDL_KEYDOWN ) {
          if ( sdl_event.key.keysym.sym == SDLK_q ) {
               decrement_tile_index ( );
          } else if ( sdl_event.key.keysym.sym == SDLK_e ) {
               increment_tile_index ( );
          }
     }
}

void editor_state::change_tile_at_screen_position ( int x, int y )
{
     if ( !m_map_area.contains ( m_mouse ) ) {
          return;
     }

     // find the world position
     vector world_pos ( m_mouse );

     // offset by the camera
     world_pos += m_camera.viewport ( ).bottom_left ( );
     world_pos -= vector ( 0, k_top_border );

     // find the tile index we need to set
     vector tile_index ( world_pos.x ( ) / room::k_tile_width,
                         world_pos.y ( ) / room::k_tile_width );

     m_room.set_tile ( tile_index, m_tile_index_to_place );
}

void editor_state::update_tile_sprite_clip ( )
{
     auto& clip = m_tile_sprite_to_place.clip ( );

     clip.set ( m_tile_index_to_place * room::k_tile_width, 0,
                m_tile_index_to_place * room::k_tile_width + room::k_tile_width,
                room::k_tile_height );
}

void editor_state::draw_tile_strip ( SDL_Surface* back_buffer )
{
     const ubyte tiles_to_show = 5;
     vector_base_type start = static_cast< vector_base_type >( m_tile_index_to_place ) - tiles_to_show;
     vector_base_type stop = static_cast< vector_base_type >( m_tile_index_to_place ) + tiles_to_show;

     vector start_pos ( ( sdl_window::k_back_buffer_width / 2 ) - ( room::k_tile_width / 2 ) -
                        ( tiles_to_show * room::k_tile_width ),
                        219 );

     vector_base_type max_tile = ( m_tilesheet->w / room::k_tile_width ) - 1;

     for ( vector_base_type i = start; i <= stop; ++i ) {

          if ( i < 0 ) {
               start_pos.move_x ( room::k_tile_width );
               continue;
          } else if ( i >= max_tile ) {
               break;
          }

          SDL_Rect src { i * room::k_tile_width, 0,
                         room::k_tile_width,
                         room::k_tile_height };
          SDL_Rect dst { start_pos.x ( ), start_pos.y ( ) };

          SDL_BlitSurface ( m_tilesheet, &src, back_buffer, &dst );

          start_pos.move_x ( room::k_tile_width );
     }

     rectangle outline ( ( sdl_window::k_back_buffer_width / 2 ) - ( room::k_tile_width / 2 ),
                         219,
                         ( sdl_window::k_back_buffer_width / 2 ) + ( room::k_tile_width / 2 ),
                         219 + room::k_tile_height );

     draw_utils::draw_border ( outline, 0xFFFFFF, back_buffer );
}

void editor_state::increment_tile_index ( )
{
     if ( m_tile_index_to_place < ( m_max_tile_index - 1) ) {
          m_tile_index_to_place++;
     }
}

void editor_state::decrement_tile_index ( )
{
     if ( m_tile_index_to_place > 0 ) {
          m_tile_index_to_place--;
     }
}