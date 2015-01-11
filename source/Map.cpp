#include "Map.hpp"
#include "Utils.hpp"

#include <fstream>

using namespace bryte;

const Real32 Map::c_tile_dimension_in_meters = static_cast<Real32>( c_tile_dimension_in_pixels /
                                                                    pixels_per_meter );

void Map::Room::initialize ( Uint32 width, Uint32 height, Tile* tiles )
{
     LOG_INFO ( "Building Room: %d by %d\n", width, height );

     this->width  = width;
     this->height = height;
     this->tiles  = tiles;

     for ( Uint32 y = 0; y < height; ++y ) {
          for ( Uint32 x = 0; x < width; ++x ) {
               auto index = y * width + x;

               if ( x == 0 ) {
                    tiles [ index ].value = 7;
                    tiles [ index ].solid = true;
               } else if ( x == width - 1 ) {
                    tiles [ index ].value = 5;
                    tiles [ index ].solid = true;
               } else if ( y == 0 ) {
                    tiles [ index ].value = 6;
                    tiles [ index ].solid = true;
               } else if ( y == height - 1 ) {
                    tiles [ index ].value = 4;
                    tiles [ index ].solid = true;
               } else {
                    tiles [ index ].value = 0;
                    tiles [ index ].solid = 0;
               }
          }
     }

     Int32 bottom_left  = 0;
     Int32 bottom_right = width - 1;
     Int32 top_left     = width * ( height - 1 );
     Int32 top_right    = width * height - 1;

     tiles [ bottom_left ].value  = 11;
     tiles [ bottom_left ].solid  = true;
     tiles [ bottom_right ].value = 10;
     tiles [ bottom_right ].solid = true;
     tiles [ top_left ].value     = 8;
     tiles [ top_left ].solid     = true;
     tiles [ top_right ].value    = 9;
     tiles [ top_right ].solid    = true;
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
     return m_current_room->tiles [ coordinate_to_tile_index ( tile_x, tile_y ) ].value;
}

Bool Map::get_coordinate_solid ( Int32 tile_x, Int32 tile_y ) const
{
     return m_current_room->tiles [ coordinate_to_tile_index ( tile_x, tile_y ) ].solid;
}

Void Map::set_coordinate_value ( Int32 tile_x, Int32 tile_y, Uint8 value )
{
     m_current_room->tiles [ coordinate_to_tile_index ( tile_x, tile_y ) ].value = value;
}

Void Map::set_coordinate_solid ( Int32 tile_x, Int32 tile_y, Bool solid )
{
     m_current_room->tiles [ coordinate_to_tile_index ( tile_x, tile_y ) ].solid = solid;
}

Bool Map::is_position_solid ( Real32 x, Real32 y ) const
{
     return m_current_room->tiles [ position_to_tile_index ( x, y ) ].solid;
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

void Map::Room::save ( const Char8* filepath )
{
     std::ofstream file ( filepath, std::ios::binary );

     if ( !file.is_open ( ) ) {
          LOG_ERROR ( "Unable to save room: %s\n", filepath );
          return;
     }

     file.write ( reinterpret_cast<const Char8*>( &width ), sizeof ( width ) );
     file.write ( reinterpret_cast<const Char8*>( &height ), sizeof ( height ) );

     for ( Int32 y = 0; y < height; ++y ) {
          for ( Int32 x = 0; x < width; ++x ) {
               auto& tile = tiles [ y * width + x ];

               file.write ( reinterpret_cast<const Char8*> ( &tile.value ), sizeof ( tile.value ) );
               file.write ( reinterpret_cast<const Char8*> ( &tile.solid ), sizeof ( tile.solid ) );
          }
     }
}

void Map::Room::load ( const Char8* filepath )
{
     std::ifstream file ( filepath, std::ios::binary );

     if ( !file.is_open ( ) ) {
          LOG_ERROR ( "Unable to save room: %s\n", filepath );
          return;
     }

     Uint8 width = 0;
     Uint8 height = 0;

     file.read ( reinterpret_cast<Char8*>( &width ), sizeof ( width ) );
     file.read ( reinterpret_cast<Char8*>( &height ), sizeof ( height ) );

     for ( Int32 y = 0; y < height; ++y ) {
          for ( Int32 x = 0; x < width; ++x ) {
               auto& tile = tiles [ y * width + x ];

               file.read ( reinterpret_cast<Char8*> ( &tile.value ), sizeof ( tile.value ) );
               file.read ( reinterpret_cast<Char8*> ( &tile.solid ), sizeof ( tile.solid ) );
          }
     }
}

