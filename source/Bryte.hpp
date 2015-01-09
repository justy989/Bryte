#ifndef BRYTE_HPP
#define BRYTE_HPP

#include "Utils.hpp"

#include "GameMemory.hpp"
#include "GameInput.hpp"

#include "Map.hpp"

#include "StopWatch.hpp"
#include "Random.hpp"

#include <SDL2/SDL.h>

namespace bryte
{
     enum Direction {
          left = 0,
          up,
          right,
          down,
          count
     };

     struct Character {
     public:

          enum State {
               dead,
               spawning,
               alive,
               blinking,
               attacking,
               dying,
          };

     public:

          Bool collides_with ( const Character& character );
          Bool attack_collides_with ( const Character& character );

          Void attack ( );
          Void damage ( Int32 amount, Direction push );

          Void update ( Real32 time_delta );

          Real32 calc_attack_x ( );
          Real32 calc_attack_y ( );

     public:

          State     state;
          Direction facing;

          Stopwatch state_watch;

          Int32  health;
          Int32  max_health;

          Real32 position_x;
          Real32 position_y;

          Real32 velocity_x;
          Real32 velocity_y;

          Real32 width;
          Real32 height;

          Real32 collision_height;

          Direction damage_pushed;

          Stopwatch damage_watch;
          Stopwatch cooldown_watch;
     };

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


     struct GameState {
     public:

          Bool initialize ( );
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
}

// exported functions to be called by the application
extern "C" Bool game_init ( GameMemory& );
extern "C" Void game_destroy ( );
extern "C" Void game_reload_memory ( GameMemory& );
extern "C" Void game_user_input ( const GameInput& );
extern "C" Void game_update ( Real32 );
extern "C" Void game_render ( SDL_Surface* );

// exported function types
using GameInitFunc         = decltype ( game_init )*;
using GameDestroyFunc      = decltype ( game_destroy )*;
using GameReloadMemoryFunc = decltype ( game_reload_memory )*;
using GameUserInputFunc    = decltype ( game_user_input )*;
using GameUpdateFunc       = decltype ( game_update )*;
using GameRenderFunc       = decltype ( game_render )*;

#endif

