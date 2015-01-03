#include "Bryte.hpp"
#include "Utils.hpp"

using namespace bryte;

static const Int32  c_player_width        = 16;
static const Int32  c_player_height       = 24;
static const Int32  c_half_player_width   = c_player_width / 2;
static const Int32  c_half_player_height  = c_player_height / 2;
static const Real32 c_player_speed        = 50.0f;

static const Uint8 c_width_1         = 20;
static const Uint8 c_height_1        = 16;
static const Uint8 c_width_2         = 16;
static const Uint8 c_height_2        = 24;
static const Uint8 c_tile_dimension  = 16;

GameMemory      g_game_memory;
MemoryLocations g_memory_locations;

Uint8 g_tilemap_1 [ c_height_1 ][ c_width_1 ] = {
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

Uint8 g_tilemap_2 [ c_height_2 ][ c_width_2 ] = {
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

Void Map::build ( )
{
     m_tile_dimension = c_tile_dimension;

     g_memory_locations.rooms = g_game_memory.push_array<Room>( Map::c_max_rooms );
     auto* rooms              = g_memory_locations.rooms;


     rooms [ 0 ].m_tiles  = reinterpret_cast<Uint8*>( g_tilemap_1 );

     rooms [ 0 ].m_width  = c_width_1;
     rooms [ 0 ].m_height = c_height_1;

     rooms [ 0 ].m_exit_count = 1;

     rooms [ 0 ].m_exits [ 0 ].location_x    = 3;
     rooms [ 0 ].m_exits [ 0 ].location_y    = 4;
     rooms [ 0 ].m_exits [ 0 ].room_index    = 1;
     rooms [ 0 ].m_exits [ 0 ].destination_x = 5;
     rooms [ 0 ].m_exits [ 0 ].destination_y = 2;


     rooms [ 1 ].m_tiles  = reinterpret_cast<Uint8*>( g_tilemap_2 );

     rooms [ 1 ].m_width  = c_width_2;
     rooms [ 1 ].m_height = c_height_2;

     rooms [ 1 ].m_exit_count = 1;

     rooms [ 1 ].m_exits [ 0 ].location_x    = 5;
     rooms [ 1 ].m_exits [ 0 ].location_y    = 2;
     rooms [ 1 ].m_exits [ 0 ].room_index    = 0;
     rooms [ 1 ].m_exits [ 0 ].destination_x = 3;
     rooms [ 1 ].m_exits [ 0 ].destination_y = 4;

     m_room = rooms;
}

Void Map::render ( SDL_Surface* surface, Real32 camera_x, Real32 camera_y )
{
     SDL_Rect tile_rect   { 0, 0, m_tile_dimension, m_tile_dimension };
     Uint32   floor_color = SDL_MapRGB ( surface->format, 190, 190, 190 );
     Uint32   wall_color  = SDL_MapRGB ( surface->format, 30, 30, 30 );
     Uint32   door_color  = SDL_MapRGB ( surface->format, 30, 110, 30 );

     for ( Uint32 y = 0; y < m_room->m_height; ++y ) {
          for ( Uint32 x = 0; x < m_room->m_width; ++x ) {

               auto   tile_index = y * m_room->m_width + x;
               Uint32 tile_color = m_room->m_tiles [ tile_index ] ? wall_color : floor_color;

               for ( Uint32 d = 0; d < m_room->m_exit_count; ++d ) {
                    if ( m_room->m_exits [ d ].location_x == x &&
                         m_room->m_exits [ d ].location_y == y ) {
                         tile_color = door_color;
                         break;
                    }
               }

               tile_rect.x = x * m_tile_dimension;
               tile_rect.y = y * m_tile_dimension;

               tile_rect.x += static_cast<Int32>( camera_x );
               tile_rect.y += static_cast<Int32>( camera_y );

               SDL_FillRect ( surface, &tile_rect, tile_color );
          }
     }
}

Uint32 Map::map_to_tile_index ( Uint32 x, Uint32 y )
{
     Uint32 tile_x = x / static_cast<Uint32>( m_tile_dimension );
     Uint32 tile_y = y / static_cast<Uint32>( m_tile_dimension );

     return get_tile_index ( tile_x, tile_y );
}

Uint32 Map::get_tile_index ( Uint32 tile_x, Uint32 tile_y )
{
     ASSERT ( static_cast<Uint8>( tile_x ) < m_room->m_width );
     ASSERT ( static_cast<Uint8>( tile_y ) < m_room->m_height );

     return tile_y * static_cast<Uint32>( m_room->m_width ) + tile_x;
}

Uint32 Map::get_tile_index_x ( Uint32 tile_index )
{
     return tile_index % static_cast<Uint32>( m_room->m_width );
}

Uint32 Map::get_tile_index_y ( Uint32 tile_index )
{
     return tile_index / static_cast<Uint32>( m_room->m_width );
}

Bool Map::is_tile_solid_on_map ( Uint32 x, Uint32 y )
{
     return m_room->m_tiles [ map_to_tile_index ( x, y ) ] > 0;
}

Uint32 Map::check_player_exit ( Uint32 x, Uint32 y )
{
     Uint32 player_tile_x = x / static_cast<Uint32>( m_tile_dimension );
     Uint32 player_tile_y = y / static_cast<Uint32>( m_tile_dimension );

     for ( Uint8 d = 0; d < m_room->m_exit_count; ++d ) {
          auto& exit = m_room->m_exits [ d ];

          if ( exit.location_x == player_tile_x && exit.location_y == player_tile_y ) {
               m_room = &g_memory_locations.rooms [ exit.room_index ];
               return get_tile_index ( exit.destination_x, exit.destination_y );
          }
     }

     return 0;
}

extern "C" Bool bryte_init ( GameMemory& game_memory )
{
     g_game_memory.memory = game_memory.memory;
     g_game_memory.size   = game_memory.size;

     g_memory_locations.game_state = g_game_memory.push_object<GameState> ( );

     auto* game_state = g_memory_locations.game_state;

     game_state->player_position_x = 20.0f;
     game_state->player_position_y = 20.0f;

     game_state->map.build ( );

     return true;
}

extern "C" Void bryte_destroy ( )
{

}

extern "C" Void bryte_reload_memory ( GameMemory& game_memory )
{
     g_game_memory.memory = game_memory.memory;
     g_game_memory.size   = game_memory.size;

     auto* game_state = g_memory_locations.game_state;

     game_state->map.build ( );
}

extern "C" Void bryte_user_input ( SDL_Scancode scan_code, bool key_down )
{
     auto* game_state = g_memory_locations.game_state;

     switch ( scan_code ) {
     default:
          break;
     case SDL_SCANCODE_W:
          game_state->direction_keys [ Direction::up ] = key_down;
          break;
     case SDL_SCANCODE_S:
          game_state->direction_keys [ Direction::down ] = key_down;
          break;
     case SDL_SCANCODE_A:
          game_state->direction_keys [ Direction::left ] = key_down;
          break;
     case SDL_SCANCODE_D:
          game_state->direction_keys [ Direction::right ] = key_down;
          break;
     }
}

extern "C" Void bryte_update ( Real32 time_delta )
{
     auto* game_state = g_memory_locations.game_state;

     Real32 target_position_x = game_state->player_position_x;
     Real32 target_position_y = game_state->player_position_y;

     if ( game_state->direction_keys [ Direction::up ] ) {
          target_position_y -= c_player_speed * time_delta;
     }

     if ( game_state->direction_keys [ Direction::down ] ) {
          target_position_y += c_player_speed * time_delta;
     }

     if ( game_state->direction_keys [ Direction::left ] ) {
          target_position_x -= c_player_speed * time_delta;
     }

     if ( game_state->direction_keys [ Direction::right ] ) {
          target_position_x += c_player_speed * time_delta;
     }

     // collision
     if ( !game_state->map.is_tile_solid_on_map ( target_position_x,
                                                  target_position_y ) ) {
          game_state->player_position_x = target_position_x;
          game_state->player_position_y = target_position_y;
     }

     auto& player_exit = game_state->player_exit_destination;
     auto& map         = game_state->map;

     // check if the player has exitted
     if ( !player_exit ) {
          player_exit = map.check_player_exit ( game_state->player_position_x,
                                                game_state->player_position_y );

          if ( player_exit ) {
               game_state->player_position_x = map.get_tile_index_x ( player_exit );
               game_state->player_position_y = map.get_tile_index_y ( player_exit );

               game_state->player_position_x *= map.m_tile_dimension;
               game_state->player_position_y *= map.m_tile_dimension;
          }
     } else {
          auto player_tile_index = map.map_to_tile_index ( game_state->player_position_x,
                                                           game_state->player_position_y );

          // clear the exit destination if they've left the tile
          if ( player_exit != player_tile_index ) {
               player_exit = 0;
          }
     }
}

extern "C" Void bryte_render ( SDL_Surface* back_buffer )
{
     auto* game_state = g_memory_locations.game_state;

     game_state->camera_x = -( game_state->player_position_x - back_buffer->w / 2 );
     game_state->camera_y = -( game_state->player_position_y - back_buffer->h / 2 );

     Real32 min_x = -( game_state->map.m_room->m_width * game_state->map.m_tile_dimension - back_buffer->w );
     Real32 min_y = -( game_state->map.m_room->m_height * game_state->map.m_tile_dimension - back_buffer->h );

     CLAMP ( game_state->camera_x, min_x, 0 );
     CLAMP ( game_state->camera_y, min_y, 0 );

     game_state->map.render ( back_buffer, game_state->camera_x, game_state->camera_y );

     Int32 player_screen_x = static_cast<Int32>( game_state->player_position_x + game_state->camera_x );
     Int32 player_screen_y = static_cast<Int32>( game_state->player_position_y + game_state->camera_y );

     SDL_Rect player_rect { player_screen_x - c_half_player_width,
                            player_screen_y - c_half_player_height,
                            c_player_width, c_player_height };

     Uint32 red = SDL_MapRGB ( back_buffer->format, 255, 0, 0 );

     SDL_FillRect ( back_buffer, &player_rect, red );
}

