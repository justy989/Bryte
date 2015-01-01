#include "Platform.hpp"

#include <cstdio>
#include <dlfcn.h>
#include <cassert>

Platform::Platform ( ) :
     m_window               ( nullptr ),
     m_renderer             ( nullptr ),
     m_back_buffer_texture  ( nullptr ),
     m_back_buffer_surface  ( nullptr ),
     m_game_init_func       ( nullptr ),
     m_game_destroy_func    ( nullptr ),
     m_game_user_input_func ( nullptr ),
     m_game_update_func     ( nullptr ),
     m_game_render_func     ( nullptr )
{
     SDL_Init ( SDL_INIT_VIDEO );
}

Platform::~Platform ( )
{
     if ( m_back_buffer_surface ) {
          SDL_FreeSurface ( m_back_buffer_surface );
     }

     if ( m_back_buffer_texture ) {
          SDL_DestroyTexture ( m_back_buffer_texture );
     }

     if ( m_renderer ) {
          SDL_DestroyRenderer ( m_renderer );
     }

     if ( m_window ) {
          SDL_DestroyWindow ( m_window );
     }

     SDL_Quit ( );
}

Bool Platform::create_window ( const Char8* window_title, Int32 window_width, Int32 window_height,
                               Int32 back_buffer_width, Int32 back_buffer_height )
{
     // create the window with the specified parameters
     m_window = SDL_CreateWindow ( window_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   window_width, window_height, 0 );

     if ( !m_window ) {
          PRINT_SDL_ERROR ( "SDL_CreateWindow" );
          return false;
     }

     // create the renderer for the window
     m_renderer = SDL_CreateRenderer ( m_window, -1, SDL_RENDERER_ACCELERATED );

     if ( !m_renderer ) {
          PRINT_SDL_ERROR ( "SDL_CreateRenderer" );
          return false;
     }

     // attempt to create a surface to draw on
     m_back_buffer_surface = SDL_CreateRGBSurface ( 0, back_buffer_width, back_buffer_height, 32,
                                                    0, 0, 0, 0 );

     if ( !m_back_buffer_surface ) {
          PRINT_SDL_ERROR ( "SDL_CreateRGBSurface" );
          return false;
     }

     // create a texture based on that surface
     m_back_buffer_texture = SDL_CreateTextureFromSurface ( m_renderer, m_back_buffer_surface );

     if ( !m_back_buffer_texture ) {
          PRINT_SDL_ERROR ( "SDL_CreateTextureFromSurface" );
          return false;
     }

     return true;
}

Bool Platform::load_game_code ( const Char8* shared_library_path )
{
     m_shared_library_handle = dlopen ( shared_library_path, RTLD_LAZY );

     if ( !m_shared_library_handle ) {
          PRINT_DL_ERROR ( "dlopen" );
          return false;
     }

     static const int c_func_count = 5;

     static const char* game_func_strs [ c_func_count ] = {
          "bryte_init",
          "bryte_destroy",
          "bryte_user_input",
          "bryte_update",
          "bryte_render"
     };

     void* game_funcs [ c_func_count ] = {
          nullptr,
          nullptr,
          nullptr,
          nullptr,
          nullptr
     };

     // load each func and validate they succeeded in loading
     for ( int i = 0; i < c_func_count; ++i ) {
          game_funcs [ i ] = dlsym ( m_shared_library_handle, game_func_strs [ i ]);

          if ( !game_funcs [ i ] ) {
               printf ( "Failed to load: %s\n", game_func_strs [ i ] );
               PRINT_DL_ERROR ( "dlsym" );
               return false;
          }
     }

     // set the loaded functions
     m_game_init_func       = reinterpret_cast<Game_Init_Func>( game_funcs [ 0 ] );
     m_game_destroy_func    = reinterpret_cast<Game_Destroy_Func>( game_funcs [ 1 ] );
     m_game_user_input_func = reinterpret_cast<Game_User_Input_Func>( game_funcs [ 2 ] );
     m_game_update_func     = reinterpret_cast<Game_Update_Func>( game_funcs [ 3 ] );
     m_game_render_func     = reinterpret_cast<Game_Render_Func>( game_funcs [ 4 ] );

     return true;
}

Bool Platform::run_game ( )
{
     SDL_Event sdl_event  = {};
     Bool      done       = false;
     SDL_Rect  clear_rect { 0, 0, m_back_buffer_surface->w, m_back_buffer_surface->h };
     Uint32    black      = SDL_MapRGB ( m_back_buffer_surface->format, 0, 0, 0 );

     assert ( m_game_init_func );
     assert ( m_game_destroy_func );
     assert ( m_game_user_input_func );
     assert ( m_game_update_func );
     assert ( m_game_render_func );

     if ( !m_game_init_func ( ) ) {
          return false;
     }

     while ( !done ) {

          while ( SDL_PollEvent ( &sdl_event ) ) {
               if ( sdl_event.type == SDL_QUIT ) {
                    done = true;
               }
          }

          SDL_FillRect ( m_back_buffer_surface, &clear_rect, black );

          m_game_render_func ( m_back_buffer_surface );

          SDL_UpdateTexture ( m_back_buffer_texture, nullptr, m_back_buffer_surface->pixels,
                              m_back_buffer_surface->pitch );

          SDL_RenderClear ( m_renderer );
          SDL_RenderCopy ( m_renderer, m_back_buffer_texture, nullptr, nullptr );

          SDL_RenderPresent ( m_renderer );
     }

     m_game_destroy_func ( );

     return 0;
}

