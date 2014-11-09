/* game: Sets up application and handles game states.
 */

#ifndef BRYTE_GAME_HPP
#define BRYTE_GAME_CPP

#include "sdl.hpp"
#include "sdl_window.hpp"

namespace bryte
{
     class game {
     public:

          game ( int argc, char** argv );

          int run ( );

     private:

          void update ( );
          void draw ( );

     private:

          enum class state {
               title,
               editor,
               world
          };

          static const int k_fps_delay = 1000 / 30;

     private:

          sdl m_sdl;
          sdl_window m_sdl_window;

          state m_state;
     };
}

#endif