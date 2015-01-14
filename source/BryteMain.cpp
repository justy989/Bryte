#include <cstdio>

#include <SDL2/SDL.h>

#include "Application.hpp"
#include "Bryte.hpp"

void print_help ( )
{
     printf ( "Bryte Application\n" );
     printf ( "Usage: ./bryte [ options ]\n" );
     printf ( "  -m filename of master map list\n" );
     printf ( "  -i map index to load from master list\n" );
     printf ( "  -h displays this helpful information\n\n" );
}

int main ( int argc, char** argv )
{
     Application::Settings settings;

     settings.window_title                  = "Bryte 0.01";
     settings.window_width                  = 1024;
     settings.window_height                 = 768;

     settings.back_buffer_width             = 256;
     settings.back_buffer_height            = 240;

     settings.shared_library_path           = "./bryte_game.so";

     settings.game_memory_allocation_size   = MEGABYTES ( 32 );

     settings.locked_frames_per_second      = 30;

     bryte::Settings bryte_settings;

     bryte_settings.map_master_list_filename = "map_list.txt";
     bryte_settings.map_index = 0;

     for ( int i = 1; i < argc; ++i ) {
          if ( strcmp ( argv [ i ], "-h" ) == 0 ) {
               print_help ( );
               return 0;
          } else if ( strcmp ( argv [ i ], "-m" ) == 0 ) {
               if ( argc >= i + 1 ) {
                    bryte_settings.map_master_list_filename = argv [ i + 1 ];
                    ++i;
               }
          } else if ( strcmp ( argv [ i ], "-i" ) == 0 ) {
               if ( argc >= i + 1 ) {
                    bryte_settings.map_index = atoi ( argv [ i + 1 ] );
                    ++i;
               }
          } else {
               printf ( "unrecognized option: %s, see help.\n", argv [ i ] );
               return 0;
          }
     }

     Application application;

     return application.run_game ( settings, &bryte_settings ) ? 0 : 1;
}

