#include "Bryte.hpp"

using namespace bryte;

const Int32  c_player_width  = 16;
const Int32  c_player_height = 24;
const Real32 c_player_speed  = 100.0f;

Game_State g_game_state;

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

extern "C" Bool bryte_init ( )
{
     g_game_state.player_position_x = 20.0f;
     g_game_state.player_position_y = 20.0f;

     return true;
}

extern "C" Void bryte_destroy ( )
{

}

extern "C" Void bryte_user_input ( SDL_Scancode scan_code, bool key_down )
{
     switch ( scan_code ) {
     default:
          break;
     case SDL_SCANCODE_W:
          g_game_state.direction_keys [ Direction::up ] = key_down;
          break;
     case SDL_SCANCODE_S:
          g_game_state.direction_keys [ Direction::down ] = key_down;
          break;
     case SDL_SCANCODE_A:
          g_game_state.direction_keys [ Direction::left ] = key_down;
          break;
     case SDL_SCANCODE_D:
          g_game_state.direction_keys [ Direction::right ] = key_down;
          break;
     }
}

extern "C" Void bryte_update ( Real32 time_delta )
{
     static bool did_stuff = false;

     if ( !did_stuff ) {
          did_stuff = true;
          g_game_state.player_position_x = 20.0f;
          g_game_state.player_position_y = 20.0f;
     }

     if ( g_game_state.direction_keys [ Direction::up ] ) {
          g_game_state.player_position_y -= c_player_speed * time_delta;
     }

     if ( g_game_state.direction_keys [ Direction::down ] ) {
          g_game_state.player_position_y += c_player_speed * time_delta;
     }

     if ( g_game_state.direction_keys [ Direction::left ] ) {
          g_game_state.player_position_x -= c_player_speed * time_delta;
     }

     if ( g_game_state.direction_keys [ Direction::right ] ) {
          g_game_state.player_position_x += c_player_speed * time_delta;
     }
}

extern "C" Void bryte_render ( SDL_Surface* back_buffer )
{
     g_game_state.map.build ( );
     g_game_state.map.render ( back_buffer );

     SDL_Rect player_rect { static_cast<int>( g_game_state.player_position_x ),
                            static_cast<int>( g_game_state.player_position_y ),
                            c_player_width, c_player_height };

     Uint32 red = SDL_MapRGB ( back_buffer->format, 255, 0, 0 );

     SDL_FillRect ( back_buffer, &player_rect, red );
}

