#ifndef BRYTE_EDITOR_HPP
#define BRYTE_EDITOR_HPP

#include <SDL2/SDL.h>

#include "GameMemory.hpp"
#include "GameInput.hpp"
#include "Map.hpp"

namespace editor {
     struct State {
          bryte::Map map;

     };
}

// exported functions to be called by the application
extern "C" Bool game_init ( GameMemory& );
extern "C" Void game_destroy ( );
extern "C" Void game_reload_memory ( GameMemory& );
extern "C" Void game_user_input ( const GameInput& );
extern "C" Void game_update ( Real32 );
extern "C" Void game_render ( SDL_Surface* );

#endif

