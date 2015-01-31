#include "GameFunction.hpp"
#include "Utils.hpp"

#ifdef LINUX
    #include <dlfcn.h>

const Char8* c_game_func_strs [ GameFunctions::c_func_count ] = {
    "game_init",
    "game_destroy",
    "game_user_input",
    "game_update",
    "game_render"
};

#else
    #include "Bryte.hpp"
#endif

GameFunctions::GameFunctions ( ) :
#ifdef LINUX
    shared_library_handle ( nullptr ),
    shared_library_filepath ( nullptr ),
#endif
    game_init_func ( nullptr ),
    game_destroy_func ( nullptr ),
    game_user_input_func ( nullptr ),
    game_update_func ( nullptr ),
    game_render_func ( nullptr )
{

}

GameFunctions::~GameFunctions ( )
{
#ifdef LINUX
    if ( shared_library_handle ) {
        LOG_INFO ( "Closing shared library\n" );
        dlclose ( shared_library_handle );
    }
#endif
}

Bool GameFunctions::load ( const Char8* shared_library_path )
{
#ifdef LINUX
    if ( shared_library_handle ) {
        LOG_INFO ( "Freeing shared library handle\n" );
        dlclose ( shared_library_handle );
    }

    LOG_INFO ( "Loading shared library: %s\n", shared_library_path );
    shared_library_handle = dlopen ( shared_library_path, RTLD_LAZY );

    if ( !shared_library_handle ) {
        PRINT_DL_ERROR ( "dlopen" );
        return false;
    }

    Char8* save_path = strdup ( shared_library_path );

    // if we succeded, save the shared library path
    if ( shared_library_filepath ) {
        free ( shared_library_filepath );
    }

    shared_library_filepath = save_path;

    // load each func and validate they succeeded in loading
    Void* game_funcs [ c_func_count ];

    for ( Int32 i = 0; i < c_func_count; ++i ) {
        LOG_INFO ( "Loading function: %s\n", c_game_func_strs [ i ] );
        game_funcs [ i ] = dlsym ( shared_library_handle, c_game_func_strs [ i ] );

        if ( !game_funcs [ i ] ) {
            PRINT_DL_ERROR ( "dlsym" );
            return false;
        }
    }

    // set the loaded functions
    game_init_func = reinterpret_cast<GameInitFunc>( game_funcs [ 0 ] );
    game_destroy_func = reinterpret_cast<GameDestroyFunc>( game_funcs [ 1 ] );
    game_user_input_func = reinterpret_cast<GameUserInputFunc>( game_funcs [ 2 ] );
    game_update_func = reinterpret_cast<GameUpdateFunc>( game_funcs [ 3 ] );
    game_render_func = reinterpret_cast<GameRenderFunc>( game_funcs [ 4 ] );
#else

    (Void*)shared_library_path; // unused

    game_init_func = game_init;
    game_destroy_func = game_destroy;
    game_user_input_func = game_user_input;
    game_update_func = game_update;
    game_render_func = game_render;
#endif

    return true;
}
