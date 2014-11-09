#include "game.hpp"

using namespace bryte;

game::game ( int argc, char** argv ) :
m_sdl_window ( "bryte 0.01a", 1024, 768 )
{

}

int game::run ( )
{
     SDL_Event sdl_event;
     bool quit = false;

     while ( !quit ) {
          while ( SDL_PollEvent ( &sdl_event ) ) {
               if ( sdl_event.type == SDL_QUIT ) {
                    quit = true;
                    break;
               }
          }

          m_sdl_window.render ( );
          SDL_Delay ( 1000 / 30 );
     }

     return 0;
}
