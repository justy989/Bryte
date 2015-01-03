#ifndef BRYTE_HPP
#define BRYTE_HPP

#include "Defines.hpp"
#include "GameMemory.hpp"
#include "Map.hpp"

#include <SDL2/SDL.h>

namespace bryte
{
     enum Direction {
          left = 0 ,
          up,
          right,
          down,
          count
     };

     struct GameState {
     public:

          Real32 player_position_x;
          Real32 player_position_y;

          Int32  player_exit_tile_index;

          Bool   direction_keys [ Direction::count ];

          Map    map;

          Real32 camera_x;
          Real32 camera_y;
     };

}

extern "C" Bool bryte_init ( GameMemory& );
extern "C" Void bryte_destroy ( );
extern "C" Void bryte_reload_memory ( GameMemory& );
extern "C" Void bryte_user_input ( SDL_Scancode, bool );
extern "C" Void bryte_update ( Real32 );
extern "C" Void bryte_render ( SDL_Surface* );

// Game code function types
using GameInitFunc         = decltype ( bryte_init )*;
using GameDestroyFunc      = decltype ( bryte_destroy )*;
using GameReloadMemoryFunc = decltype ( bryte_reload_memory )*;
using GameUserInputFunc    = decltype ( bryte_user_input )*;
using GameUpdateFunc       = decltype ( bryte_update )*;
using GameRenderFunc       = decltype ( bryte_render )*;

#endif

