#ifndef BRYTE_EDITOR_HPP
#define BRYTE_EDITOR_HPP

#include <SDL2/SDL.h>

#include "GameMemory.hpp"
#include "GameInput.hpp"
#include "Map.hpp"

namespace editor
{
     struct State {
          bryte::Map map;

          bryte::Map::Room room;

          SDL_Surface* tilesheet;
     };
}

// exported functions to be called by the application
extern "C" Bool game_init ( GameMemory& );
extern "C" Void game_destroy ( GameMemory& );
extern "C" Void game_user_input ( GameMemory&, const GameInput& );
extern "C" Void game_update ( GameMemory&, Real32 );
extern "C" Void game_render ( GameMemory&, SDL_Surface* );

#endif

