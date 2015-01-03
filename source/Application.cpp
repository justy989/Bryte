#include "Application.hpp"
#include "Utils.hpp"

#include <thread>
#include <fstream>

#include <cstdio>
#include <cassert>
#include <cstring>
#include <dlfcn.h>

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::milliseconds;

const Char8* Application::c_game_func_strs [ c_func_count ] = {
     "bryte_init",
     "bryte_destroy",
     "bryte_reload_memory",
     "bryte_user_input",
     "bryte_update",
     "bryte_render"
};

static const Char8* c_game_memory_filepath  = "bryte_memory.mem";
static const Char8* c_record_input_filepath = "bryte_input.in";

Application::Application ( ) :
     m_window                ( nullptr ),
     m_renderer              ( nullptr ),
     m_back_buffer_texture   ( nullptr ),
     m_back_buffer_surface   ( nullptr ),
     m_shared_library_path   ( nullptr ),
     m_shared_library_handle ( nullptr ),
     m_game_init_func        ( nullptr ),
     m_game_destroy_func     ( nullptr ),
     m_game_user_input_func  ( nullptr ),
     m_game_update_func      ( nullptr ),
     m_game_render_func      ( nullptr ),
     m_previous_update_timestamp ( high_resolution_clock::now ( ) ),
     m_current_update_timestamp ( m_previous_update_timestamp )
{
     LOG_INFO ( "Initializing SDL\n" );
     SDL_Init ( SDL_INIT_VIDEO );
}

Application::~Application ( )
{
     if ( m_game_memory.memory ) {
          LOG_INFO ( "Freeing allocated game memory.\n" );
          free ( m_game_memory.memory );
     }

     if ( m_shared_library_handle ) {
          LOG_INFO ( "Closing shared library\n" );
          dlclose ( m_shared_library_handle );
     }

     if ( m_back_buffer_surface ) {
          LOG_INFO ( "Freeing SDL back buffer surface\n" );
          SDL_FreeSurface ( m_back_buffer_surface );
     }

     if ( m_back_buffer_texture ) {
          LOG_INFO ( "Destroying SDL back buffer texture\n" );
          SDL_DestroyTexture ( m_back_buffer_texture );
     }

     if ( m_renderer ) {
          LOG_INFO ( "Destroying SDL renderer\n" );
          SDL_DestroyRenderer ( m_renderer );
     }

     if ( m_window ) {
          LOG_INFO ( "Destroying SDL window\n" );
          SDL_DestroyWindow ( m_window );
     }

     LOG_INFO ( "Quitting SDL\n" );
     SDL_Quit ( );
}

Bool Application::create_window ( const Char8* window_title, Int32 window_width, Int32 window_height,
                                  Int32 back_buffer_width, Int32 back_buffer_height )
{

     // create the window with the specified parameters
     LOG_INFO ( "Creating SDL window: '%s' %d, %d\n", window_title, window_width, window_height );
     m_window = SDL_CreateWindow ( window_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   window_width, window_height, 0 );

     if ( !m_window ) {
          PRINT_SDL_ERROR ( "SDL_CreateWindow" );
          return false;
     }

     // create the renderer for the window
     LOG_INFO ( "Creating SDL renderer\n" );
     m_renderer = SDL_CreateRenderer ( m_window, -1, SDL_RENDERER_ACCELERATED );

     if ( !m_renderer ) {
          PRINT_SDL_ERROR ( "SDL_CreateRenderer" );
          return false;
     }

     // attempt to create a surface to draw on
     LOG_INFO ( "Creating SDL back buffer surface: %d, %d\n", back_buffer_width, back_buffer_height );
     m_back_buffer_surface = SDL_CreateRGBSurface ( 0, back_buffer_width, back_buffer_height, 32,
                                                    0, 0, 0, 0 );

     if ( !m_back_buffer_surface ) {
          PRINT_SDL_ERROR ( "SDL_CreateRGBSurface" );
          return false;
     }

     // create a texture based on that surface
     LOG_INFO ( "Creating SDL back buffer texture: %d, %d\n", back_buffer_width, back_buffer_height );
     m_back_buffer_texture = SDL_CreateTextureFromSurface ( m_renderer, m_back_buffer_surface );

     if ( !m_back_buffer_texture ) {
          PRINT_SDL_ERROR ( "SDL_CreateTextureFromSurface" );
          return false;
     }

     return true;
}

Bool Application::load_game_code ( const Char8* shared_library_path )
{
     if ( m_shared_library_handle ) {
          LOG_INFO ( "Freeing shared library handle\n" );
          dlclose ( m_shared_library_handle );
     }

     LOG_INFO ( "Loading shared library: %s\n", shared_library_path );
     m_shared_library_handle = dlopen ( shared_library_path, RTLD_LAZY );

     if ( !m_shared_library_handle ) {
          PRINT_DL_ERROR ( "dlopen" );
          return false;
     }

     Char8* save_path = strdup ( shared_library_path );

     // if we succeded, save the shared library path
     if ( m_shared_library_path ) {
          free ( m_shared_library_path );
     }

     m_shared_library_path = save_path;

     // load each func and validate they succeeded in loading
     void* game_funcs [ c_func_count ];

     for ( int i = 0; i < c_func_count; ++i ) {
          game_funcs [ i ] = dlsym ( m_shared_library_handle, c_game_func_strs [ i ]);

          if ( !game_funcs [ i ] ) {
               printf ( "Failed to load: %s\n", c_game_func_strs [ i ] );
               PRINT_DL_ERROR ( "dlsym" );
               return false;
          }
     }

     // set the loaded functions
     m_game_init_func          = reinterpret_cast<GameInitFunc>( game_funcs [ 0 ] );
     m_game_destroy_func       = reinterpret_cast<GameDestroyFunc>( game_funcs [ 1 ] );
     m_game_reload_memory_func = reinterpret_cast<GameReloadMemoryFunc>( game_funcs [ 2 ] );
     m_game_user_input_func    = reinterpret_cast<GameUserInputFunc>( game_funcs [ 3 ] );
     m_game_update_func        = reinterpret_cast<GameUpdateFunc>( game_funcs [ 4 ] );
     m_game_render_func        = reinterpret_cast<GameRenderFunc>( game_funcs [ 5 ] );

     return true;
}

Bool Application::allocate_game_memory ( )
{
     if ( m_game_memory.memory ) {
          LOG_INFO ( "Freeing allocated game memory.\n" );
          free ( m_game_memory.memory );
     }

     LOG_INFO ( "Allocating game memory: %d bytes\n", m_game_memory.size );
     m_game_memory.memory = malloc ( m_game_memory.size );

     if ( !m_game_memory.memory ) {
          printf ( "Failed to Allocate %d memory for game.\n", m_game_memory.size );
          return false;
     }

     return true;
}

Bool Application::save_game_memory ( const Char8* path )
{
     LOG_INFO ( "Saving game memory to '%s'\n", path );

     std::ofstream file ( path, std::ios::binary );

     if ( !file.is_open ( ) ) {
          LOG_ERROR ( "Failed to open '%s' to save game memory.\n", path );
          return false;
     }

     file.write ( reinterpret_cast<char*>( m_game_memory.memory ), m_game_memory.size );

     if ( !file ) {
          LOG_ERROR ( "Failed to write %d bytes to '%s' to save game memory.\n",
                      path, m_game_memory.size );
     }

     file.close ( );
     return true;
}

Bool Application::load_game_memory ( const Char8* path )
{
     LOG_INFO ( "Loading game memory from '%s'\n", path );

     std::ifstream file ( path, std::ios::binary );

     if ( !file.is_open ( ) ) {
          LOG_ERROR ( "Failed to open '%s' to load game memory.\n", path );
          return false;
     }

     file.read ( reinterpret_cast<char*>( m_game_memory.memory ), m_game_memory.size );

     if ( !file ) {
          LOG_ERROR ( "Failed to read %d bytes from '%s' to load game memory.\n",
                      path, m_game_memory.size );
     }

     file.close ( );
     return true;
}

Void Application::handle_input ( )
{
     if ( m_input_recorder.is_recording ( ) ) {
          m_input_recorder.write_frame ( m_game_input );
     }

     if ( m_input_recorder.is_playing_back ( ) ) {
          if ( !m_input_recorder.read_frame ( m_game_input ) ) {
               load_game_memory ( c_game_memory_filepath );
          }
     }

     m_game_user_input_func ( m_game_input );
}

Void Application::clear_back_buffer ( )
{
     Uint32    black      = SDL_MapRGB ( m_back_buffer_surface->format, 0, 0, 0 );
     SDL_Rect  clear_rect { 0, 0, m_back_buffer_surface->w, m_back_buffer_surface->h };

     SDL_FillRect ( m_back_buffer_surface, &clear_rect, black );
}

Void Application::render_to_window ( )
{
     SDL_UpdateTexture ( m_back_buffer_texture, nullptr, m_back_buffer_surface->pixels,
                         m_back_buffer_surface->pitch );

     SDL_RenderClear ( m_renderer );
     SDL_RenderCopy ( m_renderer, m_back_buffer_texture, nullptr, nullptr );

     SDL_RenderPresent ( m_renderer );
}

Bool Application::poll_sdl_events ( )
{
     SDL_Event sdl_event = {};

     m_game_input.reset ( );

     while ( SDL_PollEvent ( &sdl_event ) ) {
          if ( sdl_event.type == SDL_QUIT ) {
               return false;
          }

          if ( sdl_event.type == SDL_KEYDOWN ) {
               auto sc = sdl_event.key.keysym.scancode;

               if ( sc == SDL_SCANCODE_ESCAPE ) {
                    return false;
               }

               if ( sc == SDL_SCANCODE_0 ) {
                    load_game_code ( m_shared_library_path );
                    m_game_reload_memory_func ( m_game_memory );
                    continue;
               }

               if ( sc == SDL_SCANCODE_1 ) {
                    m_input_recorder.start_recording ( c_record_input_filepath );
                    save_game_memory ( c_game_memory_filepath );
                    continue;
               }

               if ( sc == SDL_SCANCODE_2 ) {
                    m_input_recorder.stop_recording ( );
                    load_game_memory ( c_game_memory_filepath );
                    m_input_recorder.start_playing_back ( c_record_input_filepath );
                    continue;
               }

               if ( sc == SDL_SCANCODE_3 ) {
                    m_input_recorder.stop_playing_back ( );
                    continue;
               }

               if ( !m_game_input.add_key_change ( sc, true ) ) {
                    LOG_WARNING ( "Unable to handle more than %d keys per frame\n",
                                  GameInput::c_max_key_change_count );
               }

          } else if ( sdl_event.type == SDL_KEYUP ) {
               auto sc = sdl_event.key.keysym.scancode;

               if ( !m_game_input.add_key_change ( sc, false ) ) {
                    LOG_WARNING ( "Unable to handle more than %d keys per frame\n",
                                  GameInput::c_max_key_change_count );
               }
          }
     }

     return true;
}

Real32 Application::time_and_limit_loop ( Int32 locked_frames_per_second )
{
     m_previous_update_timestamp = m_current_update_timestamp;
     m_current_update_timestamp  = high_resolution_clock::now ( );

     auto duration = m_current_update_timestamp - m_previous_update_timestamp;
     auto dt_ms = duration_cast<milliseconds>( duration ).count ( );

     auto max_allowed_milliseconds = 1000 / locked_frames_per_second;

     if ( dt_ms < max_allowed_milliseconds ) {
          auto time_until_limit = max_allowed_milliseconds - dt_ms;
          std::this_thread::sleep_for ( milliseconds ( time_until_limit ) );
          dt_ms += time_until_limit;
     } else if ( dt_ms > max_allowed_milliseconds * 2 ) {
          // log a warning if we take much too long on a frame
          LOG_WARNING ( "game loop executed in %d milliseconds\n", dt_ms );
     }

     return static_cast<float>( dt_ms ) / 1000.0f;
}

Bool Application::run_game ( const Settings& settings )
{
     if ( !create_window ( settings.window_title, settings.window_width, settings.window_height,
                           settings.back_buffer_width, settings.back_buffer_height ) ) {
          return false;
     }

     if ( !load_game_code ( settings.shared_library_path ) ) {
          return false;
     }

     m_game_memory.size = settings.game_memory_allocation_size;

     if ( !allocate_game_memory ( ) ) {
          return false;
     }

     Real32 time_delta = 0.0f;

     ASSERT ( m_game_init_func );
     ASSERT ( m_game_destroy_func );
     ASSERT ( m_game_reload_memory_func );
     ASSERT ( m_game_user_input_func );
     ASSERT ( m_game_update_func );
     ASSERT ( m_game_render_func );

     if ( !m_game_init_func ( m_game_memory) ) {
          return false;
     }

     while ( true ) {

          time_delta = time_and_limit_loop ( settings.locked_frames_per_second );

          if ( !poll_sdl_events ( ) ) {
               break;
          }

          handle_input ( );

          m_game_update_func ( time_delta );

          clear_back_buffer ( );
          m_game_render_func ( m_back_buffer_surface );
          render_to_window ( );
     }

     m_game_destroy_func ( );

     return 0;
}

