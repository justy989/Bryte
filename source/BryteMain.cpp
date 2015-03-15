#include <cstdio>

#include <SDL2/SDL.h>

#include "Application.hpp"
#include "Bryte.hpp"

Void print_help ( )
{
     printf ( "Bryte Application\n" );
     printf ( "Usage: ./bryte [ options ]\n" );
     printf ( "  -r region info index\n" );
     printf ( "  -i map index to load from master list\n" );
     printf ( "  -x tile x to spawn player on\n" );
     printf ( "  -y tile y to spawn player on\n" );
     printf ( "  -h displays this helpful information\n\n" );
}

Int32 main ( Int32 argc, Char8** argv )
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

     bryte_settings.region_index = 0;
     bryte_settings.map_index = 0;
     bryte_settings.player_spawn_tile_x = 6;
     bryte_settings.player_spawn_tile_y = 2;

     for ( int i = 1; i < argc; ++i ) {
          if ( strcmp ( argv [ i ], "-h" ) == 0 ) {
               print_help ( );
               return 0;
          } else if ( strcmp ( argv [ i ], "-r" ) == 0 ) {
               if ( argc >= i + 1 ) {
                    bryte_settings.region_index = atoi ( argv [ i + 1 ] );
                    ++i;
               }
          } else if ( strcmp ( argv [ i ], "-i" ) == 0 ) {
               if ( argc >= i + 1 ) {
                    bryte_settings.map_index = atoi ( argv [ i + 1 ] );
                    ++i;
               }
          } else if ( strcmp ( argv [ i ], "-x" ) == 0 ) {
               if ( argc >= i + 1 ) {
                    bryte_settings.player_spawn_tile_x = atoi ( argv [ i + 1 ] );
                    ++i;
               }
          } else if ( strcmp ( argv [ i ], "-y" ) == 0 ) {
               if ( argc >= i + 1 ) {
                    bryte_settings.player_spawn_tile_y = atoi ( argv [ i + 1 ] );
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

