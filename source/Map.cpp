#include "Map.hpp"
#include "Utils.hpp"

#include <fstream>

using namespace bryte;

const Real32 Map::c_tile_dimension_in_meters = static_cast<Real32>( c_tile_dimension_in_pixels /
                                                                    pixels_per_meter );

void Map::initialize ( Uint8 width, Uint8 height )
{
     m_width  = width;
     m_height = height;

     // clear all tiles
     for ( Uint32 y = 0; y < height; ++y ) {
          for ( Uint32 x = 0; x < width; ++x ) {
               auto index = y * width + x;

               m_tiles [ index ].value = 0;
               m_tiles [ index ].solid = false;
          }
     }
}

Int32 Map::position_to_tile_index ( Real32 x, Real32 y ) const
{
     Int32 tile_x = x / c_tile_dimension_in_meters;
     Int32 tile_y = y / c_tile_dimension_in_meters;

     return coordinate_to_tile_index ( tile_x, tile_y );
}

Int32 Map::coordinate_to_tile_index ( Int32 tile_x, Int32 tile_y ) const
{
     ASSERT ( tile_x >= 0 && tile_x < m_width );
     ASSERT ( tile_y >= 0 && tile_y < m_height );

     return tile_y * static_cast<Int32>( m_width ) + tile_x;
}

Int32 Map::tile_index_to_coordinate_x ( Int32 tile_index ) const
{
     return tile_index % static_cast<Int32>( m_width );
}

Int32 Map::tile_index_to_coordinate_y ( Int32 tile_index ) const
{
     return tile_index / static_cast<Int32>( m_width );
}

Uint8 Map::get_coordinate_value ( Int32 tile_x, Int32 tile_y ) const
{
     return m_tiles [ coordinate_to_tile_index ( tile_x, tile_y ) ].value;
}

Bool Map::get_coordinate_solid ( Int32 tile_x, Int32 tile_y ) const
{
     return m_tiles [ coordinate_to_tile_index ( tile_x, tile_y ) ].solid;
}

Void Map::set_coordinate_value ( Int32 tile_x, Int32 tile_y, Uint8 value )
{
     m_tiles [ coordinate_to_tile_index ( tile_x, tile_y ) ].value = value;
}

Void Map::set_coordinate_solid ( Int32 tile_x, Int32 tile_y, Bool solid )
{
     m_tiles [ coordinate_to_tile_index ( tile_x, tile_y ) ].solid = solid;
}

Bool Map::is_position_solid ( Real32 x, Real32 y ) const
{
     return m_tiles [ position_to_tile_index ( x, y ) ].solid;
}

Map::Exit* Map::check_position_exit ( Uint8 x, Uint8 y )
{
     for ( Uint8 d = 0; d < m_exit_count; ++d ) {
          auto& exit = m_exits [ d ];

          if ( exit.location_x == x && exit.location_y == y ) {
               return &exit;
          }
     }

     return nullptr;
}

Bool Map::add_exit ( Uint8 location_x, Uint8 location_y )
{
     if ( m_exit_count >= c_max_exits ) {
          return false;
     }

     m_exits [ m_exit_count ].location_x = location_x;
     m_exits [ m_exit_count ].location_y = location_y;

     m_exit_count++;

     return true;
}

void Map::save ( const Char8* filepath )
{
     LOG_INFO ( "Saving Map '%s'\n", filepath );

     std::ofstream file ( filepath, std::ios::binary );

     if ( !file.is_open ( ) ) {
          LOG_ERROR ( "Unable to save room: %s\n", filepath );
          return;
     }

     file.write ( reinterpret_cast<const Char8*>( &m_width ), sizeof ( m_width ) );
     file.write ( reinterpret_cast<const Char8*>( &m_height ), sizeof ( m_height ) );

     for ( Int32 y = 0; y < m_height; ++y ) {
          for ( Int32 x = 0; x < m_width; ++x ) {
               auto& tile = m_tiles [ y * m_width + x ];

               file.write ( reinterpret_cast<const Char8*> ( &tile ), sizeof ( tile ) );
          }
     }

     file.write ( reinterpret_cast<const Char8*>( &m_exit_count ), sizeof ( Exit ) );

     for ( Int32 i = 0; i < m_exit_count; ++i ) {
          auto& exit = m_exits [ i ];
          file.write ( reinterpret_cast<const Char8*> ( &exit ), sizeof ( exit ) );
     }
}

void Map::load ( const Char8* filepath )
{
     LOG_INFO ( "Loading Map '%s'\n", filepath );

     std::ifstream file ( filepath, std::ios::binary );

     if ( !file.is_open ( ) ) {
          LOG_ERROR ( "Unable to save room: %s\n", filepath );
          return;
     }

     file.read ( reinterpret_cast<Char8*>( &m_width ), sizeof ( m_width ) );
     file.read ( reinterpret_cast<Char8*>( &m_height ), sizeof ( m_height ) );

     for ( Int32 y = 0; y < m_height; ++y ) {
          for ( Int32 x = 0; x < m_width; ++x ) {
               auto& tile = m_tiles [ y * m_width + x ];

               file.read ( reinterpret_cast<Char8*> ( &tile ), sizeof ( tile ) );
          }
     }

     file.read ( reinterpret_cast<Char8*>( &m_exit_count ), sizeof ( Exit ) );

     for ( Int32 i = 0; i < m_exit_count; ++i ) {
          auto& exit = m_exits [ i ];
          file.read ( reinterpret_cast<Char8*> ( &exit ), sizeof ( exit ) );
     }

}

