#ifndef BRYTE_HPP
#define BRYTE_HPP

#include "Utils.hpp"

#include "GameMemory.hpp"
#include "GameInput.hpp"

#include "Map.hpp"
#include "Interactives.hpp"
#include "Character.hpp"
#include "Enemy.hpp"
#include "Pickup.hpp"

#include "Random.hpp"

#include "Vector.hpp"

#include "MapDisplay.hpp"
#include "CharacterDisplay.hpp"
#include "InteractivesDisplay.hpp"

#include "Text.hpp"

#include <SDL2/SDL.h>

namespace bryte
{
     struct Settings {
          const Char8* map_master_list_filename;

          Uint32       map_index;

          Int32        player_spawn_tile_x;
          Int32        player_spawn_tile_y;
     };

     struct State {
     public:

          Bool initialize ( GameMemory& game_memory, Settings* settings );
          Void destroy    ( );

          Bool spawn_enemy ( Real32 x, Real32 y, Uint8 id, Direction facing, Pickup::Type drop );
          Bool spawn_pickup ( Real32 x, Real32 y, Pickup::Type type );

          Void spawn_map_enemies ( );
          Void clear_enemies ( );

          Void player_death ( );

     public:

          static const Uint32 c_max_enemies = 32;
          static const Uint32 c_max_pickups = 8;

     public:

          Random       random;

          Character    player;
          Int32        player_exit_tile_index;

          Enemy        enemies [ c_max_enemies ];
          Uint32       enemy_count;

          Pickup       pickups [ c_max_pickups ];

          Map          map;
          Interactives interactives;

          Vector       camera;

          Text text;

          MapDisplay          map_display;
          CharacterDisplay    character_display;
          InteractivesDisplay interactives_display;

          SDL_Surface*        pickup_sheet;

          Bool  direction_keys [ Direction::count ];
          Bool  attack_key;
          Bool  activate_key;

          Int32 player_spawn_tile_x;
          Int32 player_spawn_tile_y;

          Int32 player_key_count;
     };

     struct MemoryLocations {
          State*     state;

          Void calculate_locations ( GameMemory* memory );
     };
}

// exported functions to be called by the application
extern "C" Bool game_init       ( GameMemory&, void* settings );
extern "C" Void game_destroy    ( GameMemory& );
extern "C" Void game_user_input ( GameMemory&, const GameInput& );
extern "C" Void game_update     ( GameMemory&, Real32 );
extern "C" Void game_render     ( GameMemory&, SDL_Surface* );

#endif

