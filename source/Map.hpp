#ifndef BRYTE_MAP_HPP
#define BRYTE_MAP_HPP

#include "Types.hpp"

namespace bryte
{
     class Map {
     public:

          struct Tile {
               Uint8 value;
               Bool  solid;
          };

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

               void initialize ( Uint32 width, Uint32 height, Tile* tiles );

               void save ( const Char8* filepath );
               void load ( const Char8* filepath );

          public:

               Tile* tiles;

               Uint8  width;
               Uint8  height;

               Exit   exits [ c_max_exits ];
               Uint8  exit_count;
          };

     public:

          Map ( );

          void set_current_room ( Room* room );

          Int32 position_to_tile_index     ( Real32 x, Real32 y ) const;

          Int32 coordinate_to_tile_index   ( Int32 tile_x, Int32 tile_y ) const;
          Int32 tile_index_to_coordinate_x ( Int32 tile_index ) const;
          Int32 tile_index_to_coordinate_y ( Int32 tile_index ) const;

          Uint8 get_coordinate_value ( Int32 tile_x, Int32 tile_y ) const;
          Bool  get_coordinate_solid ( Int32 tile_x, Int32 tile_y ) const;
          Void  set_coordinate_value ( Int32 tile_x, Int32 tile_y, Uint8 value );
          Void  set_coordinate_solid ( Int32 tile_x, Int32 tile_y, Bool solid );

          Bool  is_position_solid   ( Real32 x, Real32 y ) const;
          const Exit* check_position_exit ( Real32 x, Real32 y ) const;

          inline Int32 width ( ) const;
          inline Int32 height ( ) const;

     public:

          static const Int32  c_tile_dimension_in_pixels = 16;
          static const Real32 c_tile_dimension_in_meters;

          static const Uint8  c_max_rooms = 8;

     public:

          Room* m_current_room;
     };

     inline void Map::set_current_room ( Room* room )
     {
          m_current_room = room;
     }

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

