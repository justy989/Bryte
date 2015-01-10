#include <cstdio>

#include <SDL2/SDL.h>

#include "Application.hpp"
#include "Utils.hpp"
#include "Log.hpp"

void print_help ( )
{
     printf ( "Bryte Application\n" );
     printf ( "Usage: ./bryte [ options ]\n" );
     printf ( "  -b run bryte (default)\n" );
     printf ( "  -e run the editor\n\n" );
}

int run_bryte ( )
{
     Application platform;
     Application::Settings settings;

     settings.window_title                  = "Bryte 0.01";
     settings.window_width                  = 1024;
     settings.window_height                 = 768;

     settings.back_buffer_width             = 256;
     settings.back_buffer_height            = 240;

     settings.shared_library_path           = "./bryte_game.so";

     settings.game_memory_allocation_size   = MEGABYTES ( 32 );

     settings.locked_frames_per_second      = 30;

     return platform.run_game ( settings ) ? 0 : 1;
}

int run_editor ( )
{
     Application platform;
     Application::Settings settings;

     settings.window_title                  = "Bryte Editor 0.01";
     settings.window_width                  = 1024;
     settings.window_height                 = 768;

     settings.back_buffer_width             = 256;
     settings.back_buffer_height            = 240;

     settings.shared_library_path           = "./bryte_editor.so";

     settings.game_memory_allocation_size   = MEGABYTES ( 32 );

     settings.locked_frames_per_second      = 30;

     return platform.run_game ( settings ) ? 0 : 1;
}

int main ( int argc, char** argv )
{
     for ( int i = 1; i < argc; ++i ) {
          if ( strcmp ( argv [ i ], "-b" ) == 0 ) {
               return run_bryte ( );
          } else if ( strcmp ( argv [ i ], "-e" ) == 0 ) {
               return run_editor ( );
          } else if ( strcmp ( argv [ i ], "-h" ) == 0 ) {
               print_help ( );
               return 0;
          } else {
               printf ( "Unrecognized option: %s, see help.\n", argv [ i ] );
               return 0;
          }
     }

     return run_bryte ( );
}

