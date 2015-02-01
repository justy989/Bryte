#ifndef BRYTE_MAP_HPP
#define BRYTE_MAP_HPP

#include "Types.hpp"
#include "GameMemory.hpp"
#include "Utils.hpp"
#include "Direction.hpp"
#include "Pickup.hpp"

namespace bryte
{
     class Interactives;

     class Map {
     public:

// TODO: GTFO
#pragma pack(push, 1)
          struct Coordinates {
               Int32 x;
               Int32 y;
          };

          struct Location {
               Uint8 x;
               Uint8 y;
          };

          struct Tile {
               Uint8 value;
               Bool  solid;
          };

          struct Fixture {
               Void set ( Uint8 x, Uint8 y, Uint8 id );

               Location location;
               Uint8 id;
          };

          struct EnemySpawn : public Fixture {
               Direction    facing;
               Pickup::Type drop;
          };

          struct PersistedExits {
               static const Uint32 c_max_persisted_exits = 8;

               Uint8   exit_count;
               Fixture exits [ c_max_persisted_exits ];
          };
#pragma pack(pop)

     public:

          Map ( );

          Bool load_master_list ( const Char8* filepath );

          Void initialize ( Uint8 width, Uint8 height );

          Void save ( const Char8* filepath, Interactives& interactives );
          Void load ( const Char8* filepath, Interactives& interactives );
          Void load_from_master_list ( Uint8 map_index, Interactives& interactives );

          Int32 position_to_tile_index     ( Real32 x, Real32 y ) const;

          Int32 coordinate_to_tile_index   ( Int32 tile_x, Int32 tile_y ) const;
          Int32 tile_index_to_coordinate_x ( Int32 tile_index ) const;
          Int32 tile_index_to_coordinate_y ( Int32 tile_index ) const;

          Uint8 get_coordinate_value ( Int32 tile_x, Int32 tile_y ) const;
          Bool  get_coordinate_solid ( Int32 tile_x, Int32 tile_y ) const;
          Uint8 get_coordinate_light ( Int32 tile_x, Int32 tile_y ) const;

          Void  set_coordinate_value ( Int32 tile_x, Int32 tile_y, Uint8 value );
          Void  set_coordinate_solid ( Int32 tile_x, Int32 tile_y, Bool solid );

          Fixture*    check_coordinates_for_decor       ( Int32 x, Int32 y );
          Fixture*    check_coordinates_for_lamp        ( Int32 x, Int32 y );
          EnemySpawn* check_coordinates_for_enemy_spawn ( Int32 x, Int32 y );

          Uint8 base_light_value         ( ) const;
          Void  add_to_base_light        ( Uint8 delta );
          Void  subtract_from_base_light ( Uint8 delta );

          Void illuminate  ( Int32 x, Int32 y, Uint8 value );
          Void reset_light ( );

          Bool add_decor       ( Int32 location_x, Int32 location_y, Uint8 id );
          Bool add_lamp        ( Int32 location_x, Int32 location_y, Uint8 id );
          Bool add_enemy_spawn ( Int32 location_x, Int32 location_y, Uint8 id,
                                 Direction facing, Pickup::Type drop );

          Void remove_decor       ( Fixture* decor );
          Void remove_lamp        ( Fixture* lamp );
          Void remove_enemy_spawn ( EnemySpawn* enemy_spawn );

          inline Int32 width  ( ) const;
          inline Int32 height ( ) const;

          inline Uint8 decor_count       ( ) const;
          inline Uint8 lamp_count        ( ) const;
          inline Uint8 enemy_spawn_count ( ) const;

          inline Fixture&    decor       ( Uint8 index );
          inline Fixture&    lamp        ( Uint8 index );
          inline EnemySpawn& enemy_spawn ( Uint8 index );

     private:

          template < typename T >
          Bool add_element ( T* element_array, Uint8* element_count, Uint8 max_elements );

          template < typename T >
          Void remove_element ( T* element_array, Uint8* element_count, Uint8 max_elements, T* element );

          template < typename T >
          T* check_coordinates_for_fixture ( T* fixture_array, Uint8 fixture_count, Uint8 x, Uint8 y );

          Void persist_exits ( const Interactives& interactives );
          Void restore_exits ( Interactives& interactives );

     public:

          static const Uint32 c_max_map_name_size = 32;
          static const Uint32 c_max_maps = 32;

          static const Int32  c_first_master_map = -1;

          static const Int32  c_tile_dimension_in_pixels = 16;
          static const Real32 c_tile_dimension_in_meters;

          static const Uint32 c_max_tiles = 1024;

          static const Uint32 c_max_light = c_max_tiles;
          static const Int32  c_light_decay = 32;

          static const Uint32 c_max_decors = 64;

          static const Uint32 c_max_lamps = 32;
          static const Uint32 c_unique_lamp_count = 4;
          static const Uint8  c_unique_lamps_light [ c_unique_lamp_count ];

          static const Uint32 c_max_enemy_spawns = 32;

     private:

          Char8          m_master_list [ c_max_maps ][ c_max_map_name_size ];
          Uint8          m_master_count;

          PersistedExits m_persisted_exits [ c_max_maps ];

          Int32          m_current_master_map;

          Tile           m_tiles [ c_max_tiles ];

          Uint8          m_width;
          Uint8          m_height;

          Uint8          m_base_light_value;
          Uint8          m_light [ c_max_light ];

          Fixture        m_decors [ c_max_decors ];
          Uint8          m_decor_count;

          Fixture        m_lamps [ c_max_lamps ];
          Uint8          m_lamp_count;

          EnemySpawn     m_enemy_spawns [ c_max_enemy_spawns ];
          Uint8          m_enemy_spawn_count;
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

               if ( fixture.location.x == x && fixture.location.y == y ) {
                    return &fixture;
               }
          }

          return nullptr;
     }
}

#endif

