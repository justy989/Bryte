#include "Map.hpp"
#include "Utils.hpp"

using namespace bryte;

const Int32  Map::c_tile_dimension_in_pixels = 16;
const Real32 Map::c_tile_dimension_in_meters = static_cast<Real32>( c_tile_dimension_in_pixels /
                                                                    pixels_per_meter );

void Map::Room::initialize ( Uint32 width, Uint32 height, Uint8* tiles )
{
     LOG_INFO ( "Building Room: %d by %d\n", width, height );

     this->width  = width;
     this->height = height;
     this->tiles  = tiles;

     for ( Uint32 y = 0; y < height; ++y ) {
          for ( Uint32 x = 0; x < width; ++x ) {
               auto index = y * width + x;

               if ( x == 0 ) {
                    tiles [ index ] = 7;
               } else if ( x == width - 1 ) {
                    tiles [ index ] = 5;
               } else if ( y == 0 ) {
                    tiles [ index ] = 6;
               } else if ( y == height - 1 ) {
                    tiles [ index ] = 4;
               } else {
                    tiles [ index ] = 0;
               }
          }
     }

     tiles [ 0 ] = 11;
     tiles [ width - 1 ] = 10;
     tiles [ width * ( height - 1 ) ] = 8;
     tiles [ width * height - 1 ] = 9;
}

Map::Map ( ) :
     m_current_room ( nullptr )
{

}

Int32 Map::position_to_tile_index ( Real32 x, Real32 y ) const
{
     Int32 tile_x = x / c_tile_dimension_in_meters;
     Int32 tile_y = y / c_tile_dimension_in_meters;

     return coordinate_to_tile_index ( tile_x, tile_y );
}

Int32 Map::coordinate_to_tile_index ( Int32 tile_x, Int32 tile_y ) const
{
     ASSERT ( tile_x >= 0 && tile_x < m_current_room->width );
     ASSERT ( tile_y >= 0 && tile_y < m_current_room->height );

     return tile_y * static_cast<Int32>( m_current_room->width ) + tile_x;
}

Int32 Map::tile_index_to_coordinate_x ( Int32 tile_index ) const
{
     return tile_index % static_cast<Int32>( m_current_room->width );
}

Int32 Map::tile_index_to_coordinate_y ( Int32 tile_index ) const
{
     return tile_index / static_cast<Int32>( m_current_room->width );
}

Uint8 Map::get_coordinate_value ( Int32 tile_x, Int32 tile_y ) const
{
     return m_current_room->tiles [ coordinate_to_tile_index ( tile_x, tile_y ) ];
}

Void Map::set_coordinate_value ( Int32 tile_x, Int32 tile_y, Uint8 value )
{
     m_current_room->tiles [ coordinate_to_tile_index ( tile_x, tile_y ) ] = value;
}

Bool Map::is_position_solid ( Real32 x, Real32 y ) const
{
     return m_current_room->tiles [ position_to_tile_index ( x, y ) ] > 0;
}

const Map::Exit* Map::check_position_exit ( Real32 x, Real32 y ) const
{
     Int32 player_tile_x = x / c_tile_dimension_in_meters;
     Int32 player_tile_y = y / c_tile_dimension_in_meters;

     for ( Uint8 d = 0; d < m_current_room->exit_count; ++d ) {
          auto& exit = m_current_room->exits [ d ];

          if ( exit.location_x == player_tile_x && exit.location_y == player_tile_y ) {
               return &exit;
          }
     }

     return nullptr;
}

