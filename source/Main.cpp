#include <cstdio>

#include <SDL2/SDL.h>

#include "Platform.hpp"

static const int c_window_width  = 800;
static const int c_window_height = 600;

static const int c_back_buffer_width = 256;
static const int c_back_buffer_height = 240;

int main ( int argc, char** argv )
{
     Platform platform;

     if ( !platform.create_window ( "bryte 0.01", c_window_width, c_window_height,
                                    c_back_buffer_width, c_back_buffer_height ) ) {
          return 1;
     }

     if ( !platform.load_game_code ( "./bryte_game.so" ) ) {
          return 1;
     }

     return platform.run_game ( ) ? 0 : 1;
}

