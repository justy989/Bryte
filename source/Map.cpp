#include "Map.hpp"
#include "Utils.hpp"
#include "Interactives.hpp"
#include "Enemy.hpp"

#include <fstream>

using namespace bryte;

Void Map::Fixture::set ( Uint8 x, Uint8 y, Uint8 id )
{
     location.x = x;
     location.y = y;
     this->id = id;
}

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

     m_current_master_map = c_first_master_map;

     m_activate_on_all_enemies_killed.x = 0;
     m_activate_on_all_enemies_killed.y = 0;

     clear_persistence ( );

     return true;
}

Void Map::initialize ( Uint8 width, Uint8 height )
{
     m_width  = width;
     m_height = height;

     // clear all tiles
     for ( Uint32 y = 0; y < height; ++y ) {
          for ( Uint32 x = 0; x < width; ++x ) {
               Auto index = y * width + x;

               m_tiles [ index ].value = 1;
               m_tiles [ index ].solid = false;
          }
     }

     m_base_light_value = 128;
     reset_light ( );
}

Map::Coordinates Map::position_to_coordinates ( Real32 x, Real32 y )
{
     return Coordinates { meters_to_pixels ( x ) / Map::c_tile_dimension_in_pixels,
                          meters_to_pixels ( y ) / Map::c_tile_dimension_in_pixels };
}

Map::Coordinates Map::vector_to_coordinates ( const Vector& v )
{
     return Coordinates { meters_to_pixels ( v.x ( ) ) / Map::c_tile_dimension_in_pixels,
                          meters_to_pixels ( v.y ( ) ) / Map::c_tile_dimension_in_pixels};
}

Vector Map::coordinates_to_vector ( Int32 tile_x, Int32 tile_y )
{
     return Vector { pixels_to_meters ( tile_x * Map::c_tile_dimension_in_pixels ),
                     pixels_to_meters ( tile_y * Map::c_tile_dimension_in_pixels ) };
}

Vector Map::coordinates_to_vector ( const Coordinates& coords )
{
     return Vector { pixels_to_meters ( coords.x * Map::c_tile_dimension_in_pixels ),
                     pixels_to_meters ( coords.y * Map::c_tile_dimension_in_pixels ) };
}

Vector Map::location_to_vector ( const Location& loc )
{
     return Vector { pixels_to_meters ( loc.x * Map::c_tile_dimension_in_pixels ),
                     pixels_to_meters ( loc.y * Map::c_tile_dimension_in_pixels ) };
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

Map::Fixture* Map::check_coordinates_for_decor ( Int32 x, Int32 y )
{
     return check_coordinates_for_fixture<Fixture> ( m_decors, m_decor_count, x, y );
}

Map::Fixture* Map::check_coordinates_for_lamp ( Int32 x, Int32 y )
{
     return check_coordinates_for_fixture<Fixture> ( m_lamps, m_lamp_count, x, y );
}

Map::EnemySpawn* Map::check_coordinates_for_enemy_spawn ( Int32 x, Int32 y )
{
     return check_coordinates_for_fixture<EnemySpawn> ( m_enemy_spawns, m_enemy_spawn_count, x, y );
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
     Int32 tile_radius = ( static_cast<Int32>( value ) - static_cast<Int32>( m_base_light_value ) ) /
                           c_light_decay;

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
          Auto& lamp = m_lamps [ i ];
          illuminate ( lamp.location.x * c_tile_dimension_in_pixels,
                       lamp.location.y * c_tile_dimension_in_pixels,
                       c_unique_lamps_light [ lamp.id ] );
     }
}

Bool Map::add_decor ( Int32 location_x, Int32 location_y, Uint8 id )
{
     if ( !add_element<Fixture> ( m_decors, &m_decor_count, c_max_decors ) ) {
          return false;
     }

     m_decors [ m_decor_count - 1 ].set ( location_x, location_y, id );

     return true;
}

Void Map::remove_decor ( Fixture* decor )
{
     remove_element<Fixture> ( m_decors, &m_decor_count, c_max_decors, decor );
}

Bool Map::add_lamp ( Int32 location_x, Int32 location_y, Uint8 id )
{
     if ( !add_element<Fixture> ( m_lamps, &m_lamp_count, c_max_lamps ) ) {
          return false;
     }

     m_lamps [ m_lamp_count - 1 ].set ( location_x, location_y, id );

     return true;
}

Void Map::remove_lamp ( Fixture* lamp )
{
     remove_element<Fixture> ( m_lamps, &m_lamp_count, c_max_lamps, lamp );
}

Bool Map::add_enemy_spawn ( Int32 location_x, Int32 location_y, Uint8 id,
                            Direction facing, Pickup::Type drop )
{
     if ( !add_element<EnemySpawn> ( m_enemy_spawns, &m_enemy_spawn_count, c_max_enemy_spawns ) ) {
          return false;
     }

     Auto& enemy_spawn = m_enemy_spawns [ m_enemy_spawn_count - 1 ];

     enemy_spawn.set ( location_x, location_y, id );
     enemy_spawn.facing = facing;
     enemy_spawn.drop   = drop;

     return true;
}

Void Map::remove_enemy_spawn ( EnemySpawn* enemy_spawn )
{
     remove_element<EnemySpawn> ( m_enemy_spawns, &m_enemy_spawn_count, c_max_enemy_spawns, enemy_spawn );
}

Void Map::load_from_master_list ( Uint8 map_index, Interactives& interactives )
{
     if ( map_index >= m_master_count ) {
          LOG_ERROR ( "Failed to load map. Invalid map index %d\n", map_index );
          return;
     }

#if 0
     if ( m_current_master_map != c_first_master_map ) {
          persist_exits ( interactives );
     }
#endif

     load ( m_master_list [ map_index ], interactives );

     m_current_master_map = map_index;

     restore_exits ( interactives );
     restore_enemy_spawns ( );
}

Void Map::save ( const Char8* filepath, Interactives& interactives )
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
               Auto& tile = m_tiles [ y * m_width + x ];

               file.write ( reinterpret_cast<const Char8*> ( &tile ), sizeof ( tile ) );
          }
     }

     file.write ( reinterpret_cast<const Char8*>( &m_decor_count ), sizeof ( m_decor_count ) );

     for ( Int32 i = 0; i < m_decor_count; ++i ) {
          Auto& decor = m_decors [ i ];
          file.write ( reinterpret_cast<const Char8*> ( &decor ), sizeof ( decor ) );
     }

     file.write ( reinterpret_cast<const Char8*>( &m_lamp_count ), sizeof ( m_lamp_count ) );

     for ( Int32 i = 0; i < m_lamp_count; ++i ) {
          Auto& lamp = m_lamps [ i ];
          file.write ( reinterpret_cast<const Char8*> ( &lamp ), sizeof ( lamp ) );
     }

     file.write ( reinterpret_cast<const Char8*> ( &m_base_light_value ), sizeof ( m_base_light_value ) );

     file.write ( reinterpret_cast<const Char8*>( &m_enemy_spawn_count ), sizeof ( m_enemy_spawn_count ) );

     for ( Int32 i = 0; i < m_enemy_spawn_count; ++i ) {
          Auto& enemy_spawn = m_enemy_spawns [ i ];
          file.write ( reinterpret_cast<const Char8*> ( &enemy_spawn ), sizeof ( enemy_spawn ) );
     }

     for ( Int32 y = 0; y < interactives.height ( ); ++y ) {
          for ( Int32 x = 0; x < interactives.width ( ); ++x ) {
               Auto& interactive = interactives.get_from_tile ( x, y );
               file.write ( reinterpret_cast<const Char8*> ( &interactive ), sizeof ( interactive ) );
          }
     }

     file.write ( reinterpret_cast<const Char8*> ( &m_activate_on_all_enemies_killed ),
                  sizeof ( m_activate_on_all_enemies_killed ) );
}

Void Map::load ( const Char8* filepath, Interactives& interactives )
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
               Auto& tile = m_tiles [ y * m_width + x ];

               file.read ( reinterpret_cast<Char8*> ( &tile ), sizeof ( tile ) );
          }
     }

     file.read ( reinterpret_cast<Char8*>( &m_decor_count ), sizeof ( m_decor_count ) );

     for ( Int32 i = 0; i < m_decor_count; ++i ) {
          Auto& decor = m_decors [ i ];
          file.read ( reinterpret_cast<Char8*> ( &decor ), sizeof ( decor ) );
     }

     file.read ( reinterpret_cast<Char8*>( &m_lamp_count ), sizeof ( m_lamp_count ) );

     for ( Int32 i = 0; i < m_lamp_count; ++i ) {
          Auto& lamp = m_lamps [ i ];
          file.read ( reinterpret_cast<Char8*> ( &lamp ), sizeof ( lamp ) );
     }

     file.read ( reinterpret_cast<Char8*> ( &m_base_light_value ), sizeof ( m_base_light_value ) );

     reset_light ( );

     file.read ( reinterpret_cast<Char8*>( &m_enemy_spawn_count ), sizeof ( m_enemy_spawn_count ) );

     for ( Int32 i = 0; i < m_enemy_spawn_count; ++i ) {
          Auto& enemy_spawn = m_enemy_spawns [ i ];
          file.read ( reinterpret_cast<Char8*> ( &enemy_spawn ), sizeof ( enemy_spawn ) );
     }

     interactives.reset ( m_width, m_height );

     for ( Int32 y = 0; y < interactives.height ( ); ++y ) {
          for ( Int32 x = 0; x < interactives.width ( ); ++x ) {
               Auto& interactive = interactives.get_from_tile ( x, y );
               file.read ( reinterpret_cast<Char8*> ( &interactive ), sizeof ( interactive ) );
          }
     }

     file.read ( reinterpret_cast<Char8*> ( &m_activate_on_all_enemies_killed ),
                 sizeof ( m_activate_on_all_enemies_killed ) );
}

void Map::clear_persistence ( )
{
     for ( Uint32 i = 0; i < c_max_maps; ++i  ) {
          m_persisted_exits [ i ].exit_count = 0;

          for ( Uint32 j = 0; j < c_max_enemy_spawns; ++j ) {
               Auto& persisted_enemy = m_persisted_enemies [ i ].enemies [ j ];

               persisted_enemy.alive = true;
               persisted_enemy.location.x = 0;
               persisted_enemy.location.y = 0;
          }
     }
}

Void Map::persist_exit ( const Interactive& exit, Uint8 x, Uint8 y )
{
     ASSERT ( exit.type == Interactive::Type::exit );

     Auto& map_persisted_exits = m_persisted_exits [ m_current_master_map ];

     if ( map_persisted_exits.exit_count >= PersistedExits::c_max_persisted_exits ) {
          LOG_ERROR ( "Too many exits on map to persist: %u\n", map_persisted_exits.exit_count );
     }

     Auto& persisted_exit = map_persisted_exits.exits [ map_persisted_exits.exit_count ];

     persisted_exit.location.x = x;
     persisted_exit.location.y = y;
     persisted_exit.id         = exit.interactive_exit.state;

     LOG_DEBUG ( "Persisted exit %d %d to state %d\n", x, y, exit.interactive_exit.state );

     map_persisted_exits.exit_count++;
}

Void Map::persist_enemy ( const Enemy& enemy, Uint8 index )
{
     ASSERT ( index < Map::c_max_enemy_spawns );

     Auto& persisted_enemy = m_persisted_enemies [ m_current_master_map ].enemies [ index ];
     Auto center = enemy.collision_center ( );

     persisted_enemy.alive = enemy.is_alive ( );
     persisted_enemy.location.x = meters_to_pixels ( center.x ( ) ) / Map::c_tile_dimension_in_pixels;
     persisted_enemy.location.y = meters_to_pixels ( center.y ( ) ) / Map::c_tile_dimension_in_pixels;
}

Void Map::restore_exits ( Interactives& interactives )
{
     Auto& map_persisted_exits = m_persisted_exits [ m_current_master_map ];

     for ( Uint8 i = 0; i < map_persisted_exits.exit_count; ++i ) {
          Auto& persisted_exit = map_persisted_exits.exits [ i ];
          Auto& interactive = interactives.get_from_tile ( persisted_exit.location.x,
                                                           persisted_exit.location.y );

          if ( interactive.type != Interactive::Type::exit ) {
               LOG_ERROR ( "Unable to persist exit at %d %d, map has changed.\n",
                           persisted_exit.location.x, persisted_exit.location.y );
               continue;
          }

          LOG_DEBUG ( "Restoring exit %d %d to state %d\n",
                      persisted_exit.location.x, persisted_exit.location.y,
                      persisted_exit.id );

          Auto& exit = interactive.interactive_exit;
          exit.state = static_cast<Exit::State> ( persisted_exit.id );
     }

     map_persisted_exits.exit_count = 0;
}

Void Map::restore_enemy_spawns ( )
{
     Uint8 delete_enemy_spawns [ c_max_enemy_spawns ];

     Auto& persisted_enemies = m_persisted_enemies [ m_current_master_map ].enemies;

     // figure out which enemies should not be persisted
     for ( Uint8 i = 0; i < m_enemy_spawn_count; ++i ) {
          Auto& persisted_enemy = persisted_enemies [ i ];

          if ( persisted_enemy.alive ) {
               // persist non-zero locations
               if ( persisted_enemy.location.x && persisted_enemy.location.y ) {
                    m_enemy_spawns [ i ].location = persisted_enemy.location;
               }

               delete_enemy_spawns [ i ] = false;
          } else {
               delete_enemy_spawns [ i ] = true;
          }
     }

     Uint8 new_enemy_spawn_count = 0;
     Uint8 last_enemy = m_enemy_spawn_count - 1;

     // remove enemies that no longer need to be persisted
     for ( Uint8 i = 0; i < m_enemy_spawn_count; ++i ) {
          if ( delete_enemy_spawns [ i ] ) {
               for ( Uint8 j = i; j < last_enemy; ++j ) {
                    m_enemy_spawns [ j ] = m_enemy_spawns [ j + 1 ];
               }
          } else {
               new_enemy_spawn_count++;
          }
     }

     m_enemy_spawn_count = new_enemy_spawn_count;
}

