#ifndef BRYTE_EDITOR_STATE_HPP
#define BRYTE_EDITOR_STATE_HPP

#include "surface_man.hpp"
#include "room_display.hpp"

#include <SDL2/SDL.h>

namespace bryte
{
     class editor_state {
     public:

          editor_state ( surface_man& sman );

          void update ( );
          void draw ( SDL_Surface* back_buffer );
          void handle_sdl_event ( const SDL_Event& sdl_event );

     private:

          room m_room;

          camera m_camera;

          room_display m_room_display;
     };
}

#endif