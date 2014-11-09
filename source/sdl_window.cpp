#include "sdl_window.hpp"

#include <stdexcept>

using namespace bryte;

sdl_window::sdl_window ( const char* title, int width, int height ) :
     m_window ( nullptr ),
     m_renderer ( nullptr ),
     m_back_buffer_surface ( nullptr ),
     m_back_buffer_texture ( nullptr ),
     m_width ( width ),
     m_height ( height )

{
     // create the window
     m_window = SDL_CreateWindow ( title,
                                   SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   width, height,
                                   SDL_WINDOW_SHOWN );

     if ( !m_window ) {
          throw std::runtime_error ( std::string ( "SDL_CreateWindow() failed: " ) + SDL_GetError ( ) );
     }

     // create the hardware accelerated renderer
     m_renderer = SDL_CreateRenderer ( m_window, -1,
                                       SDL_RENDERER_ACCELERATED );

     if ( !m_renderer ) {
          throw std::runtime_error ( std::string ( "SDL_CreateRenderer() failed: " ) + SDL_GetError ( ) );
     }

     // create a back buffer surface with out specified width and height
     m_back_buffer_surface = SDL_CreateRGBSurface ( 0, k_back_buffer_width, k_back_buffer_height,
                                                    32, 0, 0, 0, 0 );

     if ( !m_back_buffer_surface ) {
          throw std::runtime_error ( std::string ( "SDL_CreateRGBSurface() failed: " ) + SDL_GetError ( ) );
     }

     // create a back buffer texture based on our surface
     m_back_buffer_texture = SDL_CreateTextureFromSurface ( m_renderer, m_back_buffer_surface );

     if ( !m_back_buffer_texture ) {
          throw std::runtime_error ( std::string ( "SDL_CreateTextureFromSurface() failed: " ) + SDL_GetError ( ) );
     }
}

sdl_window::~sdl_window ( )
{
     // free everything we have created
     SDL_DestroyTexture ( m_back_buffer_texture );
     SDL_FreeSurface ( m_back_buffer_surface );
     SDL_DestroyRenderer ( m_renderer );
     SDL_DestroyWindow ( m_window );
}

void sdl_window::render ( )
{
     // update the texture
     SDL_UpdateTexture ( m_back_buffer_texture, nullptr, m_back_buffer_surface->pixels, m_back_buffer_surface->pitch );

     // copy the texture to the back buffer
     SDL_RenderClear ( m_renderer );
     SDL_RenderCopy ( m_renderer, m_back_buffer_texture, nullptr, nullptr );

     // present the back buffer
     SDL_RenderPresent ( m_renderer );
}
