#include <cstdio>

#include <SDL2/SDL.h>

#include "Platform.hpp"

static const int c_window_width  = 800;
static const int c_window_height = 600;


int main ( int argc, char** argv )
{
     Platform platform;

     if ( !platform.create_window ( "bryte 0.01", c_window_width, c_window_height ) ) {
          return 1;
     }

#if 0 // TODO: create game shared object to load
     if ( !platform.load_game_code ( "bryte.so" ) ) {
          return 1;
     }
#endif

     return platform.run_game ( ) ? 0 : 1;
}

