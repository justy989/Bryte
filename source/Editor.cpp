#include "Editor.hpp"
#include "Log.hpp"

using namespace bryte;

extern "C" Bool game_init ( GameMemory& )
{
     LOG_INFO ( "Initializing Editor\n" );

     return true;
}

extern "C" Void game_destroy ( )
{

}

extern "C" Void game_reload_memory ( GameMemory& )
{

}

extern "C" Void game_user_input ( const GameInput& )
{

}

extern "C" Void game_update ( Real32 )
{

}

extern "C" Void game_render ( SDL_Surface* )
{

}

