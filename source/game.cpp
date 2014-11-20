#include "game.hpp"

#include <stdexcept>

using namespace bryte;

game::game ( int argc, char** argv ) :
     m_quit ( false ),
     m_configuration ( std::string ( "bryte.cfg" ) ),
     m_sdl_window ( "bryte 0.01a",
                    m_configuration.window_width ( ),
                    m_configuration.window_height ( ) ),
     m_state ( game_state::title ),
     m_title_state ( m_surface_man,
                     m_sdl_window.width ( ),
                     m_sdl_window.height ( ) ),
     m_editor_state ( m_surface_man,
                      m_sdl_window.width ( ),
                      m_sdl_window.height ( ) )
{

}

int game::run ( )
{
     SDL_Event sdl_event;

     // loop forever
     while ( !m_quit ) {

          // grab events of the sdl event queue
          while ( SDL_PollEvent ( &sdl_event ) ) {
               if ( sdl_event.type == SDL_QUIT ) {
                    m_quit = true;
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
     case game_state::title:
          m_state = m_title_state.update ( );
          break;
     case game_state::editor:
          m_state = m_editor_state.update ( );
          break;
     case game_state::world:
          break;
     case game_state::quit:
          break;
      default:
          throw std::out_of_range ( "Tried to execute unknown game state." );
     }

     // quit if the game state changes to the quit state
     if ( m_state == game_state::quit ) {
          m_quit = true;
     }
}

void game::draw ( )
{
     switch ( m_state ) {
     case game_state::title:
          m_title_state.draw ( m_sdl_window.back_buffer ( ) );
          break;
     case game_state::editor:
          m_editor_state.draw ( m_sdl_window.back_buffer ( ) );
          break;
     case game_state::world:
          break;
     case game_state::quit:
          break;
     default:
          throw std::out_of_range ( "Tried to execute unknown game state." );
     }

     m_sdl_window.render ( );
}

void game::handle_sdl_event ( const SDL_Event& sdl_event )
{
     switch ( m_state ) {
     case game_state::title:
          m_title_state.handle_sdl_event ( sdl_event );
          break;
     case game_state::editor:
          m_editor_state.handle_sdl_event ( sdl_event );
          break;
     case game_state::world:
          break;
     case game_state::quit:
          break;
     default:
          throw std::out_of_range ( "Tried to execute unknown game state." );
     }
}

