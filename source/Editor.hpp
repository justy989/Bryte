#ifndef BRYTE_EDITOR_HPP
#define BRYTE_EDITOR_HPP

#include <SDL2/SDL.h>

#include "GameMemory.hpp"
#include "GameInput.hpp"

#include "Map.hpp"
#include "Interactives.hpp"

#include "Text.hpp"
#include "Vector.hpp"

#include "InteractivesDisplay.hpp"

namespace editor
{
     struct Settings {
          const char* map_load_filename;

          Int32 map_width;
          Int32 map_height;

          const char* map_tilesheet_filename;
          const char* map_decorsheet_filename;
          const char* map_lampsheet_filename;

          const char* map_rat_filename;

          const char* map_save_filename;
     };

     enum Mode {
          tile = 0,
          decor,
          light,
          enemy,
          exit,
          lever,
          pushable_block,
          torch,
          pushable_torch,
          count
     };

     struct State {
     public:

          static const Real32 c_camera_speed;

          Void mouse_button_changed_down ( bool left );
          Void option_button_changed_down ( bool up );
          Void option_scroll ( Int32 scroll );

     public:

          Settings* settings;

          Text text;

          bryte::Map map;
          bryte::Interactives interactives;

          Mode mode;

          Int32  mouse_x;
          Int32  mouse_y;
          Int32  mouse_screen_x;
          Int32  mouse_screen_y;
          Int32  mouse_tile_x;
          Int32  mouse_tile_y;

          Vector camera;

          Bool camera_direction_keys [ 4 ];
          Bool left_button_down;
          Bool right_button_down;

          SDL_Surface* mode_icons_surface;

          SDL_Surface* tilesheet;
          SDL_Surface* decorsheet;
          SDL_Surface* lampsheet;

          SDL_Surface* rat_surface;

          bryte::InteractivesDisplay interactives_display;

          Uint8 current_tile;
          Bool  current_solid;
          Uint8 current_decor;
          Uint8 current_lamp;
          Uint8 current_exit_direction;
          Uint8 current_exit_state;
          Uint8 current_torch;
          Uint8 current_pushable_torch;
          Uint8 current_field;

          Bool   track_current_interactive;
          Uint32 current_interactive_x;
          Uint32 current_interactive_y;

          Bool draw_solids;
          Bool draw_light;

          char message_buffer [ 128 ];
     };

     struct MemoryLocations {
          State* state;
     };
}

// exported functions to be called by the application
extern "C" Bool game_init       ( GameMemory&, void* );
extern "C" Void game_destroy    ( GameMemory& );
extern "C" Void game_user_input ( GameMemory&, const GameInput& );
extern "C" Void game_update     ( GameMemory&, Real32 );
extern "C" Void game_render     ( GameMemory&, SDL_Surface* );

#endif

