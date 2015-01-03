#include "Bryte.hpp"
#include "Utils.hpp"

using namespace bryte;

const Int32  c_player_width  = 16;
const Int32  c_player_height = 24;
const Real32 c_player_speed  = 50.0f;

GameMemory g_game_memory;

static const Int32 c_width       = 20;
static const Int32 c_height      = 16;
static const Int32 c_tile_width  = 16;
static const Int32 c_tile_height = 16;

Int8 g_tiles [ c_height ][ c_width ] = {
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

static GameState* get_game_state ( )
{
     return reinterpret_cast<GameState*>( reinterpret_cast<char*>( g_game_memory.memory ) + 0 );
}

Void Map::build ( )
{
     m_tiles       = reinterpret_cast<Int8*>( g_tiles );

     m_width       = c_width;
     m_height      = c_height;

     m_tile_width  = c_tile_width;
     m_tile_height = c_tile_height;
}

Void Map::render ( SDL_Surface* surface, Real32 camera_x, Real32 camera_y )
{
     SDL_Rect tile_rect   { 0, 0, m_tile_width, m_tile_height };
     Uint32   floor_color = SDL_MapRGB ( surface->format, 190, 190, 190 );
     Uint32   wall_color  = SDL_MapRGB ( surface->format, 30, 30, 30 );

     for ( Int32 y = 0; y < m_height; ++y ) {
          for ( Int32 x = 0; x < m_width; ++x ) {

               tile_rect.x = x * m_tile_width;
               tile_rect.y = y * m_tile_height;

               tile_rect.x += static_cast<Int32>( camera_x );
               tile_rect.y += static_cast<Int32>( camera_y );

               SDL_FillRect ( surface, &tile_rect,
                              m_tiles [ y * m_width + x ] ? wall_color : floor_color );
          }
     }
}

Bool Map::is_tile_solid ( Int32 x, Int32 y )
{
     int index_x = x / m_tile_width;
     int index_y = y / m_tile_height;

     ASSERT ( index_x >= 0 && index_x < m_width );
     ASSERT ( index_y >= 0 && index_y < m_height );

     Int32 final_index = index_y * m_width + index_x;

     return m_tiles [ final_index ];
}

extern "C" Bool bryte_init ( bryte::GameMemory& game_memory )
{
     g_game_memory.memory = game_memory.memory;
     g_game_memory.size   = game_memory.size;

     GameState* game_state = get_game_state ( );

     game_state->player_position_x = 20.0f;
     game_state->player_position_y = 20.0f;

     game_state->map.build ( );

     return true;
}

extern "C" Void bryte_destroy ( )
{

}

extern "C" Void bryte_reload_memory ( bryte::GameMemory& game_memory )
{
     g_game_memory.memory = game_memory.memory;
     g_game_memory.size   = game_memory.size;

     GameState* game_state = get_game_state ( );

     game_state->map.build ( );
}

extern "C" Void bryte_user_input ( SDL_Scancode scan_code, bool key_down )
{
     GameState* game_state = get_game_state ( );

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
     GameState* game_state = get_game_state ( );

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

     if ( !game_state->map.is_tile_solid ( static_cast<Int32>( target_position_x ),
                                           static_cast<Int32>( target_position_y ) ) ) {
          game_state->player_position_x = target_position_x;
          game_state->player_position_y = target_position_y;
     }
}

extern "C" Void bryte_render ( SDL_Surface* back_buffer )
{
     GameState* game_state = get_game_state ( );

     game_state->camera_x = -( game_state->player_position_x - back_buffer->w / 2 );
     game_state->camera_y = -( game_state->player_position_y - back_buffer->h / 2 );

     Real32 min_x = -( game_state->map.m_width * game_state->map.m_tile_width - back_buffer->w );
     Real32 min_y = -( game_state->map.m_height * game_state->map.m_tile_height - back_buffer->h );

     CLAMP ( game_state->camera_x, min_x, 0 );
     CLAMP ( game_state->camera_y, min_y, 0 );

     game_state->map.render ( back_buffer, game_state->camera_x, game_state->camera_y );

     SDL_Rect player_rect { static_cast<Int32>( game_state->player_position_x + game_state->camera_x ),
                            static_cast<Int32>( game_state->player_position_y + game_state->camera_y ),
                            c_player_width, c_player_height };

     Uint32 red = SDL_MapRGB ( back_buffer->format, 255, 0, 0 );

     SDL_FillRect ( back_buffer, &player_rect, red );
}

