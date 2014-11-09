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

          enum class state {
               title,
               editor,
               world
          };

     private:

          sdl m_sdl;
          sdl_window m_sdl_window;

          state m_state;
     };
}

#endif