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

Map::Exit* Map::check_coordinates_for_exit ( Int32 x, Int32 y )
{
     for ( Uint8 d = 0; d < m_exit_count; ++d ) {
          auto& exit = m_exits [ d ];

          if ( exit.location.x == x && exit.location.y == y ) {
               return &exit;
          }
     }

     return nullptr;
}

Map::Fixture* Map::check_coordinates_for_fixture ( Map::Fixture* fixture_array, Uint8 fixture_count,
                                                   Uint8 x, Uint8 y )
{
     for ( Uint8 d = 0; d < fixture_count; ++d ) {
          auto& fixture = fixture_array [ d ];

          if ( fixture.location.x == x && fixture.location.y == y ) {
               return &fixture;
          }
     }

     return nullptr;
}

Map::Fixture* Map::check_coordinates_for_decor ( Int32 x, Int32 y )
{
     return check_coordinates_for_fixture ( m_decors, m_decor_count, x, y );
}

Map::Fixture* Map::check_coordinates_for_lamp ( Int32 x, Int32 y )
{
     return check_coordinates_for_fixture ( m_lamps, m_lamp_count, x, y );
}

Map::Fixture* Map::check_coordinates_for_enemy_spawn ( Int32 x, Int32 y )
{
     return check_coordinates_for_fixture ( m_enemy_spawns, m_enemy_spawn_count, x, y );
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

Void Map::illuminate ( Int32 x, Int32 y, Uint8 value )
{
     Int32 tile_radius = ( static_cast<Int32>( value ) - static_cast<Int32>( m_base_light_value ) ) / c_light_decay;

     if ( tile_radius <= 0 ) {
          return;
     }

     Int32 tile_x      = x / c_tile_dimension_in_pixels;
     Int32 tile_y      = y / c_tile_dimension_in_pixels;
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

               if ( manhattan_distance > tile_radius ) {
                    continue;
               }

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
          illuminate ( lamp.location.x * c_tile_dimension_in_pixels,
                       lamp.location.y * c_tile_dimension_in_pixels,
                       c_unique_lamps_light [ lamp.id ] );
     }
}

Bool Map::add_fixture ( Map::Fixture* fixture_array, Uint8* fixture_count, Uint8 max_fixtures,
                        Int32 location_x, Int32 location_y, Uint8 id )
{
     if ( *fixture_count >= max_fixtures ) {
          return false;
     }

     fixture_array [ *fixture_count ].location.x = location_x;
     fixture_array [ *fixture_count ].location.y = location_y;
     fixture_array [ *fixture_count ].id         = id;

     (*fixture_count)++;

     return true;
}

Void Map::remove_fixture ( Fixture* fixture_array, Uint8* fixture_count, Uint8 max_fixtures,
                           Fixture* fixture )
{
     ASSERT ( fixture >= fixture_array && fixture < fixture_array + max_fixtures );

     Fixture* last = fixture_array + ( *fixture_count - 1 );

     // slide down all the elements after it
     while ( fixture <= last ) {
          Fixture* next = fixture + 1;
          *fixture = *next;
          fixture = next;
     }

     (*fixture_count)--;
}

Bool Map::add_decor ( Int32 location_x, Int32 location_y, Uint8 id )
{
     return add_fixture ( m_decors, &m_decor_count, c_max_decors, location_x, location_y, id );
}

Void Map::remove_decor ( Fixture* decor )
{
     remove_fixture ( m_decors, &m_decor_count, c_max_decors, decor );
}

Bool Map::add_lamp ( Int32 location_x, Int32 location_y, Uint8 id )
{
     return add_fixture ( m_lamps, &m_lamp_count, c_max_lamps, location_x, location_y, id );
}

Void Map::remove_lamp ( Fixture* lamp )
{
     remove_fixture ( m_lamps, &m_lamp_count, c_max_lamps, lamp );
}

Bool Map::add_enemy_spawn ( Int32 location_x, Int32 location_y, Uint8 id )
{
     return add_fixture ( m_enemy_spawns, &m_enemy_spawn_count, c_max_enemy_spawns, location_x, location_y, id );
}

Void Map::remove_enemy_spawn ( Fixture* enemy_spawn )
{
     remove_fixture ( m_enemy_spawns, &m_enemy_spawn_count, c_max_enemy_spawns, enemy_spawn );
}

Bool Map::add_exit ( Int32 location_x, Int32 location_y, Uint8 id )
{
     if ( m_exit_count >= c_max_exits ) {
          return false;
     }

     m_exits [ m_exit_count ].location.x = location_x;
     m_exits [ m_exit_count ].location.y = location_y;
     m_exits [ m_exit_count ].id = id;

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

     file.write ( reinterpret_cast<const Char8*>( &m_exit_count ), sizeof ( m_exit_count ) );

     for ( Int32 i = 0; i < m_exit_count; ++i ) {
          auto& exit = m_exits [ i ];
          file.write ( reinterpret_cast<const Char8*> ( &exit ), sizeof ( exit ) );
     }

     file.write ( reinterpret_cast<const Char8*>( &m_decor_count ), sizeof ( m_decor_count ) );

     for ( Int32 i = 0; i < m_decor_count; ++i ) {
          auto& decor = m_decors [ i ];
          file.write ( reinterpret_cast<const Char8*> ( &decor ), sizeof ( decor ) );
     }

     file.write ( reinterpret_cast<const Char8*>( &m_lamp_count ), sizeof ( m_lamp_count ) );

     for ( Int32 i = 0; i < m_lamp_count; ++i ) {
          auto& lamp = m_lamps [ i ];
          file.write ( reinterpret_cast<const Char8*> ( &lamp ), sizeof ( lamp ) );
     }

     file.write ( reinterpret_cast<const Char8*> ( &m_base_light_value ), sizeof ( m_base_light_value ) );

     file.write ( reinterpret_cast<const Char8*>( &m_enemy_spawn_count ), sizeof ( m_enemy_spawn_count ) );

     for ( Int32 i = 0; i < m_enemy_spawn_count; ++i ) {
          auto& enemy_spawn = m_enemy_spawns [ i ];
          file.write ( reinterpret_cast<const Char8*> ( &enemy_spawn ), sizeof ( enemy_spawn ) );
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

     file.read ( reinterpret_cast<Char8*>( &m_exit_count ), sizeof ( m_exit_count ) );

     for ( Int32 i = 0; i < m_exit_count; ++i ) {
          auto& exit = m_exits [ i ];
          file.read ( reinterpret_cast<Char8*> ( &exit ), sizeof ( exit ) );
     }

     file.read ( reinterpret_cast<Char8*>( &m_decor_count ), sizeof ( m_decor_count ) );

     for ( Int32 i = 0; i < m_decor_count; ++i ) {
          auto& decor = m_decors [ i ];
          file.read ( reinterpret_cast<Char8*> ( &decor ), sizeof ( decor ) );
     }

     file.read ( reinterpret_cast<Char8*>( &m_lamp_count ), sizeof ( m_lamp_count ) );

     for ( Int32 i = 0; i < m_lamp_count; ++i ) {
          auto& lamp = m_lamps [ i ];
          file.read ( reinterpret_cast<Char8*> ( &lamp ), sizeof ( lamp ) );
     }

     file.read ( reinterpret_cast<Char8*> ( &m_base_light_value ), sizeof ( m_base_light_value ) );

     reset_light ( );

     file.read ( reinterpret_cast<Char8*>( &m_enemy_spawn_count ), sizeof ( m_enemy_spawn_count ) );

     for ( Int32 i = 0; i < m_enemy_spawn_count; ++i ) {
          auto& enemy_spawn = m_enemy_spawns [ i ];
          file.read ( reinterpret_cast<Char8*> ( &enemy_spawn ), sizeof ( enemy_spawn ) );
     }
}

