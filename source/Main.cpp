#include <cstdio>

#include <SDL2/SDL.h>

#include "Application.hpp"
#include "Utils.hpp"
#include "Log.hpp"

int main ( int argc, char** argv )
{
     Application platform;
     Application::Settings settings;

     settings.window_title                  = "Bryte 0.01";
     settings.window_width                  = 1024;
     settings.window_height                 = 768;

     settings.back_buffer_width             = 256;
     settings.back_buffer_height            = 240;

     settings.shared_library_path           = "./bryte_game.so";

     settings.game_memory_allocation_size   = MEGABYTES ( 32 );

     settings.locked_frames_per_second      = 30;

     return platform.run_game ( settings ) ? 0 : 1;
}

