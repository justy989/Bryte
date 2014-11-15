#ifndef BRYTE_EDITOR_STATE_HPP
#define BRYTE_EDITOR_STATE_HPP

#include "surface_man.hpp"
#include "room_display.hpp"
#include "sdl_window.hpp"

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

     private:

          void handle_scroll ( const SDL_Event& sdl_event );
          void handle_click ( const SDL_Event& sdl_event );

          void change_tile_at_screen_position ( int x, int y );

     private:

          room m_room;

          camera m_camera;

          room_display m_room_display;

          mode m_mode;

          ubyte m_tile_to_place;

          int m_window_width;
          int m_window_height;
     };
}

#endif