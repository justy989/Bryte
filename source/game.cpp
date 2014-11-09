#include "game.hpp"

#include <stdexcept>

using namespace bryte;

game::game ( int argc, char** argv ) :
     m_sdl_window ( "bryte 0.01a", 1024, 768 ),
     m_state ( state::title )
{

}

int game::run ( )
{
     SDL_Event sdl_event;
     bool quit = false;

     // loop forever
     while ( !quit ) {

          // grab events of the sdl event queue
          while ( SDL_PollEvent ( &sdl_event ) ) {
               if ( sdl_event.type == SDL_QUIT ) {
                    quit = true;
                    break;
               }
          }

          // update and draw the frame
          update ( );
          draw ( );

          // limit the fps
          SDL_Delay ( k_fps_delay );
     }

     return 0;
}

void game::update ( )
{
     switch ( m_state ) {
     case state::title:
          break;
     case state::editor:
          break;
     case state::world:
          break;
     default:
          throw std::out_of_range ( "Tried to execute unknown game state." );
     }
}

void game::draw ( )
{
     switch ( m_state ) {
     case state::title:
          break;
     case state::editor:
          break;
     case state::world:
          break;
     default:
          throw std::out_of_range ( "Tried to execute unknown game state." );
     }

     m_sdl_window.render ( );
}