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

     m_current_map = c_first_master_map;

     m_activate_on_kill_all.x = 0;
     m_activate_on_kill_all.y = 0;

     clear_persistence ( );

     return true;
}

Void Map::initialize ( Uint8 width, Uint8 height )
{
     m_width  = width;
     m_height = height;

     m_light_width = m_width * c_tile_dimension_in_pixels;
     m_light_height = m_height * c_tile_dimension_in_pixels;
     m_light_pixels = m_light_width * m_light_height;

     // clear all tiles
     for ( Uint32 y = 0; y < height; ++y ) {
          for ( Uint32 x = 0; x < width; ++x ) {
               Auto index = y * width + x;

               m_tiles [ index ].value = 1;
               m_tiles [ index ].flags = 0;
          }
     }

     // clear border exits
     for ( Int32 i = 0; i < Direction::count; ++i ) {
          Auto& border_exit = m_border_exits [ i ];

          border_exit.bottom_left = Location { 0, 0 };
          border_exit.map_index = 0;
          border_exit.map_bottom_left = Location { 0, 0 };
     }

     m_base_light_value = 128;
     reset_light ( );

     m_upgrade.location.x = 0;
     m_upgrade.location.y = 0;
     m_upgrade.id = 0;
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
     return m_tiles [ coordinate_to_tile_index ( tile_x, tile_y ) ].flags & TileFlags::solid;
}

Bool Map::get_coordinate_invisible ( Int32 tile_x, Int32 tile_y ) const
{
     return m_tiles [ coordinate_to_tile_index ( tile_x, tile_y ) ].flags & TileFlags::invisible;
}

Uint8 Map::get_coordinate_light ( Int32 tile_x, Int32 tile_y ) const
{
     ASSERT ( tile_x >= 0 && tile_x < m_width );
     ASSERT ( tile_y >= 0 && tile_y < m_height );

     Int32 pixel_x = tile_x * c_tile_dimension_in_pixels;
     Int32 pixel_y = tile_y * c_tile_dimension_in_pixels;

     return m_light [ pixel_y * m_light_width + pixel_x ];
}

Uint8 Map::get_pixel_light ( Int32 x, Int32 y ) const
{
     ASSERT ( x >= 0 && x < m_light_width );
     ASSERT ( y >= 0 && y < m_light_height );

     return m_light [ y * m_light_width + x ];
}

Void Map::set_coordinate_value ( Int32 tile_x, Int32 tile_y, Uint8 value )
{
     m_tiles [ coordinate_to_tile_index ( tile_x, tile_y ) ].value = value;
}

Void Map::set_coordinate_solid ( Int32 tile_x, Int32 tile_y, Bool solid )
{
     Auto& flags = m_tiles [ coordinate_to_tile_index ( tile_x, tile_y ) ].flags;

     if ( solid ) {
          flags |= TileFlags::solid;
     } else {
          flags &= ~TileFlags::solid;
     }
}

Void Map::set_coordinate_invisible ( Int32 tile_x, Int32 tile_y, Bool invisible )
{
     Auto& flags = m_tiles [ coordinate_to_tile_index ( tile_x, tile_y ) ].flags;

     if ( invisible ) {
          flags |= TileFlags::invisible;
     } else {
          flags &= ~TileFlags::invisible;
     }
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
     Real32 radius = ( static_cast<Real32>( value ) - static_cast<Real32>( m_base_light_value ) ) / 2;

     if ( radius <= 0.0f ) {
          return;
     }

     Int32 min_x  = x - static_cast<Int32>( radius );
     Int32 max_x  = x + static_cast<Int32>( radius );
     Int32 min_y  = y - static_cast<Int32>( radius );
     Int32 max_y  = y + static_cast<Int32>( radius );

     CLAMP ( min_x, 0, m_light_width - 1 );
     CLAMP ( max_x, 0, m_light_width - 1 );
     CLAMP ( min_y, 0, m_light_height - 1 );
     CLAMP ( max_y, 0, m_light_height - 1 );

     for ( Int32 j = min_y; j <= max_y; ++j ) {
          for ( Int32 i = min_x; i <= max_x; ++i ) {
               Real32 diff_x = static_cast<Real32>( x - i );
               Real32 diff_y = static_cast<Real32>( y - j );
               Real32 distance = sqrt ( diff_x * diff_x + diff_y * diff_y );

               if ( distance > radius ) {
                    continue;
               }

               Uint8& light_value     = m_light [ j * m_light_width + i ];
               Uint8  new_light_value = value - static_cast<Uint8>( distance * 2.0f );

               if ( light_value < new_light_value ) {
                    light_value = new_light_value;
               }
          }
     }
}

Void Map::reset_light ( )
{
     for ( Int32 i = 0; i < m_light_pixels; ++i ) {
          m_light [ i ] = m_base_light_value;
     }

     for ( Uint8 i = 0; i < m_lamp_count; ++i ) {
          Auto& lamp = m_lamps [ i ];
          illuminate ( lamp.location.x * c_tile_dimension_in_pixels + c_tile_dimension_in_pixels / 2,
                       lamp.location.y * c_tile_dimension_in_pixels + c_tile_dimension_in_pixels / 2,
                       c_lamp_light );
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

Void Map::set_border_exit ( Direction side, const BorderExit& border_exit )
{
     ASSERT ( side <= Direction::count );

     m_border_exits [ side ] = border_exit;
}

Map::BorderExit& Map::get_border_exit ( Direction side )
{
     ASSERT ( side <= Direction::count );

     return m_border_exits [ side ];
}

Bool Map::load_from_master_list ( Uint8 map_index, Interactives& interactives )
{
     if ( map_index >= m_master_count ) {
          LOG_ERROR ( "Failed to load map. Invalid map index %d\n", map_index );
          return false;
     }

     char filepath [ c_max_map_name_size ];

     sprintf ( filepath, "map/%s", m_master_list [ map_index ] );

     Bool success = load ( filepath, interactives );

     m_current_map = map_index;

     restore_exits ( interactives );
     restore_enemy_spawns ( );
     restore_activate_on_kill_all ( interactives );

     m_secret.found = false;
     m_killed_all_enemies = false;

     if ( m_persisted_secrets [ map_index ] ) {
          find_secret ( );
     }

     return success;
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

               file.write ( reinterpret_cast<const Char8*>( &tile ), sizeof ( tile ) );
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
          file.write ( reinterpret_cast<const Char8*>( &lamp ), sizeof ( lamp ) );
     }

     file.write ( reinterpret_cast<const Char8*>( &m_base_light_value ), sizeof ( m_base_light_value ) );

     file.write ( reinterpret_cast<const Char8*>( &m_enemy_spawn_count ), sizeof ( m_enemy_spawn_count ) );

     for ( Int32 i = 0; i < m_enemy_spawn_count; ++i ) {
          Auto& enemy_spawn = m_enemy_spawns [ i ];
          file.write ( reinterpret_cast<const Char8*>( &enemy_spawn ), sizeof ( enemy_spawn ) );
     }

     for ( Int32 y = 0; y < interactives.height ( ); ++y ) {
          for ( Int32 x = 0; x < interactives.width ( ); ++x ) {
               Auto& interactive = interactives.get_from_tile ( x, y );
               file.write ( reinterpret_cast<const Char8*>( &interactive ), sizeof ( interactive ) );

          }
     }

     for ( Int32 i = 0; i < Direction::count; ++i ) {
          file.write ( reinterpret_cast<const Char8*>( &m_border_exits [ i ] ), sizeof ( BorderExit ) );
     }

     file.write ( reinterpret_cast<const Char8*> ( &m_activate_on_kill_all ),
                  sizeof ( m_activate_on_kill_all ) );

     file.write ( reinterpret_cast<const Char8*> ( &m_secret.location ), sizeof ( m_secret.location ) );
     file.write ( reinterpret_cast<const Char8*> ( &m_secret.clear_tile ), sizeof ( m_secret.clear_tile ) );

     file.write ( reinterpret_cast<const Char8*> ( &m_upgrade ), sizeof ( m_upgrade ) );
}

Bool Map::load ( const Char8* filepath, Interactives& interactives )
{
     LOG_INFO ( "Loading Map '%s'\n", filepath );

     std::ifstream file ( filepath, std::ios::binary );

     if ( !file.is_open ( ) ) {
          LOG_ERROR ( "Unable to save room: %s\n", filepath );
          return false;
     }

     file.read ( reinterpret_cast<Char8*>( &m_width ), sizeof ( m_width ) );
     file.read ( reinterpret_cast<Char8*>( &m_height ), sizeof ( m_height ) );

     LOG_INFO ( "Dimensions: %d, %d\n", m_width, m_height );

     if ( m_width <= 0 || m_height <= 0 ||
          m_width * m_height > c_max_tiles ) {
          LOG_ERROR ( "Invalid map dimensions: %d, %d\n", m_width, m_height );
          return false;
     }

     m_light_width = m_width * c_tile_dimension_in_pixels;
     m_light_height = m_height * c_tile_dimension_in_pixels;
     m_light_pixels = m_light_width * m_light_height;

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

     for ( Int32 i = 0; i < Direction::count; ++i ) {
          file.read ( reinterpret_cast<Char8*>( &m_border_exits [ i ] ), sizeof ( BorderExit ) );
     }

     file.read ( reinterpret_cast<Char8*> ( &m_activate_on_kill_all ),
                 sizeof ( m_activate_on_kill_all ) );

     file.read ( reinterpret_cast<Char8*> ( &m_secret.location ), sizeof ( m_secret.location ) );
     file.read ( reinterpret_cast<Char8*> ( &m_secret.clear_tile ), sizeof ( m_secret.clear_tile ) );

     file.read ( reinterpret_cast<Char8*> ( &m_upgrade ), sizeof ( m_upgrade ) );

     return true;
}

Bool Map::save_persistence ( const Char8* region_name, Uint8 save_slot )
{
     char filepath [ 128 ];

     sprintf ( filepath, "save/slot_%d_%s.brp", save_slot, region_name );

     LOG_INFO ( "Saving region persistence: %s\n", filepath );

     std::ofstream file ( filepath, std::ios::binary );

     if ( !file.is_open ( ) ) {
          LOG_ERROR ( "Unable to open file to save persistence.\n" );
          return false;
     }

     file.write ( reinterpret_cast<const Char8*>( m_persisted_exits ), sizeof ( PersistedExit ) * c_max_exits );
     file.write ( reinterpret_cast<const Char8*>( &m_persisted_exit_count ), sizeof ( m_persisted_exit_count ) );

     file.write ( reinterpret_cast<const Char8*>( m_persisted_enemies ), sizeof ( PersistedEnemies ) * c_max_maps );
     file.write ( reinterpret_cast<const Char8*>( m_persisted_secrets ), sizeof ( Bool ) * c_max_maps );
     file.write ( reinterpret_cast<const Char8*>( m_persisted_activate_on_kill_all ),
                  sizeof ( Bool ) * c_max_maps );

     file.close ( );

     return true;
}

Bool Map::load_persistence ( const Char8* region_name, Uint8 save_slot )
{
     char filepath [ 128 ];

     sprintf ( filepath, "save/slot_%d_%s.brp", save_slot, region_name );

     LOG_INFO ( "Loading region persistence: %s\n", filepath );

     std::ifstream file ( filepath, std::ios::binary );

     if ( !file.is_open ( ) ) {
          // This is OK if the player has never been to this region.
          LOG_INFO ( "No region persistence found.\n" );
          return false;
     }

     file.read ( reinterpret_cast<Char8*>( m_persisted_exits ), sizeof ( PersistedExit ) * c_max_exits );
     file.read ( reinterpret_cast<Char8*>( &m_persisted_exit_count ), sizeof ( m_persisted_exit_count ) );

     file.read ( reinterpret_cast<Char8*>( m_persisted_enemies ), sizeof ( PersistedEnemies ) * c_max_maps );
     file.read ( reinterpret_cast<Char8*>( m_persisted_secrets ), sizeof ( Bool ) * c_max_maps );
     file.read ( reinterpret_cast<Char8*>( m_persisted_activate_on_kill_all ),
                 sizeof ( Bool ) * c_max_maps );

     file.close ( );

     return true;
}

void Map::clear_persistence ( )
{
     m_persisted_exit_count = 0;

     for ( Uint32 i = 0; i < c_max_exits; ++i ) {
          m_persisted_exits [ i ].map [ 0 ].index = 0;
          m_persisted_exits [ i ].map [ 1 ].index = 0;
          m_persisted_exits [ i ].map [ 0 ].location = Location { 0, 0 };
          m_persisted_exits [ i ].map [ 1 ].location = Location { 0, 0 };
          m_persisted_exits [ i ].state = 0;
     }

     for ( Uint32 i = 0; i < c_max_maps; ++i  ) {
          for ( Uint32 j = 0; j < c_max_enemy_spawns; ++j ) {
               Auto& persisted_enemy = m_persisted_enemies [ i ].enemies [ j ];

               persisted_enemy.alive = true;
               persisted_enemy.location.x = 0;
               persisted_enemy.location.y = 0;
          }

          m_persisted_secrets [ i ] = false;
          m_persisted_activate_on_kill_all [ i ] = false;
     }
}

Void Map::persist_exit ( const Interactive& exit, Uint8 x, Uint8 y )
{
     // try to find the exit
     for ( Uint32 i = 0; i < m_persisted_exit_count; ++i ) {
          for ( Uint32 m = 0; m < 2; ++m ) {
               PersistedExit::Map& map_info = m_persisted_exits [ i ].map [ m ];
               Location& map_location = map_info.location;

               if ( map_info.index == m_current_map &&
                    map_location.x == x && map_location.y == y ) {
                    m_persisted_exits [ i ].state = exit.interactive_exit.state;
                    return;
               }
          }
     }

     // if we didn't find the exit, create one
     if ( m_persisted_exit_count >= c_max_exits ) {
          LOG_INFO ( "Error persisting exit at %d, %d, on map %d, hit max persisted exits %d\n",
                      x, y, m_current_map, c_max_exits );
          return;
     }

     PersistedExit& new_exit = m_persisted_exits [ m_persisted_exit_count ];

     new_exit.state = exit.interactive_exit.state;
     new_exit.map [ 0 ].location = Location { x, y };
     new_exit.map [ 0 ].index = m_current_map;
     new_exit.map [ 1 ].location = Location { exit.interactive_exit.exit_index_x,
                                              exit.interactive_exit.exit_index_y };
     new_exit.map [ 1 ].index = exit.interactive_exit.map_index;

     m_persisted_exit_count++;
}

Void Map::persist_enemy ( const Enemy& enemy, Uint8 index )
{
     ASSERT ( index < Map::c_max_enemy_spawns );

     Auto& persisted_enemy = m_persisted_enemies [ m_current_map ].enemies [ index ];
     Auto center = enemy.collision_center ( );

     persisted_enemy.alive = enemy.is_alive ( );
     persisted_enemy.location.x = meters_to_pixels ( center.x ( ) ) / Map::c_tile_dimension_in_pixels;
     persisted_enemy.location.y = meters_to_pixels ( center.y ( ) ) / Map::c_tile_dimension_in_pixels;
}

Void Map::persist_secret ( )
{
     m_persisted_secrets [ m_current_map ] = m_secret.found;
}

Void Map::persist_killed_all_enemies ( )
{
     m_persisted_activate_on_kill_all [ m_current_map ] = m_killed_all_enemies;
}

Void Map::restore_exits ( Interactives& interactives )
{
     // try to find an exit that matches this map_index
     for ( Uint32 i = 0; i < m_persisted_exit_count; ++i ) {
          for ( Uint32 m = 0; m < 2; ++m ) {
               PersistedExit::Map& map_info = m_persisted_exits [ i ].map [ m ];

               if ( map_info.index == m_current_map ) {
                    Auto& exit = interactives.get_from_tile ( map_info.location.x,
                                                              map_info.location.y );

                    if ( exit.type != Interactive::Type::exit ) {
                         LOG_ERROR ( "Failed to restore persisted exit at %d, %d on map %d, map has changed?\n",
                                     map_info.location.x, map_info.location.y,
                                     m_current_map );
                         continue;
                    }

                    exit.interactive_exit.state =
                         static_cast<Exit::State>( m_persisted_exits [ i ].state );
               }
          }
     }
}

Void Map::restore_enemy_spawns ( )
{
     Uint8 delete_enemy_spawns [ c_max_enemy_spawns ];

     Auto& persisted_enemies = m_persisted_enemies [ m_current_map ].enemies;

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

Void Map::restore_activate_on_kill_all ( Interactives& interactives )
{
     m_killed_all_enemies = m_persisted_activate_on_kill_all [ m_current_map ];

     if ( m_killed_all_enemies ) {
          Auto& interactive = interactives.get_from_tile (  m_activate_on_kill_all.x,
                                                            m_activate_on_kill_all.y );
          // NOTE: persist non-exit changes
          if ( interactive.type != Interactive::Type::exit ) {
               interactives.activate ( m_activate_on_kill_all.x, m_activate_on_kill_all.y );
          }
     }
}

Void Map::find_secret ( )
{
     if ( m_secret.found ) {
          return;
     }

     m_secret.found = true;

     set_coordinate_value ( m_secret.clear_tile.x, m_secret.clear_tile.y, 1 );
     set_coordinate_solid ( m_secret.clear_tile.x, m_secret.clear_tile.y, false );
}

