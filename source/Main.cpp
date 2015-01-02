#include <cstdio>

#include <SDL2/SDL.h>

#include "Platform.hpp"
#include "Utils.hpp"

static const int c_window_width       = 800;
static const int c_window_height      = 600;

static const int c_back_buffer_width  = 256;
static const int c_back_buffer_height = 240;

static const int c_fps_limit          = 30;

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

     if ( !platform.allocate_game_memory ( MEGABYTES ( 32 ) ) ) {
          return 1;
     }

     return platform.run_game ( c_fps_limit ) ? 0 : 1;
}

