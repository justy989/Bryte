#include "Platform.hpp"

#include <cstdio>

Platform::Platform ( ) :
     m_window               ( nullptr ),
     m_renderer             ( nullptr ),
     m_back_buffer_texture  ( nullptr ),
     m_back_buffer_surface  ( nullptr ),
     m_game_init_func       ( nullptr ),
     m_game_destroy_func    ( nullptr ),
     m_game_user_input_func ( nullptr ),
     m_game_update_func     ( nullptr ),
     m_game_render_func     ( nullptr )
{
     SDL_Init ( SDL_INIT_VIDEO );
}

Platform::~Platform ( )
{
     if ( m_back_buffer_surface ) {
          SDL_FreeSurface ( m_back_buffer_surface );
     }

     if ( m_back_buffer_texture ) {
          SDL_DestroyTexture ( m_back_buffer_texture );
     }

     if ( m_renderer ) {
          SDL_DestroyRenderer ( m_renderer );
     }

     if ( m_window ) {
          SDL_DestroyWindow ( m_window );
     }

     SDL_Quit ( );
}

bool Platform::create_window ( const char* window_title, int window_width, int window_height,
                               int back_buffer_width, int back_buffer_height )
{
     // create the window with the specified parameters
     m_window = SDL_CreateWindow ( window_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   window_width, window_height, 0 );

     if ( !m_window ) {
          PRINT_SDL_ERROR ( "SDL_CreateWindow" );
          return false;
     }

     m_renderer = SDL_CreateRenderer ( m_window, -1, SDL_RENDERER_ACCELERATED );

     if ( !m_renderer ) {
          PRINT_SDL_ERROR ( "SDL_CreateRenderer" );
          return false;
     }

     // attempt to create a surface to draw on
     m_back_buffer_surface = SDL_CreateRGBSurface ( 0, back_buffer_width, back_buffer_height, 32,
                                                    0, 0, 0, 0 );

     if ( !m_back_buffer_surface ) {
          PRINT_SDL_ERROR ( "SDL_CreateRGBSurface" );
          return false;
     }

     m_back_buffer_texture = SDL_CreateTextureFromSurface ( m_renderer, m_back_buffer_surface );

     if ( !m_back_buffer_texture ) {
          PRINT_SDL_ERROR ( "SDL_CreateTextureFromSurface" );
          return false;
     }

     return true;
}

bool Platform::run_game ( )
{
     bool done = false;
     SDL_Event sdl_event;

     while ( !done ) {

          while ( SDL_PollEvent ( &sdl_event ) ) {
               if ( sdl_event.type == SDL_QUIT ) {
                    done = true;
               }
          }

          SDL_Rect clear_rect { 0, 0, m_back_buffer_surface->w, m_back_buffer_surface->h };
          SDL_Rect tmp_rect { 16, 16, 16, 16 };

          Uint32 black = SDL_MapRGB ( m_back_buffer_surface->format, 0, 0, 0 );
          Uint32 red = SDL_MapRGB ( m_back_buffer_surface->format, 255, 0, 0 );

          SDL_FillRect ( m_back_buffer_surface, &clear_rect, black );
          SDL_FillRect ( m_back_buffer_surface, &tmp_rect, red );

          SDL_UpdateTexture ( m_back_buffer_texture, nullptr, m_back_buffer_surface->pixels,
                              m_back_buffer_surface->pitch );

          SDL_RenderClear ( m_renderer );
          SDL_RenderCopy ( m_renderer, m_back_buffer_texture, nullptr, nullptr );

          SDL_RenderPresent ( m_renderer );
     }

     return 0;
}

