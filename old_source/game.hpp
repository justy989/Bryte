/* game: Sets up application and handles game states.
 */

#ifndef BRYTE_GAME_HPP
#define BRYTE_GAME_CPP

#include "configuration.hpp"

#include "sdl.hpp"
#include "sdl_window.hpp"

#include "surface_man.hpp"

#include "text_raii.hpp"

#include "title_state.hpp"
#include "editor_state.hpp"
#include "world_state.hpp"

namespace bryte
{
     class game {
     public:

          game ( int argc, char** argv );

          int run ( );

     private:

          void update ( );
          void draw ( );
          void handle_sdl_event ( const SDL_Event& sdl_event );

     private:

          static const int k_fps_delay = 1000 / 30;

     private:

          bool m_quit;

          configuration m_configuration;

          sdl m_sdl;
          sdl_window m_sdl_window;

          surface_man m_surface_man;

          text_raii m_text_raii;

          game_state m_state;

          title_state m_title_state;
          editor_state m_editor_state;
          world_state m_world_state;
     };
}

#endif
