#ifdef LINUX

#ifndef BRYTE_EDITOR_HPP
#define BRYTE_EDITOR_HPP

#include <SDL2/SDL.h>

#include "GameMemory.hpp"
#include "GameInput.hpp"

#include "Map.hpp"
#include "Interactives.hpp"

#include "Text.hpp"
#include "Vector.hpp"

#include "MapDisplay.hpp"
#include "CharacterDisplay.hpp"
#include "InteractivesDisplay.hpp"

namespace editor
{
     struct Settings {
          const Char8* map_load_filename;

          Int32 region;

          Int32 map_width;
          Int32 map_height;

          const Char8* map_tilesheet_filename;
          const Char8* map_decorsheet_filename;
          const Char8* map_lampsheet_filename;

          const Char8* map_rat_filename;

          const Char8* map_save_filename;
     };

     enum Mode {
          tile = 0,
          decor,
          light,
          enemy,
          lever,
          pushable_block,
          torch,
          pushable_torch,
          light_detector,
          exit,
          pressure_plate,
          popup_block,
          all_killed,
          bombable_block,
          turret,
          ice,
          moving_walkway,
          ice_detector,
          secret,
          hole,
          portal,
          border,
          upgrade,
          count
     };

     struct State {
     public:

          static const Real32 c_camera_speed;

          Void mouse_button_left_clicked ( );
          Void mouse_button_right_clicked ( );
          Void option_button_up_pressed ( );
          Void option_button_down_pressed ( );
          Void mouse_scrolled ( Int32 scroll );

          Bool mouse_on_map ( );

          bryte::Interactive& place_or_clear_interactive ( bryte::Interactive::Type type,
                                                           Int32 mouse_tile_x, Int32 mouse_tile_y );

          Void render_upgrade ( SDL_Surface* back_buffer );

          Void render_selected_interactive ( SDL_Surface* back_buffer );

     public:

          Settings* settings;

          Text text;

          bryte::Map map;
          bryte::Interactives interactives;

          SDL_Surface* upgrade_surface;

          SDL_Surface* mode_icons_surface;

          bryte::MapDisplay          map_display;
          bryte::CharacterDisplay    character_display;
          bryte::InteractivesDisplay interactives_display;

          Vector camera;

          Mode mode;

          Int32 mouse_x;
          Int32 mouse_y;
          Int32 mouse_screen_x;
          Int32 mouse_screen_y;
          Int32 mouse_tile_x;
          Int32 mouse_tile_y;

          Bool  camera_direction_keys [ 4 ];
          Bool  left_button_down;
          Bool  right_button_down;

          Uint8 current_tile_flag;
          Uint8 current_tile;
          Bool  current_solid;
          Bool  current_invisible;
          Uint8 current_decor;
          Uint8 current_lamp;
          Uint8 current_enemy;
          Uint8 current_enemy_direction;
          Uint8 current_enemy_drop;
          Uint8 current_exit_direction;
          Uint8 current_exit_state;
          Uint8 current_pushable_solid;
          Uint8 current_torch;
          Uint8 current_pushable_torch;
          Uint8 current_light_detector_bryte;
          Uint8 current_popup_block;
          Uint8 current_field;
          Uint8 current_turret_direction;
          Uint8 current_turret_automatic;
          Uint8 current_moving_walkway;
          Uint8 current_portal;
          Uint8 current_border;
          Uint8 current_upgrade;

          Bool   track_current_interactive;
          Uint32 current_interactive_x;
          Uint32 current_interactive_y;

          Bool draw_solids;
          Bool draw_light;

          Char8 message_buffer [ 128 ];
     };

     struct MemoryLocations {
          State* state;
     };
}

// exported functions to be called by the application
extern "C" Bool game_init       ( GameMemory&, Void* );
extern "C" Void game_destroy    ( GameMemory& );
extern "C" Void game_user_input ( GameMemory&, const GameInput& );
extern "C" Void game_update     ( GameMemory&, Real32 );
extern "C" Void game_render     ( GameMemory&, SDL_Surface* );

#endif

#endif
