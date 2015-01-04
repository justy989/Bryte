#include "Map.hpp"
#include "Globals.hpp"
#include "Utils.hpp"

using namespace bryte;

const Real32 Map::c_tile_dimension = 1.8f;

static const Uint8  c_map_1_width    = 20;
static const Uint8  c_map_1_height   = 16;
static const Uint8  c_map_2_width    = 16;
static const Uint8  c_map_2_height   = 24;

Uint8 g_tilemap_1 [ c_map_1_height ][ c_map_1_width ] = {
     { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
     { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 },
     { 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 },
     { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1 },
     { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1 },
     { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 },
     { 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 },
     { 1, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1 },
     { 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
     { 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
     { 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1 },
     { 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
     { 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
     { 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1 },
     { 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
     { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
};

Uint8 g_tilemap_2 [ c_map_2_height ][ c_map_2_width ] = {
     { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
     { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
     { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1 },
     { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
     { 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1 },
     { 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1 },
     { 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
     { 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
     { 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
     { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
     { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
     { 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1 },
     { 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
     { 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
     { 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
     { 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
     { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
     { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
     { 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1 },
     { 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
     { 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
     { 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
     { 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
     { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
};

Map::Map ( ) :
     m_current_room ( nullptr )
{

}

Void Map::build ( )
{
     Globals::g_memory_locations.rooms = Globals::g_game_memory.push_array<Room>( Map::c_max_rooms );
     auto* rooms                       = Globals::g_memory_locations.rooms;

     rooms [ 0 ].tiles  = reinterpret_cast<Uint8*>( g_tilemap_1 );

     rooms [ 0 ].width  = c_map_1_width;
     rooms [ 0 ].height = c_map_1_height;

     rooms [ 0 ].exit_count = 1;

     rooms [ 0 ].exits [ 0 ].location_x    = 1;
     rooms [ 0 ].exits [ 0 ].location_y    = 8;
     rooms [ 0 ].exits [ 0 ].room_index    = 1;
     rooms [ 0 ].exits [ 0 ].destination_x = 14;
     rooms [ 0 ].exits [ 0 ].destination_y = 1;

     rooms [ 1 ].tiles  = reinterpret_cast<Uint8*>( g_tilemap_2 );

     rooms [ 1 ].width  = c_map_2_width;
     rooms [ 1 ].height = c_map_2_height;

     rooms [ 1 ].exit_count = 1;

     rooms [ 1 ].exits [ 0 ].location_x    = 14;
     rooms [ 1 ].exits [ 0 ].location_y    = 1;
     rooms [ 1 ].exits [ 0 ].room_index    = 0;
     rooms [ 1 ].exits [ 0 ].destination_x = 1;
     rooms [ 1 ].exits [ 0 ].destination_y = 8;

     m_current_room = rooms;
}

Int32 Map::position_to_tile_index ( Real32 x, Real32 y )
{
     Int32 tile_x = x / c_tile_dimension;
     Int32 tile_y = y / c_tile_dimension;

     return coordinate_to_tile_index ( tile_x, tile_y );
}

Int32 Map::coordinate_to_tile_index ( Int32 tile_x, Int32 tile_y )
{
     ASSERT ( static_cast<Uint8>( tile_x ) < m_current_room->width );
     ASSERT ( static_cast<Uint8>( tile_y ) < m_current_room->height );

     return tile_y * static_cast<Int32>( m_current_room->width ) + tile_x;
}

Int32 Map::tile_index_to_coordinate_x ( Int32 tile_index )
{
     return tile_index % static_cast<Int32>( m_current_room->width );
}

Int32 Map::tile_index_to_coordinate_y ( Int32 tile_index )
{
     return tile_index / static_cast<Int32>( m_current_room->width );
}

Uint8 Map::get_coordinate_value ( Int32 tile_x, Int32 tile_y )
{
     return m_current_room->tiles [ coordinate_to_tile_index ( tile_x, tile_y ) ];
}

Void Map::set_coordinate_value ( Int32 tile_x, Int32 tile_y, Uint8 value )
{
     m_current_room->tiles [ coordinate_to_tile_index ( tile_x, tile_y ) ] = value;
}

Bool Map::is_position_solid ( Real32 x, Real32 y )
{
     return m_current_room->tiles [ position_to_tile_index ( x, y ) ] > 0;
}

Int32 Map::check_position_exit ( Real32 x, Real32 y )
{
     Int32 player_tile_x = x / c_tile_dimension;
     Int32 player_tile_y = y / c_tile_dimension;

     for ( Uint8 d = 0; d < m_current_room->exit_count; ++d ) {
          auto& exit = m_current_room->exits [ d ];

          if ( exit.location_x == player_tile_x && exit.location_y == player_tile_y ) {
               m_current_room = &Globals::g_memory_locations.rooms [ exit.room_index ];
               return coordinate_to_tile_index ( exit.destination_x, exit.destination_y );
          }
     }

     return 0;
}

