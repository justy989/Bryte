#include "sdl_window.hpp"

#include <stdexcept>

using namespace bryte;

sdl_window::sdl_window ( const char* title, int width, int height ) :
     m_window ( nullptr ),
     m_gl_context ( 0 )
{
     // create the window
     m_window = SDL_CreateWindow ( title,
                                   SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   width, height,
                                   SDL_WINDOW_OPENGL );

     if ( !m_window ) {
          throw std::runtime_error ( std::string ( "SDL_CreateWindow() failed: " ) + SDL_GetError ( ) );
     }

     // create the opengl context for the window
     m_gl_context = SDL_GL_CreateContext ( m_window );

     if ( !m_gl_context ) {
          throw std::runtime_error ( std::string ( "SDL_GL_CreateContext() failed: " ) + SDL_GetError ( ) );
     }

     // finish by setting up openGL the way we want it
     setup_open_gl ( );
}

sdl_window::~sdl_window ( )
{
     SDL_GL_DeleteContext ( m_gl_context );
     SDL_DestroyWindow ( m_window );
}

void sdl_window::setup_open_gl ( )
{
     // set the opengl version
     SDL_GL_SetAttribute ( SDL_GL_CONTEXT_MAJOR_VERSION, 2 );
     SDL_GL_SetAttribute ( SDL_GL_CONTEXT_MINOR_VERSION, 1 );

     // turn off vsync, literally the worst 'feature' ever invented
     SDL_GL_SetSwapInterval ( 0 );

     // set the clear color
     glClearColor ( 0.0f, 0.0f, 0.0f, 1.0f );

     // clear matrices
     glMatrixMode ( GL_PROJECTION );
     glLoadIdentity ( );

     glMatrixMode ( GL_MODELVIEW );
     glLoadIdentity ( );

     // set our viewport
     glOrtho ( -1.0, 1.0, -1.0, 1.0, -0.05, 0.05 );
}

void sdl_window::render_backbuffer ( const SDL_Surface* surface )
{
     glClear ( GL_COLOR_BUFFER_BIT );

     // temporary just to see that opengl works
     glBegin ( GL_QUADS );
     glVertex2f ( -0.5f, -0.5f );
     glVertex2f ( 0.5f, -0.5f );
     glVertex2f ( 0.5f, 0.5f );
     glVertex2f ( -0.5f, 0.5f );
     glEnd ( );

     SDL_GL_SwapWindow ( m_window );
}
