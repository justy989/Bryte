#include "editor_state.hpp"

#include "draw_utils.hpp"

using namespace bryte;

editor_state::editor_state ( surface_man& sman,
                             int window_width,
                             int window_height ) :
     m_window_width ( window_width ),
     m_window_height ( window_height ),
     m_mouse ( window_width, window_height,
               sdl_window::k_back_buffer_width,
               sdl_window::k_back_buffer_height ),
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
     m_tile_orientation_to_place ( rotation::zero ),
     m_tile_sprite_to_place ( nullptr, vector ( ),
                              rectangle ( 0, 0, 0, 0 ) ),
     m_max_tile_index ( ( m_tilesheet->w / room::k_tile_width ) - 1 ),
     m_ui_buttons_surface ( sman.load ( "editor_button_icons.bmp" ) ),
     m_tile_index_inc_btn ( m_ui_buttons_surface, vector ( 223, 222 ), rectangle ( 77, 0, 86, 10 ) ),
     m_tile_index_dec_btn ( m_ui_buttons_surface, vector ( 25, 222 ), rectangle ( 66, 0, 76, 10 ) ),
     m_quit_btn ( m_ui_buttons_surface, vector ( 238, 2 ), rectangle ( 55, 0, 65, 10 ) ),
     m_map_area ( 0, k_top_border, sdl_window::k_back_buffer_width, k_bottom_border )
{
     m_room_display.change_tilesheet ( m_tilesheet );

     m_tile_sprite_to_place = clipped_sprite ( m_room_display.orientation_tilesheet_surface ( ),
                                               vector ( ), rectangle ( 0, 0, 0, 0 ) );

     update_tile_sprite_clip ( );
}

game_state editor_state::update ( )
{
     m_mouse.update ( );

     m_tile_sprite_to_place.position ( ).set ( m_mouse.position ( ) );

     m_tile_index_inc_btn.update ( m_mouse.position ( ), m_mouse.left_clicked ( ) );
     m_tile_index_dec_btn.update ( m_mouse.position ( ), m_mouse.left_clicked ( ) );
     m_quit_btn.update ( m_mouse.position ( ), m_mouse.left_clicked ( ) );

     if ( m_tile_index_inc_btn.pressed ( ) ) {
          increment_tile_index ( );
     }

     if ( m_tile_index_dec_btn.pressed ( ) ) {
          decrement_tile_index ( );
     }

     if ( m_quit_btn.pressed ( ) ) {
          return game_state::title;
     }

     update_tile_sprite_clip ( );

     return game_state::editor;
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
     m_quit_btn.draw ( back_buffer );
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
               update_tile_sprite_clip ( );
          }
          else if ( sdl_event.key.keysym.sym == SDLK_e ) {
               increment_tile_index ( );
               update_tile_sprite_clip ( );
          }
          else if ( sdl_event.key.keysym.sym == SDLK_r ) {
               m_tile_orientation_to_place++;
               m_tile_orientation_to_place %= rotation::count;
               update_tile_sprite_clip ( );
          }
     }
}

void editor_state::change_tile_at_screen_position ( int x, int y )
{
     if ( !m_map_area.contains ( m_mouse.position ( ) ) ) {
          return;
     }

     // find the world position
     vector mouse_world_pos ( m_mouse.position ( ) );

     // offset by the camera
     mouse_world_pos += m_camera.viewport ( ).bottom_left ( );
     mouse_world_pos -= vector ( 0, k_top_border );

     // find the tile index we need to set
     vector tile_location ( mouse_world_pos.x ( ) / room::k_tile_width,
                            mouse_world_pos.y ( ) / room::k_tile_width );

     auto& tile = m_room.get_tile ( tile_location );
     tile.id = m_tile_index_to_place;
     tile.orientation = static_cast<rotation>( m_tile_orientation_to_place );
}

void editor_state::update_tile_sprite_clip ( )
{
     auto& clip = m_tile_sprite_to_place.clip ( );

     vector_base_type height = m_tile_orientation_to_place * room::k_tile_height;

     clip.set ( m_tile_index_to_place * room::k_tile_width, height,
                m_tile_index_to_place * room::k_tile_width + room::k_tile_width,
                height + room::k_tile_height );
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
