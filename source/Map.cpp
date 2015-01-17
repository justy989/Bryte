#include "Map.hpp"
#include "Utils.hpp"

#include <fstream>

using namespace bryte;

const Real32 Map::c_tile_dimension_in_meters = static_cast<Real32>( c_tile_dimension_in_pixels /
                                                                    pixels_per_meter );

const Uint8 Map::c_unique_lamps_light [ Map::c_unique_lamp_count ] = {
     255, 255 - ( 2 * c_light_decay ), 255 - c_light_decay, 0
};

Bool Map::load_master_list ( const Char8* filepath )
{
     std::ifstream file ( filepath );

     if ( !file.is_open ( ) ) {
          LOG_ERROR ( "Failed to load master map list '%s'\n", filepath );
          return false;
     }

     m_master_count = 0;

     while ( !file.eof ( ) ) {
          ASSERT ( m_master_count < c_max_maps );

          file.getline ( m_master_list [ m_master_count ], c_max_map_name_size );

          m_master_count++;
     }

     return true;
}

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

     m_base_light_value = 128;
     reset_light ( );
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

Uint8 Map::get_coordinate_decor ( Int32 tile_x, Int32 tile_y ) const
{
     return m_tiles [ coordinate_to_tile_index ( tile_x, tile_y ) ].decor;
}

Uint8 Map::get_coordinate_light ( Int32 tile_x, Int32 tile_y ) const
{
     return m_light [ coordinate_to_tile_index ( tile_x, tile_y ) ];
}

Void Map::set_coordinate_value ( Int32 tile_x, Int32 tile_y, Uint8 value )
{
     m_tiles [ coordinate_to_tile_index ( tile_x, tile_y ) ].value = value;
}

Void Map::set_coordinate_solid ( Int32 tile_x, Int32 tile_y, Bool solid )
{
     m_tiles [ coordinate_to_tile_index ( tile_x, tile_y ) ].solid = solid;
}

Void Map::set_coordinate_decor ( Int32 tile_x, Int32 tile_y, Uint8 decor )
{
     m_tiles [ coordinate_to_tile_index ( tile_x, tile_y ) ].decor = decor;
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

Map::Lamp* Map::check_position_lamp ( Uint8 x, Uint8 y )
{
     for ( Uint8 d = 0; d < m_lamp_count; ++d ) {
          auto& lamp = m_lamps [ d ];

          if ( lamp.location_x == x && lamp.location_y == y ) {
               return &lamp;
          }
     }

     return nullptr;
}

Uint8 Map::base_light_value ( ) const
{
     return m_base_light_value;
}

Void Map::add_to_base_light ( Uint8 delta )
{
     m_base_light_value += delta;
}

Void Map::subtract_from_base_light ( Uint8 delta )
{
     m_base_light_value -= delta;
}

Void Map::illuminate ( Real32 x, Real32 y, Uint8 value )
{
     Int32 tile_radius = ( static_cast<Int32>( value ) - static_cast<Int32>( m_base_light_value ) ) / c_light_decay;

     if ( tile_radius < 0 ) {
          return;
     }

     Int32 tile_x      = meters_to_pixels ( x ) / c_tile_dimension_in_pixels;
     Int32 tile_y      = meters_to_pixels ( y ) / c_tile_dimension_in_pixels;
     Int32 min_tile_x  = tile_x - tile_radius;
     Int32 max_tile_x  = tile_x + tile_radius;
     Int32 min_tile_y  = tile_y - tile_radius;
     Int32 max_tile_y  = tile_y + tile_radius;

     CLAMP ( min_tile_x, 0, m_width - 1 );
     CLAMP ( max_tile_x, 0, m_width - 1 );
     CLAMP ( min_tile_y, 0, m_height - 1 );
     CLAMP ( max_tile_y, 0, m_height - 1 );

     for ( Int32 y = min_tile_y; y <= max_tile_y; ++y ) {
          for ( Int32 x = min_tile_x; x <= max_tile_x; ++x ) {
               Int32 manhattan_distance = abs ( tile_x - x ) + abs ( tile_y - y );

               Uint8& light_value     = m_light [ coordinate_to_tile_index ( x, y ) ];
               Uint8  new_light_value = value - ( manhattan_distance * c_light_decay );

               if ( light_value < new_light_value ) {
                    light_value = new_light_value;
               }
          }
     }
}

Void Map::reset_light ( )
{
     for ( Int32 y = 0; y < m_height; ++y ) {
          for ( Int32 x = 0; x < m_width; ++x ) {
               m_light [ coordinate_to_tile_index ( x, y ) ] = m_base_light_value;
          }
     }

     for ( Uint8 i = 0; i < m_lamp_count; ++i ) {
          auto& lamp = m_lamps [ i ];
          illuminate ( pixels_to_meters ( lamp.location_x ) * c_tile_dimension_in_pixels,
                       pixels_to_meters ( lamp.location_y ) * c_tile_dimension_in_pixels,
                       c_unique_lamps_light [ lamp.id ] );
     }
}

Bool Map::add_lamp ( Uint8 location_x, Uint8 location_y, Uint8 id )
{
     if ( m_lamp_count >= c_max_lamps ) {
          return false;
     }

     m_lamps [ m_lamp_count ].location_x = location_x;
     m_lamps [ m_lamp_count ].location_y = location_y;
     m_lamps [ m_lamp_count ].id         = id;

     m_lamp_count++;

     return true;
}

Void Map::remove_lamp ( Lamp* lamp )
{
     ASSERT ( lamp >= m_lamps && lamp < m_lamps + c_max_lamps );

     Lamp* last = m_lamps + ( m_lamp_count - 1 );

     // slide down all the elements after it
     while ( lamp <= last ) {
          Lamp* next = lamp + 1;
          *lamp = *next;
          lamp = next;
     }

     m_lamp_count--;
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

Void Map::remove_exit ( Exit* exit )
{
     ASSERT ( exit >= m_exits && exit < m_exits + c_max_exits );

     Exit* last = m_exits + ( m_exit_count - 1 );

     // slide down all the elements after it
     while ( exit <= last ) {
          Exit* next = exit + 1;
          *exit = *next;
          exit = next;
     }

     m_exit_count--;
}

Void Map::load_from_master_list ( Uint8 map_index )
{
     if ( map_index >= m_master_count ) {
          LOG_ERROR ( "Failed to load map. Invalid map index %d\n", map_index );
          return;
     }

     load ( m_master_list [ map_index ] );
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

     file.write ( reinterpret_cast<const Char8*> ( &m_base_light_value ), sizeof ( m_base_light_value ) );
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

     file.read ( reinterpret_cast<Char8*> ( &m_base_light_value ), sizeof ( m_base_light_value ) );

     reset_light ( );
}

