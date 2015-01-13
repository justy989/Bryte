#ifndef BRYTE_EDITOR_HPP
#define BRYTE_EDITOR_HPP

#include <SDL2/SDL.h>

#include "GameMemory.hpp"
#include "GameInput.hpp"

#include "Map.hpp"

#include "Text.hpp"

namespace editor
{
     struct Settings {
          const char* map_load_filename;

          Int32 map_width;
          Int32 map_height;

          const char* map_tilesheet_filename;

          const char* map_save_filename;
     };

     struct State {
     public:

          static const Real32 c_camera_speed;

     public:

          Settings* settings;

          Text text;

          bryte::Map map;

          Int32  mouse_x;
          Int32  mouse_y;

          Real32 camera_x;
          Real32 camera_y;

          Bool camera_direction_keys [ 4 ];
          Bool left_button_down;
          Bool right_button_down;

          SDL_Surface* tilesheet;

          Uint8 current_tile;

          Bool draw_solids;
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

