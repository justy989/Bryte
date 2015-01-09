#include "Map.hpp"
#include "Globals.hpp"
#include "Utils.hpp"

using namespace bryte;

const Int32  Map::c_tile_dimension_in_pixels = 16;
const Real32 Map::c_tile_dimension_in_meters = static_cast<Real32>( c_tile_dimension_in_pixels /
                                                                    pixels_per_meter );

static const Uint8  c_map_1_width    = 20;
static const Uint8  c_map_1_height   = 10;
static const Uint8  c_map_2_width    = 11;
static const Uint8  c_map_2_height   = 24;

Map::Map ( ) :
     m_current_room ( nullptr )
{

}

Void Map::build ( )
{
     Globals::g_memory_locations.rooms = Globals::g_game_memory.push_array<Room>( Map::c_max_rooms );
     auto* rooms                       = Globals::g_memory_locations.rooms;

     LOG_INFO ( "Building Room: %d by %d\n", c_map_1_width, c_map_1_height );
     LOG_INFO ( "Building Room: %d by %d\n", c_map_2_width, c_map_2_height );

     rooms [ 0 ].width  = c_map_1_width;
     rooms [ 0 ].height = c_map_1_height;

     rooms [ 1 ].width  = c_map_2_width;
     rooms [ 1 ].height = c_map_2_height;

     rooms [ 0 ].tiles  = Globals::g_game_memory.push_array<Uint8>( c_map_1_width * c_map_1_height );
     rooms [ 1 ].tiles  = Globals::g_game_memory.push_array<Uint8>( c_map_2_width * c_map_2_height );

     for ( Int32 i = 0; i < 2; ++i ) {
          auto*  tiles  = rooms [ i ].tiles;
          Uint32 width  = rooms [ i ].width;
          Uint32 height = rooms [ i ].height;

          for ( Uint32 y = 0; y < height; ++y ) {
               for ( Uint32 x = 0; x < width; ++x ) {
                    auto index = y * width + x;

                    if ( x == 0 ) {
                         tiles [ index ] = 7;
                    } else if ( x == width - 1 ) {
                         tiles [ index ] = 5;
                    } else if ( y == 0 ) {
                         tiles [ index ] = 6;
                    } else if ( y == height - 1 ) {
                         tiles [ index ] = 4;
                    } else {
                         tiles [ index ] = 0;
                    }
               }
          }

          tiles [ 0 ] = 11;
          tiles [ width - 1 ] = 10;
          tiles [ width * ( height - 1 ) ] = 8;
          tiles [ width * height - 1 ] = 9;
     }

     rooms [ 0 ].exit_count = 1;

     rooms [ 0 ].exits [ 0 ].location_x    = 1;
     rooms [ 0 ].exits [ 0 ].location_y    = 8;
     rooms [ 0 ].exits [ 0 ].room_index    = 1;
     rooms [ 0 ].exits [ 0 ].destination_x = 9;
     rooms [ 0 ].exits [ 0 ].destination_y = 1;

     rooms [ 1 ].exit_count = 1;

     rooms [ 1 ].exits [ 0 ].location_x    = 9;
     rooms [ 1 ].exits [ 0 ].location_y    = 1;
     rooms [ 1 ].exits [ 0 ].room_index    = 0;
     rooms [ 1 ].exits [ 0 ].destination_x = 1;
     rooms [ 1 ].exits [ 0 ].destination_y = 8;

     m_current_room = rooms;

     set_coordinate_value ( 1, 6, 4 );
     set_coordinate_value ( 2, 6, 4 );
     set_coordinate_value ( 3, 6, 13 );
     set_coordinate_value ( 3, 8, 7 );
     set_coordinate_value ( 3, 7, 7 );
}

Int32 Map::position_to_tile_index ( Real32 x, Real32 y )
{
     Int32 tile_x = x / c_tile_dimension_in_meters;
     Int32 tile_y = y / c_tile_dimension_in_meters;

     return coordinate_to_tile_index ( tile_x, tile_y );
}

Int32 Map::coordinate_to_tile_index ( Int32 tile_x, Int32 tile_y )
{
     ASSERT ( tile_x >= 0 && tile_x < m_current_room->width );
     ASSERT ( tile_y >= 0 && tile_y < m_current_room->height );

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
     Int32 player_tile_x = x / c_tile_dimension_in_meters;
     Int32 player_tile_y = y / c_tile_dimension_in_meters;

     for ( Uint8 d = 0; d < m_current_room->exit_count; ++d ) {
          auto& exit = m_current_room->exits [ d ];

          if ( exit.location_x == player_tile_x && exit.location_y == player_tile_y ) {
               m_current_room = &Globals::g_memory_locations.rooms [ exit.room_index ];
               return coordinate_to_tile_index ( exit.destination_x, exit.destination_y );
          }
     }

     return 0;
}

