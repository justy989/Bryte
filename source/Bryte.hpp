#ifndef BRYTE_HPP
#define BRYTE_HPP

#include "Types.hpp"

#include <SDL2/SDL.h>

namespace bryte {

     enum Direction {
          left = 0 ,
          up,
          right,
          down
     };

     struct Map {
          Int8* m_tiles;

          Int8  m_tile_width;
          Int8  m_tile_height;

          Int8  m_width;
          Int8  m_height;

          // TODO: exits to other maps

          Void build ( );
          Void render ( SDL_Surface* surface, Real32 camera_x, Real32 camera_y );
          Bool is_tile_solid ( Int32 x, Int32 y );
     };

     struct GameMemory {

          GameMemory ( ) : memory ( nullptr ), size ( 0 ) { }

          Void*  memory;
          Uint32 size;
     };

     struct GameState {
          Real32 player_position_x;
          Real32 player_position_y;

          Bool   direction_keys [ 4 ];

          Map    map;

          Real32 camera_x;
          Real32 camera_y;
     };
}

extern "C" Bool bryte_init ( bryte::GameMemory& );
extern "C" Void bryte_destroy ( );
extern "C" Void bryte_reload_memory ( bryte::GameMemory& );
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

