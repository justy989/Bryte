/* editor_state: allows editting rooms and their various layers. It is it's own
 *               game state separate from the game itself and utilizes a lot of
 *               gui.
 */

#ifndef BRYTE_EDITOR_STATE_HPP
#define BRYTE_EDITOR_STATE_HPP

#include "surface_man.hpp"
#include "room_display.hpp"
#include "sdl_window.hpp"
#include "ui_button.hpp"

namespace bryte
{
     class editor_state {
     public:

          editor_state ( surface_man& sman, int window_width, int window_height );

          void update ( );
          void draw ( SDL_Surface* back_buffer );
          void handle_sdl_event ( const SDL_Event& sdl_event );

     private:

          enum class mode {
               tile,
               solid,
               decor,
               interactives,
               enemies
          };

     private:

          static const int k_scroll_speed = 3;
          static const vector_base_type k_top_border = 20;
          static const vector_base_type k_bottom_border = 200;

     private:

          void handle_scroll ( const SDL_Event& sdl_event );
          void handle_click ( const SDL_Event& sdl_event );
          void handle_change_selected ( const SDL_Event& sdl_event );

          void change_tile_at_screen_position ( int x, int y );

          void update_tile_sprite_clip ( );

          void draw_tile_strip ( SDL_Surface* back_buffer );

          void increment_tile_index ( );
          void decrement_tile_index ( );

     private:

          int m_window_width;
          int m_window_height;

          vector m_mouse;

          room m_room;

          camera m_camera;

          room_display m_room_display;

          SDL_Surface* m_tilesheet;

          mode m_mode;

          ubyte m_tile_index_to_place;
          clipped_sprite m_tile_sprite_to_place;

          ubyte m_max_tile_index;

          SDL_Surface* m_ui_buttons_surface;
          ui_button m_tile_index_inc_btn;
          ui_button m_tile_index_dec_btn;

          rectangle m_map_area;
     };
}

#endif