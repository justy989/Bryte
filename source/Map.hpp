#ifndef BRYTE_MAP_HPP
#define BRYTE_MAP_HPP

#include "Types.hpp"
#include "GameMemory.hpp"
#include "Utils.hpp"
#include "Direction.hpp"
#include "Pickup.hpp"
#include "Vector.hpp"
#include "Location.hpp"

// Note: Max map dimensions without scroll: 16 x 14

namespace bryte
{
     struct Interactives;
     struct Interactive;
     struct Enemy;

     class Map {
     public:

          static const Uint32 c_max_map_name_size = 64;
          static const Uint32 c_max_maps = 64;
          static const Uint32 c_max_dialogue_size = 64;

          static const Uint32 c_max_exits = 128;

          static const Int32  c_first_master_map = 0;

          static const Int32  c_tile_dimension_in_pixels = 16;
          static const Real32 c_tile_dimension_in_meters;

          static const Uint32 c_max_tiles = 1024;

          static const Uint8  c_light_decay = 48;

          static const Uint32 c_max_decors = 64;

          static const Uint32 c_max_lamps = 32;
          static const Uint8  c_unique_lamp_count = 5;
          static const Uint8  c_lamp_light = 224;

          static const Uint32 c_max_enemy_spawns = 32;

     public:

          enum TileFlags {
               solid = 1,
               invisible = 2,
          };

          struct Tile {
               Uint8 value;
               Uint8 flags;
          };

          struct Fixture {
               Void set ( Uint8 x, Uint8 y, Uint8 id );

               Coordinates coordinates;
               Uint8 id;
          };

          struct EnemySpawn : public Fixture {
               Direction    facing;
               Pickup::Type drop;
          };

          struct Secret {
               Bool found;
               Coordinates clear_tile;
               Coordinates coordinates;
          };

          struct PersistedExit {
               struct Map {
                    Uint8 index;
                    Coordinates coordinates;
               };

               PersistedExit::Map map [ 2 ];
               Uint8 state;
          };

          struct PersistEnemy {
               Uint8 alive;
               Coordinates coordinates;
          };

          struct PersistedEnemies {
               PersistEnemy enemies [ c_max_enemy_spawns ];
          };

          struct BorderExit {
               Coordinates bottom_left;

               Uint8 map_index;
               Coordinates map_bottom_left;
          };

     public:

          Map ( );

          Bool load_master_list ( const Char8* filepath );

          Void initialize ( Uint8 width, Uint8 height );

          Bool load_from_master_list ( Uint8 map_index, Interactives& interactives );
          Bool load ( const Char8* filepath, Interactives& interactives );
          Void save ( const Char8* filepath, Interactives& interactives );

          Bool save_persistence ( const Char8* region_name, Uint8 save_slot );
          Bool load_persistence ( const Char8* region_name, Uint8 save_slot );

          Int32 location_to_tile_index   ( const Location& loc ) const;
          Location tile_index_to_location ( Int32 tile_index ) const;

          Uint8 get_tile_location_value ( const Location& loc ) const;
          Bool  get_tile_location_solid ( const Location& loc ) const;
          Bool  get_tile_location_invisible ( const Location& loc ) const;
          Uint8 get_tile_location_light ( const Location& loc ) const;

          Void  set_tile_location_value ( const Location& loc, Uint8 value );
          Void  set_tile_location_solid ( const Location& loc, Bool solid );
          Void  set_tile_location_invisible ( const Location& loc, Bool invisible );

          Fixture*    check_location_for_decor       ( const Location& loc );
          Fixture*    check_location_for_lamp        ( const Location& loc );
          EnemySpawn* check_location_for_enemy_spawn ( const Location& loc );

          Uint8 base_light_value         ( ) const;
          Void  set_base_light           ( Uint8 base );
          Void  add_to_base_light        ( Uint8 delta );
          Void  subtract_from_base_light ( Uint8 delta );

          Void illuminate  ( const Location& loc, Uint8 value );
          Void reset_light ( );

          Bool add_decor       ( const Location& loc, Uint8 id );
          Bool add_lamp        ( const Location& loc, Uint8 id );
          Bool add_enemy_spawn ( const Location& loc, Uint8 id,
                                 Direction facing, Pickup::Type drop );

          Void remove_decor       ( Fixture* decor );
          Void remove_lamp        ( Fixture* lamp );
          Void remove_enemy_spawn ( EnemySpawn* enemy_spawn );

          Void set_border_exit ( Direction side, const BorderExit& border_exit );
          BorderExit& get_border_exit ( Direction side );

          Void persist_exit ( const Interactive& exit, const Coordinates& coords );
          Void persist_enemy ( const Enemy& enemy, Uint8 index );
          Void persist_secret ( );
          Void persist_killed_all_enemies ( );

          Void clear_persistence ( );

          Void find_secret ( );

          inline Int32 width  ( ) const;
          inline Int32 height ( ) const;

          inline Int32 light_width  ( ) const;
          inline Int32 light_height ( ) const;

          inline Uint8 decor_count       ( ) const;
          inline Uint8 lamp_count        ( ) const;
          inline Uint8 enemy_spawn_count ( ) const;

          inline Fixture&    decor       ( Uint8 index );
          inline Fixture&    lamp        ( Uint8 index );
          inline EnemySpawn& enemy_spawn ( Uint8 index );

          inline Bool tile_location_is_valid ( const Location& loc ) const;

          inline Int32 current_master_map ( ) const;
          inline Void set_activate_location_on_all_enemies_killed ( Coordinates loc );
          inline Coordinates activate_on_all_enemies_killed ( ) const;
          inline Void killed_all_enemies ( );

          inline Bool found_secret ( ) const;
          inline Secret& secret ( );

          inline Fixture& upgrade ( );

          inline Void set_secret_location ( Coordinates loc );
          inline Void set_secret_clear_tile ( Coordinates loc );

          inline const Char8* dialogue ( );

     public:

          static Location vector_to_location ( const Vector& v );
          static Vector location_to_vector ( const Location& loc );
          static Void convert_tiles_to_pixels ( Location* loc );
          static Void move_half_tile_in_pixels ( Location* loc );

     private:

          template < typename T >
          Bool add_element ( T* element_array, Uint8* element_count, Uint8 max_elements );

          template < typename T >
          Void remove_element ( T* element_array, Uint8* element_count, Uint8 max_elements, T* element );

          template < typename T >
          T* check_coordinates_for_fixture ( T* fixture_array, Uint8 fixture_count, Uint8 x, Uint8 y );

          Void restore_exits ( Interactives& interactives );
          Void restore_enemy_spawns ( );
          Void restore_activate_on_kill_all ( Interactives& interactives );

     private:

          Char8          m_master_list [ c_max_maps ][ c_max_map_name_size ];
          Char8          m_map_dialogue [ c_max_maps ][ c_max_dialogue_size ];
          Uint8          m_master_count;

          Int32          m_current_map;

          Tile           m_tiles [ c_max_tiles ];

          Uint8          m_width;
          Uint8          m_height;

          Uint8          m_base_light_value;
          Uint8          m_light [ c_max_tiles ];

          Fixture        m_decors [ c_max_decors ];
          Uint8          m_decor_count;

          Fixture        m_lamps [ c_max_lamps ];
          Uint8          m_lamp_count;

          EnemySpawn     m_enemy_spawns [ c_max_enemy_spawns ];
          Uint8          m_enemy_spawn_count;

          BorderExit     m_border_exits [ Direction::count ];

          Coordinates    m_activate_on_kill_all;

          Secret         m_secret;

          Fixture        m_upgrade;

          Bool           m_killed_all_enemies;

          PersistedExit  m_persisted_exits [ c_max_exits ];
          Uint8          m_persisted_exit_count;

          PersistedEnemies m_persisted_enemies [ c_max_maps ];

          Bool m_persisted_secrets [ c_max_maps ];
          Bool m_persisted_activate_on_kill_all [ c_max_maps ];
     };

     inline Int32 Map::width ( ) const
     {
          return m_width;
     }

     inline Int32 Map::height ( ) const
     {
          return m_height;
     }

     inline Uint8 Map::lamp_count ( ) const
     {
          return m_lamp_count;
     }

     inline Map::Fixture& Map::lamp ( Uint8 index )
     {
          ASSERT ( index < m_lamp_count );

          return m_lamps [ index ];
     }

     inline Map::Fixture& Map::decor ( Uint8 index )
     {
          ASSERT ( index < m_decor_count );

          return m_decors [ index ];
     }

     inline Uint8 Map::decor_count ( ) const
     {
          return m_decor_count;
     }

     inline Uint8 Map::enemy_spawn_count ( ) const
     {
          return m_enemy_spawn_count;
     }

     inline Map::EnemySpawn& Map::enemy_spawn ( Uint8 index )
     {
          ASSERT ( index < m_enemy_spawn_count );

          return m_enemy_spawns [ index ];
     }

     template < typename T >
     Bool Map::add_element ( T* element_array, Uint8* element_count, Uint8 max_elements )
     {
          if ( *element_count >= max_elements ) {
               return false;
          }

          (*element_count)++;

          return true;
     }

     template < typename T >
     Void Map::remove_element ( T* element_array, Uint8* element_count, Uint8 max_elements, T* element )
     {
          ASSERT ( element >= element_array && element < ( element_array + max_elements ) );

          T* last = element_array + ( *element_count - 1 );

          // slide down all the elements after it
          while ( element <= last ) {
               T* next = element + 1;
               *element = *next;
               element = next;
          }

          (*element_count)--;
     }

     template < typename T >
     T* Map::check_coordinates_for_fixture ( T* fixture_array, Uint8 fixture_count, Uint8 x, Uint8 y )
     {
          for ( Uint8 d = 0; d < fixture_count; ++d ) {
               Auto& fixture = fixture_array [ d ];

               if ( fixture.coordinates.x == x && fixture.coordinates.y == y ) {
                    return &fixture;
               }
          }

          return nullptr;
     }

     inline Bool Map::tile_location_is_valid ( const Location& loc ) const
     {
          return loc.x >= 0 && loc.x < width ( ) &&
                 loc.y >= 0 && loc.y < height ( );
     }

     inline Int32 Map::current_master_map ( ) const
     {
          return m_current_map;
     }

     inline Void Map::set_activate_location_on_all_enemies_killed ( Coordinates loc )
     {
          m_activate_on_kill_all = loc;
     }

     inline Coordinates Map::activate_on_all_enemies_killed ( ) const
     {
          return m_activate_on_kill_all;
     }

     inline Bool Map::found_secret ( ) const
     {
          return m_secret.found;
     }

     inline Map::Secret& Map::secret ( )
     {
          return m_secret;
     }

     inline Map::Fixture& Map::upgrade ( )
     {
          return m_upgrade;
     }

     inline Void Map::killed_all_enemies ( )
     {
          m_killed_all_enemies = true;
     }

     inline Void Map::set_secret_location ( Coordinates coords )
     {
          m_secret.coordinates = coords;
     }

     inline Void Map::set_secret_clear_tile ( Coordinates coords )
     {
          m_secret.clear_tile = coords;
     }

     inline const Char8* Map::dialogue ( )
     {
          return m_map_dialogue [ m_current_map ];
     }
}

#endif

