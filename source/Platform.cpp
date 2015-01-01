#include "Platform.hpp"

#include <cstdio>

Platform::Platform ( ) :
     m_window ( nullptr ),
     m_window_surface ( nullptr )
{
     SDL_Init ( SDL_INIT_VIDEO );
}

Platform::~Platform ( )
{
     if ( m_window ) {
          SDL_DestroyWindow ( m_window );

          m_window_surface = nullptr;
          m_window = nullptr;
     }

     SDL_Quit ( );
}

bool Platform::create_window ( const char* window_title, int window_width, int window_height )
{
     // create the window with the specified parameters
     m_window = SDL_CreateWindow ( window_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   window_width, window_height, 0 );

     if ( !m_window ) {
          PRINT_SDL_ERROR ( "SDL_CreateWindow" );
          return false;
     }

     // attempt to get the window surface
     m_window_surface = SDL_GetWindowSurface ( m_window );

     if ( !m_window_surface ) {
          PRINT_SDL_ERROR ( "SDL_GetWindowSurface" );
          return false;
     }

     return true;
}

bool Platform::run_game ( )
{
     //if ( !game_init ( ) ) {
          //return false;
     //}

     bool done = false;
     SDL_Event sdl_event;

     while ( !done ) {

          while ( SDL_PollEvent ( &sdl_event ) ) {
               if ( sdl_event.type == SDL_QUIT ) {
                    done = true;
               }

               //game_handle_event ( );
          }

          //game_update ( );
          //game_draw ( );

          SDL_UpdateWindowSurface ( m_window );
     }

     //game_destroy ( );

     return 0;
}

