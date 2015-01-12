#include <cstdio>

#include <SDL2/SDL.h>

#include "Application.hpp"

void print_help ( )
{
     printf ( "Bryte Application\n" );
     printf ( "Usage: ./bryte [ options ]\n" );
     printf ( "  -h displays this helpful information\n\n" );
}

int main ( int argc, char** argv )
{
     for ( int i = 1; i < argc; ++i ) {
          if ( strcmp ( argv [ i ], "-h" ) == 0 ) {
               print_help ( );
               return 0;
          } else {
               printf ( "unrecognized option: %s, see help.\n", argv [ i ] );
               return 0;
          }
     }

     Application application;
     Application::Settings settings;

     settings.window_title                  = "Bryte 0.01";
     settings.window_width                  = 1024;
     settings.window_height                 = 768;

     settings.back_buffer_width             = 256;
     settings.back_buffer_height            = 240;

     settings.shared_library_path           = "./bryte_game.so";

     settings.game_memory_allocation_size   = MEGABYTES ( 32 );

     settings.locked_frames_per_second      = 30;

     return application.run_game ( settings, nullptr ) ? 0 : 1;
}

