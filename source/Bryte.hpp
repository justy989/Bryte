#ifndef BRYTE_HPP
#define BRYTE_HPP

#include "Utils.hpp"

#include "GameMemory.hpp"
#include "GameInput.hpp"

#include "Map.hpp"
#include "Character.hpp"

#include "Random.hpp"

#include <SDL2/SDL.h>

namespace bryte
{
     struct Lever {
          Real32 position_x;
          Real32 position_y;

          Int32  activate_tile_x;
          Int32  activate_tile_y;

          Real32 activate_time;
     };

     struct HealthPickup {

          static const Real32 c_dimension;

          Real32 position_x = 0.0f;
          Real32 position_y = 0.0f;

          Bool available = false;
     };

     struct State {
     public:

          Bool initialize ( GameMemory& game_memory );
          Void destroy    ( );

          Bool spawn_enemy ( Real32 x, Real32 y );

     public:

          static const Uint32 c_max_enemies = 32;
          static const Uint32 c_max_health_pickups = 8;

     public:

          Random    random;

          Character player;
          Int32     player_exit_tile_index;

          Character enemies [ c_max_enemies ];
          Uint32    enemy_count;

          Lever        lever;

          HealthPickup health_pickups [ c_max_health_pickups ];

          Bool      direction_keys [ Direction::count ];
          Bool      attack_key;
          Bool      activate_key;

          Map       map;

          Real32    camera_x;
          Real32    camera_y;

          SDL_Surface* tilesheet;
     };

     struct MemoryLocations {
          State*     state;
          Map::Room* rooms;

          Void calculate_locations ( GameMemory* memory );
     };
}

// exported functions to be called by the application
extern "C" Bool game_init ( GameMemory& );
extern "C" Void game_destroy ( GameMemory& );
extern "C" Void game_user_input ( GameMemory&, const GameInput& );
extern "C" Void game_update ( GameMemory&, Real32 );
extern "C" Void game_render ( GameMemory&, SDL_Surface* );

// exported function types
using GameInitFunc         = decltype ( game_init )*;
using GameDestroyFunc      = decltype ( game_destroy )*;
using GameUserInputFunc    = decltype ( game_user_input )*;
using GameUpdateFunc       = decltype ( game_update )*;
using GameRenderFunc       = decltype ( game_render )*;

#endif

