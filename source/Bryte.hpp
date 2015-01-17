#ifndef BRYTE_HPP
#define BRYTE_HPP

#include "Utils.hpp"

#include "GameMemory.hpp"
#include "GameInput.hpp"

#include "Map.hpp"
#include "Character.hpp"

#include "Random.hpp"

#include "Vector.hpp"

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

          Vector position;

          Bool available = false;
     };

     struct Settings {
          const Char8* map_master_list_filename;

          Uint32       map_index;
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

          Vector    camera;

          SDL_Surface* tilesheet;
          SDL_Surface* decorsheet;
          SDL_Surface* lampsheet;
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

