#ifndef BRYTE_PLATFORM_HPP
#define BRYTE_PLATFORM_HPP

#include <SDL2/SDL.h>

#define PRINT_SDL_ERROR(sdl_api) printf ( "%s() failed: %s\n", sdl_api, SDL_GetError ( ) );

class Platform {
public:

     Platform ( );
     ~Platform ( );

     bool create_window ( const char* window_title, int window_width, int window_height );

     bool run_game ( );

private:

     SDL_Window* m_window;
     SDL_Surface* m_window_surface;
};

#endif
