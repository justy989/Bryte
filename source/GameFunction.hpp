#ifndef GAME_FUNCTION_HPP
#define GAME_FUNCTION_HPP

#include "GameMemory.hpp"

// exported functions to be called by the application
extern "C" Bool game_init_stub       ( GameMemory&, void* settings );
extern "C" Void game_destroy_stub    ( GameMemory& );
extern "C" Void game_user_input_stub ( GameMemory&, const GameInput& );
extern "C" Void game_update_stub     ( GameMemory&, Real32 );
extern "C" Void game_render_stub     ( GameMemory&, SDL_Surface* );

// exported function types
using GameInitFunc         = decltype ( game_init_stub )*;
using GameDestroyFunc      = decltype ( game_destroy_stub )*;
using GameUserInputFunc    = decltype ( game_user_input_stub )*;
using GameUpdateFunc       = decltype ( game_update_stub )*;
using GameRenderFunc       = decltype ( game_render_stub )*;

#endif

