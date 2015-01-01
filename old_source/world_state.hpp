/* world_state: This is where the game is played. It handles object interactions
 *              as well as room transitions and game logic.
 */

#ifndef BRYTE_WORLD_STATE_HPP
#define BRYTE_WORLD_STATE_HPP

#include "game_state.hpp"
#include "surface_man.hpp"
#include "room_display.hpp"
#include "camera.hpp"

namespace bryte
{
     class world_state {
     public:

          world_state ( surface_man& sman );

          game_state update ( );
          void draw ( SDL_Surface* back_buffer );
          void handle_sdl_event ( const SDL_Event& sdl_event );

     private:

          room m_room;
          room_display m_room_display;

          camera m_camera;
     };
}

#endif
