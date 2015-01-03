#include "GameInput.hpp"

Void GameInput::reset ( )
{
     key_change_count = 0;
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

