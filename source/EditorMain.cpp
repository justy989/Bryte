#ifdef LINUX

#include <cstdio>

#include <SDL2/SDL.h>

#include "Application.hpp"
#include "Editor.hpp"

Void print_help ( )
{
     printf ( "Bryte Application\n" );
     printf ( "Usage: ./bryte [ options ]\n" );
     printf ( "  -i filename of map to load\n" );
     printf ( "  -o when saved, output filename\n" );
     printf ( "  -s tilesheet to load\n" );
     printf ( "  -r region to default exits to using\n" );
     printf ( "  -w starting map tiles wide\n" );
     printf ( "  -t starting map tiles tall\n" );
     printf ( "  -h displays this helpful information\n\n" );
}

Int32 main ( Int32 argc, Char8** argv )
{
     Application::Settings settings;

     settings.window_title                  = "Bryte editor 0.01";
     settings.window_width                  = 1280;
     settings.window_height                 = 1024;

     settings.back_buffer_width             = 256;
     settings.back_buffer_height            = 240;

     settings.shared_library_path           = "./bryte_editor.so";

     settings.game_memory_allocation_size   = MEGABYTES ( 32 );

     settings.locked_frames_per_second      = 30;

     editor::Settings editor_settings;

     editor_settings.region = 0;

     editor_settings.map_width  = 8;
     editor_settings.map_height = 8;

     editor_settings.map_tilesheet_filename  = "castle_tilesheet.bmp";
     editor_settings.map_decorsheet_filename = "castle_decorsheet.bmp";
     editor_settings.map_lampsheet_filename  = "castle_lampsheet.bmp";
     editor_settings.map_rat_filename        = "test_rat.bmp";
     editor_settings.map_save_filename       = "map.brm";
     editor_settings.map_load_filename       = nullptr;

     for ( int i = 1; i < argc; ++i ) {
          if ( strcmp ( argv [ i ], "-h" ) == 0 ) {
               print_help ( );
               return 0;
          } else if ( strcmp ( argv [ i ], "-r" ) == 0 ) {
               if ( argc >= i + 1 ) {
                    editor_settings.region = atoi ( argv [ i + 1 ] );
                    ++i;
               }
          } else if ( strcmp ( argv [ i ], "-w" ) == 0 ) {
               if ( argc >= i + 1 ) {
                    editor_settings.map_width = atoi ( argv [ i + 1 ] );
                    ++i;
               }
          } else if ( strcmp ( argv [ i ], "-t" ) == 0 ) {
               if ( argc >= i + 1 ) {
                    editor_settings.map_height = atoi ( argv [ i + 1 ] );
                    ++i;
               }
          } else if ( strcmp ( argv [ i ], "-o" ) == 0 ) {
               if ( argc >= i + 1 ) {
                    editor_settings.map_save_filename = argv [ i + 1 ];
                    ++i;
               }
          } else if ( strcmp ( argv [ i ], "-s" ) == 0 ) {
               if ( argc >= i + 1 ) {
                    editor_settings.map_tilesheet_filename = argv [ i + 1 ];
                    ++i;
               }
          } else if ( strcmp ( argv [ i ], "-i" ) == 0 ) {
               if ( argc >= i + 1 ) {
                    editor_settings.map_load_filename = argv [ i + 1 ];
                    ++i;
               }
          } else {
               printf ( "unrecognized option: %s, see help.\n", argv [ i ] );
               return 0;
          }
     }

     Application application;

     return application.run_game ( settings, &editor_settings ) ? 0 : 1;
}

#endif
