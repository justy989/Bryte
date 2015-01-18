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

          struct Fixture {
               Uint8 location_x;
               Uint8 location_y;

               Uint8 id;
          };

          struct Exit {
               Uint8 location_x;
               Uint8 location_y;

               Uint8 map_index;
               Uint8 exit_index;
          };

     public:

          Map ( );

          Bool load_master_list ( const Char8* filepath );

          Void initialize ( Uint8 width, Uint8 height );

          Void save ( const Char8* filepath );
          Void load ( const Char8* filepath );
          Void load_from_master_list ( Uint8 map_index );

          Int32 position_to_tile_index     ( Real32 x, Real32 y ) const;

          Int32 coordinate_to_tile_index   ( Int32 tile_x, Int32 tile_y ) const;
          Int32 tile_index_to_coordinate_x ( Int32 tile_index ) const;
          Int32 tile_index_to_coordinate_y ( Int32 tile_index ) const;

          Uint8 get_coordinate_value ( Int32 tile_x, Int32 tile_y ) const;
          Bool  get_coordinate_solid ( Int32 tile_x, Int32 tile_y ) const;
          Uint8 get_coordinate_light ( Int32 tile_x, Int32 tile_y ) const;

          Void  set_coordinate_value ( Int32 tile_x, Int32 tile_y, Uint8 value );
          Void  set_coordinate_solid ( Int32 tile_x, Int32 tile_y, Bool solid );

          Bool     is_position_solid           ( Real32 x, Real32 y ) const;
          Fixture* check_coordinates_for_decor ( Uint8 x, Uint8 y );
          Fixture* check_coordinates_for_lamp  ( Uint8 x, Uint8 y );
          Exit*    check_coordinates_for_exit  ( Uint8 x, Uint8 y );
          Fixture* check_coordinates_for_enemy_spawn ( Uint8 x, Uint8 y );

          Uint8 base_light_value ( ) const;
          Void  add_to_base_light ( Uint8 delta );
          Void  subtract_from_base_light ( Uint8 delta );

          Void illuminate ( Real32 x, Real32 y, Uint8 value );
          Void reset_light ( );

          Bool add_decor ( Uint8 location_x, Uint8 location_y, Uint8 id );
          Void remove_decor ( Fixture* decor );

          Bool add_lamp ( Uint8 location_x, Uint8 location_y, Uint8 id );
          Void remove_lamp ( Fixture* lamp );

          Bool add_enemy_spawn ( Uint8 location_x, Uint8 location_y, Uint8 id );
          Void remove_enemy_spawn ( Fixture* enemy_spawn );

          Bool add_exit ( Uint8 location_x, Uint8 location_y );
          Void remove_exit ( Exit* exit );

          inline Int32 width ( ) const;
          inline Int32 height ( ) const;

          inline Uint8 decor_count ( ) const;
          inline Fixture& decor ( Uint8 index );

          inline Uint8 lamp_count ( ) const;
          inline Fixture& lamp ( Uint8 index );

          inline Uint8 exit_count ( ) const;
          inline Exit& exit ( Uint8 index );

          inline Uint8 enemy_spawn_count ( ) const;
          inline Fixture& enemy_spawn ( Uint8 index );

     private:

          Bool add_fixture ( Fixture* fixture_array, Uint8* fixture_count, Uint8 max_fixtures,
                             Uint8 location_x, Uint8 location_y, Uint8 id );
          Void remove_fixture ( Fixture* fixture_array, Uint8* fixture_count, Uint8 max_fixtures,
                                Fixture* fixture );
          Fixture* check_coordinates_for_fixture ( Fixture* fixture_array, Uint8 fixture_count, Uint8 x, Uint8 y );

     public:

          static const Uint32 c_max_map_name_size = 32;
          static const Uint32 c_max_maps = 32;

          static const Int32  c_tile_dimension_in_pixels = 16;
          static const Real32 c_tile_dimension_in_meters;

          static const Uint32 c_max_tiles = 1024;
          static const Uint32 c_max_exits = 4;

          static const Uint32 c_max_light = c_max_tiles;
          static const Int32  c_light_decay = 32;

          static const Uint32 c_max_decors = 64;

          static const Uint32 c_max_lamps = 32;
          static const Uint32 c_unique_lamp_count = 4;
          static const Uint8  c_unique_lamps_light [ c_unique_lamp_count ];

          static const Uint32 c_max_enemy_spawns = 32;

     private:

          Char8  m_master_list [ c_max_maps ][ c_max_map_name_size ];
          Uint8  m_master_count;

          Tile   m_tiles [ c_max_tiles ];

          Uint8  m_width;
          Uint8  m_height;

          Uint8  m_base_light_value;
          Uint8  m_light [ c_max_light ];

          Fixture m_decors [ c_max_decors ];
          Uint8   m_decor_count;

          Fixture m_lamps [ c_max_lamps ];
          Uint8   m_lamp_count;

          Exit   m_exits [ c_max_exits ];
          Uint8  m_exit_count;

          Fixture m_enemy_spawns [ c_max_enemy_spawns ];
          Uint8   m_enemy_spawn_count;
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

     inline Uint8 Map::exit_count ( ) const
     {
          return m_exit_count;
     }

     inline Map::Exit& Map::exit ( Uint8 index )
     {
          ASSERT ( index < m_exit_count );

          return m_exits [ index ];
     }

     inline Uint8 Map::enemy_spawn_count ( ) const
     {
          return m_enemy_spawn_count;
     }

     inline Map::Fixture& Map::enemy_spawn ( Uint8 index )
     {
          ASSERT ( index < m_enemy_spawn_count );

          return m_enemy_spawns [ index ];
     }
}

#endif

