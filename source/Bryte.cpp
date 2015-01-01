#include "Bryte.hpp"

const Int32 c_player_width  = 16;
const Int32 c_player_height = 24;

Bryte_State g_bryte_state;

extern "C" Bool bryte_init ( )
{
     g_bryte_state.player_position_x = 50;
     g_bryte_state.player_position_y = 60;

     return true;
}

extern "C" Void bryte_destroy ( )
{

}

extern "C" Void bryte_user_input ( SDL_Scancode scan_code )
{

}

extern "C" Void bryte_update ( Real32 time_delta )
{

}

extern "C" Void bryte_render ( SDL_Surface* back_buffer )
{
     SDL_Rect player_rect { g_bryte_state.player_position_x,
                            g_bryte_state.player_position_y,
                            c_player_width, c_player_height };

     Uint32 red = SDL_MapRGB ( back_buffer->format, 255, 0, 0 );

     SDL_FillRect ( back_buffer, &player_rect, red );
}

