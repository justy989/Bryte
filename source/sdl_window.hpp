/* sdl_window: interface to sdl to fit our exact needs. 
*/

#ifndef BRYTE_SDL_WINDOW_HPP
#define BRYTE_SDL_WINDOW_HPP

#include <SDL2\SDL.h>
#include <SDL2\SDL_opengl.h>

namespace bryte
{
     class sdl_window {
     public:

          sdl_window ( const char* title, int width, int height );
          ~sdl_window ( );

          void render_backbuffer ( const SDL_Surface* surface );

          inline SDL_Window* window ( );

     private:

          void setup_open_gl ( );

     private:

          SDL_Window* m_window;

          SDL_GLContext m_gl_context;
     };

     inline SDL_Window* sdl_window::window ( ) { return m_window; }
}

#endif