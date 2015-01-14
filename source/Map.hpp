#ifndef BRYTE_MAP_HPP
#define BRYTE_MAP_HPP

#include "Types.hpp"
#include "GameMemory.hpp"
#include "Utils.hpp"

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

               Uint8 map_index;
               Uint8 exit_index;
          };

     public:

          Map ( );

          Void initialize ( Uint8 width, Uint8 height );

          Void save ( const Char8* filepath );
          Void load ( const Char8* filepath );

          Int32 position_to_tile_index     ( Real32 x, Real32 y ) const;

          Int32 coordinate_to_tile_index   ( Int32 tile_x, Int32 tile_y ) const;
          Int32 tile_index_to_coordinate_x ( Int32 tile_index ) const;
          Int32 tile_index_to_coordinate_y ( Int32 tile_index ) const;

          Uint8 get_coordinate_value ( Int32 tile_x, Int32 tile_y ) const;
          Bool  get_coordinate_solid ( Int32 tile_x, Int32 tile_y ) const;
          Void  set_coordinate_value ( Int32 tile_x, Int32 tile_y, Uint8 value );
          Void  set_coordinate_solid ( Int32 tile_x, Int32 tile_y, Bool solid );

          Bool  is_position_solid   ( Real32 x, Real32 y ) const;
          Exit* check_position_exit ( Uint8 x, Uint8 y );

          Bool add_exit ( Uint8 location_x, Uint8 location_y );
          Void remove_exit ( Exit* exit );

          inline Int32 width ( ) const;
          inline Int32 height ( ) const;

          inline Uint8 exit_count ( ) const;
          inline Exit& exit ( Uint8 index );

     public:

          static const Int32  c_tile_dimension_in_pixels = 16;
          static const Real32 c_tile_dimension_in_meters;

          static const Uint32 c_max_tiles = 1024;
          static const Uint32 c_max_exits = 4;

     private:

          Tile m_tiles [ c_max_tiles ];

          Uint8 m_width;
          Uint8 m_height;

          Exit   m_exits [ c_max_exits ];
          Uint8  m_exit_count;
     };

     inline Int32 Map::width ( ) const
     {
          return m_width;
     }

     inline Int32 Map::height ( ) const
     {
          return m_height;
     }

     inline Uint8 Map::exit_count ( ) const
     {
          return m_exit_count;
     }

     inline Map::Exit& Map::exit ( Uint8 index )
     {
          ASSERT ( index < m_exit_count );

          return m_exits [ index ];
     }
}

#endif

