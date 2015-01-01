#ifndef BRYTE_PLATFORM_HPP
#define BRYTE_PLATFORM_HPP

#include <SDL2/SDL.h>

#define PRINT_SDL_ERROR(sdl_api) printf ( "%s() failed: %s\n", sdl_api, SDL_GetError ( ) );

// Game code functions
using Game_Init_Func       = bool (*)( );
using Game_Destroy_Func    = void (*)( );
using Game_User_Input_Func = void (*)( SDL_Scancode );
using Game_Update_Func     = void (*)( float );
using Game_Render_Func     = void (*)( SDL_Surface* );

// Create's a platform application to run the game code
class Platform {
public:

     Platform ( );
     ~Platform ( );

     bool create_window ( const char* window_title, int window_width, int window_height,
                          int back_buffer_width, int back_buffer_height );

     bool run_game ( );

private:

     // SDL components required to make window and draw to it
     SDL_Window*   m_window;
     SDL_Renderer* m_renderer;
     SDL_Texture*  m_back_buffer_texture;
     SDL_Surface*  m_back_buffer_surface;

     // functions loaded from game shared library
     Game_Init_Func*       m_game_init_func;
     Game_Destroy_Func*    m_game_destroy_func;
     Game_User_Input_Func* m_game_user_input_func;
     Game_Update_Func*     m_game_update_func;
     Game_Render_Func*     m_game_render_func;
};

#endif

