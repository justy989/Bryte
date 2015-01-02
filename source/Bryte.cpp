#include "Bryte.hpp"

using namespace bryte;

const Int32 c_player_width  = 16;
const Int32 c_player_height = 24;
const Int32 c_player_speed = 1;

Game_State g_game_state;

extern "C" Bool bryte_init ( )
{
     g_game_state.player_position_x = 50;
     g_game_state.player_position_y = 60;

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
          g_game_state.player_position_x = 20;
          g_game_state.player_position_y = 20;
     }

     if ( g_game_state.direction_keys [ Direction::up ] ) {
          g_game_state.player_position_y -= c_player_speed;
     }

     if ( g_game_state.direction_keys [ Direction::down ] ) {
          g_game_state.player_position_y += c_player_speed;
     }

     if ( g_game_state.direction_keys [ Direction::left ] ) {
          g_game_state.player_position_x -= c_player_speed;
     }

     if ( g_game_state.direction_keys [ Direction::right ] ) {
          g_game_state.player_position_x += c_player_speed;
     }
}

extern "C" Void bryte_render ( SDL_Surface* back_buffer )
{
     SDL_Rect player_rect { g_game_state.player_position_x,
                            g_game_state.player_position_y,
                            c_player_width, c_player_height };

     Uint32 red = SDL_MapRGB ( back_buffer->format, 255, 0, 0 );

     SDL_FillRect ( back_buffer, &player_rect, red );
}

