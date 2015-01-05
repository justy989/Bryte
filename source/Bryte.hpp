#ifndef BRYTE_HPP
#define BRYTE_HPP

#include "Utils.hpp"

#include "GameMemory.hpp"
#include "GameInput.hpp"

#include "Map.hpp"

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

     struct Stopwatch {
     public:

          Void reset ( Real32 remaining );

          Bool tick ( Real32 delta );

     public:

          Real32 remaining;
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
          Stopwatch damage_time;
          Stopwatch blink_time;

          Stopwatch attack_time;
          Stopwatch cooldown_time;
     };

     struct Lever {
          Real32 position_x;
          Real32 position_y;

          Int32  activate_tile_x;
          Int32  activate_tile_y;

          Real32 activate_time;
     };

     struct Random {
     public:

          Random ( ) : i_f ( 0 ), i_s ( 0 ) { }

          Void   seed     ( Uint32 value );
          Uint32 generate ( Uint32 min, Uint32 max ); // not inclusive!

     public:

          Uint32 i_f;
          Uint32 i_s;
     };

     struct GameState {
     public:

          Bool initialize ( );

          Bool spawn_enemy ( Real32 x, Real32 y );

     public:

          static const Uint32 c_max_enemies = 32;

     public:

          Random    random;

          Character player;
          Int32     player_exit_tile_index;

          Character enemies [ c_max_enemies ];
          Uint32    enemy_count;

          Lever     lever;

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
extern "C" Bool bryte_init ( GameMemory& );
extern "C" Void bryte_destroy ( );
extern "C" Void bryte_reload_memory ( GameMemory& );
extern "C" Void bryte_user_input ( const GameInput& );
extern "C" Void bryte_update ( Real32 );
extern "C" Void bryte_render ( SDL_Surface* );

// exported function types
using GameInitFunc         = decltype ( bryte_init )*;
using GameDestroyFunc      = decltype ( bryte_destroy )*;
using GameReloadMemoryFunc = decltype ( bryte_reload_memory )*;
using GameUserInputFunc    = decltype ( bryte_user_input )*;
using GameUpdateFunc       = decltype ( bryte_update )*;
using GameRenderFunc       = decltype ( bryte_render )*;

#endif

