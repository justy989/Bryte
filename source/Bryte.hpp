#ifndef BRYTE_HPP
#define BRYTE_HPP

#include "GameMemory.hpp"

#include <SDL2/SDL.h>

namespace bryte {

      enum Direction {
          left = 0 ,
          up,
          right,
          down
     };

     struct Room;

     struct Exit {
     public:

          Uint8 location_x;
          Uint8 location_y;

          Uint8 room_index;

          Uint8 destination_x;
          Uint8 destination_y;
     };

     struct Room {
     public:

          static const Uint8 c_max_width  = 255;
          static const Uint8 c_max_height = 255;

          static const Uint8 c_max_exits = 4;

     public:

          Uint8* m_tiles;

          Uint8  m_width;
          Uint8  m_height;

          Exit   m_exits [ c_max_exits ];
          Uint8  m_exit_count;
     };

     struct Map {
     public:

          Void   build  ( );

          Void   render ( SDL_Surface* surface, Real32 camera_x, Real32 camera_y );

          Uint32 map_to_tile_index    ( Uint32 x, Uint32 y );

          Uint32 get_tile_index       ( Uint32 tile_x, Uint32 tile_y );
          Uint32 get_tile_index_x     ( Uint32 tile_index );
          Uint32 get_tile_index_y     ( Uint32 tile_index );

          Bool   is_tile_solid_on_map ( Uint32 x, Uint32 y );

          Uint32 check_player_exit    ( Uint32 x, Uint32 y );

     public:
          static const Uint8 c_max_rooms = 8;

          Room* m_room;

          Uint8 m_tile_dimension;
     };

     struct GameState {
     public:

          static const Uint8 c_max_directions = 4;

     public:
          Real32 player_position_x;
          Real32 player_position_y;

          Uint32 player_exit_destination;

          Bool   direction_keys [ c_max_directions ];

          Map    map;

          Real32 camera_x;
          Real32 camera_y;
     };

     struct MemoryLocations {
     public:

          MemoryLocations ( ) : game_state ( 0 ) { }

     public:

          GameState* game_state;

          Room*      rooms;
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

