/* sdl_window: Interface to sdl to fit our exact needs. We want to draw to a
 *             fixed size backbuffer surface. Then use a renderer to stretch it
 *             to be however large the window is.
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

          void render ( );

          inline SDL_Surface* back_buffer ( );

          inline int width ( ) const;
          inline int height ( ) const;

          static const int k_back_buffer_width = 256;
          static const int k_back_buffer_height = 240;

     private:

          SDL_Window* m_window;
          SDL_Renderer* m_renderer;
          SDL_Surface* m_back_buffer_surface;
          SDL_Texture* m_back_buffer_texture;

          int m_width;
          int m_height;
     };

     inline SDL_Surface* sdl_window::back_buffer ( ) { return m_back_buffer_surface; }
     inline int sdl_window::width ( ) const { return m_width; }
     inline int sdl_window::height ( ) const { return m_height; }
}

#endif