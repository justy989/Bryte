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

public:

     GameInput ( ) : key_change_count ( 0 ) { }

     Void reset          ( );
     Bool add_key_change ( SDL_Scancode scan_code, Bool down );

public:

     static const Uint32 c_max_key_change_count = 8;

public:

     KeyChange key_changes [ c_max_key_change_count ];
     Uint32    key_change_count;
};

#endif

