#include "GameInput.hpp"
#include "Log.hpp"

Void GameInput::reset ( )
{
     key_change_count               = 0;
     mouse_button_change_count      = 0;
     controller_button_change_count = 0;

     mouse_scroll = 0;
}

Bool GameInput::add_key_change ( SDL_Scancode scan_code, Bool down )
{
     if ( key_change_count < c_max_key_change_count ) {
          key_changes [ key_change_count ].scan_code = scan_code;
          key_changes [ key_change_count ].down      = down;

          key_change_count++;

          return true;
     }

     return false;
}

Bool GameInput::add_mouse_button_change ( Uint8 button, Bool down, Int32 x, Int32 y )
{
     if ( mouse_button_change_count < c_max_mouse_button_change_count ) {
          mouse_button_changes [ mouse_button_change_count ].button = button;
          mouse_button_changes [ mouse_button_change_count ].down   = down;

          mouse_position_x = x;
          mouse_position_y = y;

          mouse_button_change_count++;

          return true;
     }

     return false;
}

Bool GameInput::add_controller_button_change ( Uint8 button, Bool down )
{
     if ( controller_button_change_count < c_max_controller_button_change_count ) {
          controller_button_changes [ controller_button_change_count ].button = button;
          controller_button_changes [ controller_button_change_count ].down = down;

          controller_button_change_count++;

          return true;
     }

     return false;
}
