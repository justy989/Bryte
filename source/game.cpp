#include "game.hpp"

#include <stdexcept>

using namespace bryte;

game::game ( int argc, char** argv ) :
     m_configuration ( std::string ( "bryte.cfg" ) ),
     m_sdl_window ( "bryte 0.01a", 
                    m_configuration.window_width ( ),
                    m_configuration.window_height ( ) ),
     m_state ( state::editor ),
     m_editor_state ( m_surface_man,
                      m_sdl_window.width ( ),
                      m_sdl_window.height ( ) )
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

               handle_sdl_event ( sdl_event );
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
          m_editor_state.update ( );
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
          m_editor_state.draw ( m_sdl_window.back_buffer ( ) );
          break;
     case state::world:
          break;
     default:
          throw std::out_of_range ( "Tried to execute unknown game state." );
     }

     text::draw ( m_sdl_window.back_buffer ( ),  std::string( "BRYTE" ), vector ( 30, 30 ) );

     m_sdl_window.render ( );
}

void game::handle_sdl_event ( const SDL_Event& sdl_event )
{
     switch ( m_state ) {
     case state::title:
          break;
     case state::editor:
          m_editor_state.handle_sdl_event ( sdl_event );
          break;
     case state::world:
          break;
     default:
          throw std::out_of_range ( "Tried to execute unknown game state." );
     }
}
