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
     { 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1 },
     { 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
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
     { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
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

     rooms [ 0 ].exits [ 0 ].location_x    = 3;
     rooms [ 0 ].exits [ 0 ].location_y    = 4;
     rooms [ 0 ].exits [ 0 ].room_index    = 1;
     rooms [ 0 ].exits [ 0 ].destination_x = 5;
     rooms [ 0 ].exits [ 0 ].destination_y = 2;

     rooms [ 1 ].tiles  = reinterpret_cast<Uint8*>( g_tilemap_2 );

     rooms [ 1 ].width  = c_map_2_width;
     rooms [ 1 ].height = c_map_2_height;

     rooms [ 1 ].exit_count = 1;

     rooms [ 1 ].exits [ 0 ].location_x    = 5;
     rooms [ 1 ].exits [ 0 ].location_y    = 2;
     rooms [ 1 ].exits [ 0 ].room_index    = 0;
     rooms [ 1 ].exits [ 0 ].destination_x = 3;
     rooms [ 1 ].exits [ 0 ].destination_y = 4;

     m_current_room = rooms;
}

Void Map::render ( SDL_Surface* surface, Real32 camera_x, Real32 camera_y )
{
     SDL_Rect tile_rect      { 0, 0,
                               meters_to_pixels ( Map::c_tile_dimension ),
                               meters_to_pixels ( Map::c_tile_dimension ) };
     Uint32   floor_color    = SDL_MapRGB ( surface->format, 190, 190, 190 );
     Uint32   wall_color     = SDL_MapRGB ( surface->format, 30, 30, 30 );
     Uint32   door_color     = SDL_MapRGB ( surface->format, 30, 110, 30 );

     for ( Int32 y = 0; y < static_cast<Int32>( m_current_room->height ); ++y ) {
          for ( Int32 x = 0; x < static_cast<Int32>( m_current_room->width ); ++x ) {

               auto   tile_index = coordinate_to_tile_index ( x, y );
               Uint32 tile_color = m_current_room->tiles [ tile_index ] ? wall_color : floor_color;

               for ( Uint8 d = 0; d < m_current_room->exit_count; ++d ) {
                    auto& exit = m_current_room->exits [ d ];

                    if ( exit.location_x == x && exit.location_y == y ) {
                         tile_color = door_color;
                         break;
                    }
               }

               tile_rect.x = x * meters_to_pixels ( c_tile_dimension );
               tile_rect.y = y * meters_to_pixels ( c_tile_dimension );

               tile_rect.x += meters_to_pixels ( camera_x );
               tile_rect.y += meters_to_pixels ( camera_y );

               convert_to_sdl_origin_for_surface ( tile_rect, surface );

               SDL_FillRect ( surface, &tile_rect, tile_color );
          }
     }
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

