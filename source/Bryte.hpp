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

          Int8 m_tile_width;
          Int8 m_tile_height;

          Int8 m_width;
          Int8 m_height;

          // TODO: Exits to other maps

          Void build ( );
          Void render ( SDL_Surface* surface );
     };

     struct Game_State {

          Real32 player_position_x;
          Real32 player_position_y;

          bool direction_keys [ 4 ];

          Map map;
     };
}

extern "C" Bool bryte_init ( );
extern "C" Void bryte_destroy ( );
extern "C" Void bryte_user_input ( SDL_Scancode, bool );
extern "C" Void bryte_update ( Real32 );
extern "C" Void bryte_render ( SDL_Surface* );

// Game code function types
using Game_Init_Func       = decltype ( bryte_init )*;
using Game_Destroy_Func    = decltype ( bryte_destroy )*;
using Game_User_Input_Func = decltype ( bryte_user_input )*;
using Game_Update_Func     = decltype ( bryte_update )*;
using Game_Render_Func     = decltype ( bryte_render )*;

#endif

