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

     struct Character {
     public:

          Bool collides_with ( Real32 new_x, Real32 new_y, const Character& character );
          Bool attack_collides_with ( const Character& character );

          Void attack ( );
          Void damage ( Int32 amount, Direction push );

          Void update ( Real32 time_delta );

     public:

          Real32 position_x;
          Real32 position_y;

          Real32 velocity_x;
          Real32 velocity_y;

          Real32 width;
          Real32 height;

          Real32 collision_height;

          Int32  health;
          Int32  max_health;

          // you are pushed when damaged
          Real32 damage_move_x;
          Real32 damage_move_y;

          // the area attacked
          Real32 attack_x;
          Real32 attack_y;

          Real32 attack_time;
          Real32 cooldown_time;

          Direction facing;

     };

     struct GameState {
     public:

          Character player;
          Character enemy;

          Int32     player_exit_tile_index;

          Bool      direction_keys [ Direction::count ];
          Bool      attack_key;

          Map       map;

          Real32    camera_x;
          Real32    camera_y;
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

