#include <cstdio>

#include <SDL2/SDL.h>

#include "Application.hpp"
#include "Editor.hpp"

void print_help ( )
{
     printf ( "Bryte Application\n" );
     printf ( "Usage: ./bryte [ options ]\n" );
     printf ( "  -o when saved, output filename\n" );
     printf ( "  -s tilesheet to load\n" );
     printf ( "  -w initial map width ( tiles wide )\n" );
     printf ( "  -t initial map height ( tiles tall )\n" );
     printf ( "  -h displays this helpful information\n\n" );
}

int main ( int argc, char** argv )
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

     editor_settings.map_width  = 8;
     editor_settings.map_height = 8;

     editor_settings.map_tilesheet_filename = "castle_tilesheet.bmp";
     editor_settings.map_save_filename      = "map.brm";

     for ( int i = 1; i < argc; ++i ) {
          if ( strcmp ( argv [ i ], "-h" ) == 0 ) {
               print_help ( );
               return 0;
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
          }  else {
               printf ( "unrecognized option: %s, see help.\n", argv [ i ] );
               return 0;
          }
     }

     Application application;

     return application.run_game ( settings, &editor_settings ) ? 0 : 1;
}

