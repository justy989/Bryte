#include "Bryte.hpp"

using namespace bryte;

const Int32  c_player_width  = 16;
const Int32  c_player_height = 24;
const Real32 c_player_speed  = 100.0f;

GameMemory g_game_memory;

static const Int32 c_width       = 8;
static const Int32 c_height      = 6;
static const Int32 c_tile_width  = 24;
static const Int32 c_tile_height = 24;

Int8 g_tiles [ c_height ][ c_width ] = {
     { 1, 1, 1, 1, 1, 1, 1, 1 },
     { 1, 0, 0, 0, 0, 0, 0, 1 },
     { 1, 0, 1, 0, 0, 1, 0, 1 },
     { 1, 0, 1, 0, 0, 1, 0, 1 },
     { 1, 0, 0, 0, 0, 0, 0, 1 },
     { 1, 1, 1, 1, 1, 1, 1, 1 }
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

Void Map::render ( SDL_Surface* surface )
{
     SDL_Rect tile_rect   { 0, 0, m_tile_width, m_tile_height };
     Uint32   floor_color = SDL_MapRGB ( surface->format, 190, 190, 190 );
     Uint32   wall_color  = SDL_MapRGB ( surface->format, 30, 30, 30 );

     for ( Int32 y = 0; y < m_height; ++y ) {
          for ( Int32 x = 0; x < m_width; ++x ) {

               tile_rect.x = x * m_tile_width;
               tile_rect.y = y * m_tile_height;

               SDL_FillRect ( surface, &tile_rect,
                              m_tiles [ y * m_width + x ] ? wall_color : floor_color );
          }
     }
}

extern "C" Bool bryte_init ( void* memory, Uint32 memory_size )
{
     g_game_memory.memory = memory;
     g_game_memory.size   = memory_size;

     GameState* game_state = get_game_state ( );

     game_state->player_position_x = 20.0f;
     game_state->player_position_y = 20.0f;

     return true;
}

extern "C" Void bryte_destroy ( )
{

}

extern "C" Void bryte_reload_memory ( void* memory, Uint32 size )
{
     g_game_memory.memory = memory;
     g_game_memory.size   = size;
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
     static bool did_stuff = false;

     if ( !did_stuff ) {
          did_stuff = true;
          game_state->player_position_x = 20.0f;
          game_state->player_position_y = 20.0f;
     }

     if ( game_state->direction_keys [ Direction::up ] ) {
          game_state->player_position_y -= c_player_speed * time_delta;
     }

     if ( game_state->direction_keys [ Direction::down ] ) {
          game_state->player_position_y += c_player_speed * time_delta;
     }

     if ( game_state->direction_keys [ Direction::left ] ) {
          game_state->player_position_x -= c_player_speed * time_delta;
     }

     if ( game_state->direction_keys [ Direction::right ] ) {
          game_state->player_position_x += c_player_speed * time_delta;
     }
}

extern "C" Void bryte_render ( SDL_Surface* back_buffer )
{
     GameState* game_state = get_game_state ( );

     game_state->map.build ( );
     game_state->map.render ( back_buffer );

     SDL_Rect player_rect { static_cast<int>( game_state->player_position_x ),
                            static_cast<int>( game_state->player_position_y ),
                            c_player_width, c_player_height };

     Uint32 red = SDL_MapRGB ( back_buffer->format, 255, 0, 0 );

     SDL_FillRect ( back_buffer, &player_rect, red );
}

