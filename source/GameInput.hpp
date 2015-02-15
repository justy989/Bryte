#ifndef GAME_INPUT_HPP
#define GAME_INPUT_HPP

#include "Types.hpp"

#include <SDL2/SDL.h>

struct GameInput {
public:

     struct KeyChange {
          SDL_Scancode scan_code;
          bool         down;
     };

     struct ButtonChange {
          Uint8 button;
          bool  down;
     };

public:

     GameInput ( ) : 
          key_change_count ( 0 ), 
          mouse_button_change_count ( 0 ),
          controller_button_change_count ( 0 )
     { }

     Void reset          ( );
     Bool add_key_change ( SDL_Scancode scan_code, Bool down );
     Bool add_mouse_button_change ( Uint8 button, Bool down, Int32 x, Int32 y );
     Bool add_controller_button_change ( Uint8 button, Bool down );

public:

     static const Uint32 c_max_key_change_count = 8;
     static const Uint32 c_max_mouse_button_change_count = 4;
     static const Uint32 c_max_controller_button_change_count = 8;

public:

     KeyChange key_changes [ c_max_key_change_count ];
     Uint32    key_change_count;

     Int32     mouse_position_x;
     Int32     mouse_position_y;

     Int32     mouse_scroll;

     ButtonChange mouse_button_changes [ c_max_mouse_button_change_count ];
     Uint32 mouse_button_change_count;

     ButtonChange controller_button_changes [ c_max_controller_button_change_count ];
     Uint32 controller_button_change_count;
};

#endif

