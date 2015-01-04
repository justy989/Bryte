#ifndef BRYTE_MAP_HPP
#define BRYTE_MAP_HPP

#include "Types.hpp"

namespace bryte
{
     class Map {
     public:

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

               Uint8* tiles;

               Uint8  width;
               Uint8  height;

               Exit   exits [ c_max_exits ];
               Uint8  exit_count;
          };

     public:

          Map ( );

          Void build  ( );

          Int32 position_to_tile_index     ( Real32 x, Real32 y );

          Int32 coordinate_to_tile_index   ( Int32 tile_x, Int32 tile_y );
          Int32 tile_index_to_coordinate_x ( Int32 tile_index );
          Int32 tile_index_to_coordinate_y ( Int32 tile_index );

          Bool  is_position_solid   ( Real32 x, Real32 y );
          Int32 check_position_exit ( Real32 x, Real32 y );

          inline Int32 width ( ) const;
          inline Int32 height ( ) const;

     public:

          static const Real32 c_tile_dimension;

          static const Uint8 c_max_rooms = 8;

     public:

          Room* m_current_room;
     };

     inline Int32 Map::width ( ) const
     {
          return m_current_room->width;
     }

     inline Int32 Map::height ( ) const
     {
          return m_current_room->height;
     }

}

#endif

