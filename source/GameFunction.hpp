#ifndef GAME_FUNCTION_HPP
#define GAME_FUNCTION_HPP

#include "GameMemory.hpp"

#define PRINT_DL_ERROR(dl_api) LOG_ERROR ( "%s() failed: %s\n", dl_api, dlerror ( ) );

class GameInput;

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

struct GameFunctions
{
    GameFunctions ( );
    ~GameFunctions ( );

    Bool load ( const Char8* shared_library_path );

    GameInitFunc         game_init_func;
    GameDestroyFunc      game_destroy_func;
    GameUserInputFunc    game_user_input_func;
    GameUpdateFunc       game_update_func;
    GameRenderFunc       game_render_func;

#ifdef LINUX
    Void* shared_library_handle;
    Char8* shared_library_path;
#endif
};

#endif

