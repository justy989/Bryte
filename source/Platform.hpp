#ifndef BRYTE_PLATFORM_HPP
#define BRYTE_PLATFORM_HPP

#include "Bryte.hpp"

#include <SDL2/SDL.h>

#define PRINT_SDL_ERROR(sdl_api) printf ( "%s() failed: %s\n", sdl_api, SDL_GetError ( ) );
#define PRINT_DL_ERROR(dl_api) printf ( "%s() failed: %s\n", dl_api, dlerror ( ) );

// Create's a platform application to run the game code
class Platform {
public:

     Platform ( );
     ~Platform ( );

     Bool create_window ( const Char8* window_title, Int32 window_width, Int32 window_height,
                          Int32 back_buffer_width, Int32 back_buffer_height );

     Bool load_game_code ( const Char8* shared_library_path );

     Bool run_game ( );

private:

     static const int c_func_count = 5;
     static const Char8* c_game_func_strs [ c_func_count ];

private:

     // SDL components required to make window and draw to it
     SDL_Window*   m_window;
     SDL_Renderer* m_renderer;
     SDL_Texture*  m_back_buffer_texture;
     SDL_Surface*  m_back_buffer_surface;

     // loaded share library attributes
     Char8* m_shared_library_path;
     void* m_shared_library_handle;

     // functions loaded from game shared library
     Game_Init_Func       m_game_init_func;
     Game_Destroy_Func    m_game_destroy_func;
     Game_User_Input_Func m_game_user_input_func;
     Game_Update_Func     m_game_update_func;
     Game_Render_Func     m_game_render_func;
};

#endif

