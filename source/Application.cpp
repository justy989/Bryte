#include "Application.hpp"
#include "Utils.hpp"

#include <thread>
#include <fstream>

#include <cstdio>
#include <cassert>
#include <cstring>

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::milliseconds;

static const Char8* c_game_memory_filepath  = "bryte_memory.mem";
static const Char8* c_record_input_filepath = "bryte_input.in";

Application::Application ( ) :
     m_window                ( nullptr ),
     m_renderer              ( nullptr ),
     m_back_buffer_texture   ( nullptr ),
     m_back_buffer_surface   ( nullptr ),
     m_previous_update_timestamp ( high_resolution_clock::now ( ) ),
     m_current_update_timestamp ( m_previous_update_timestamp )
{
     LOG_INFO ( "Initializing SDL\n" );
     SDL_Init ( SDL_INIT_VIDEO );
}

Application::~Application ( )
{
     if ( m_game_memory.location ( ) ) {
          LOG_INFO ( "Freeing allocated game memory.\n" );
          free ( m_game_memory.location ( ) );
          m_game_memory.clear ( );
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
                                   window_width, window_height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE );

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

Bool Application::allocate_game_memory ( Uint32 size )
{
     if ( m_game_memory.location ( ) ) {
          LOG_INFO ( "Freeing allocated game memory.\n" );
          free ( m_game_memory.location ( ) );
          m_game_memory.clear ( );
     }

     LOG_INFO ( "Allocating game memory: %d bytes\n", size );
     Void* memory = malloc ( size );

     if ( !memory ) {
          LOG_ERROR ( "Allocation of %u bytes failed, malloc() returned NULL.\n", size );
          return false;
     }

     m_game_memory = GameMemory ( memory, size );

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

     file.write ( reinterpret_cast<Char8*>( m_game_memory.location ( ) ), m_game_memory.size ( ) );

     if ( !file ) {
          LOG_ERROR ( "Failed to write %u bytes to '%s' to save game memory.\n",
                      m_game_memory.size ( ), path );
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

     file.read ( reinterpret_cast<Char8*>( m_game_memory.location ( ) ), m_game_memory.size ( ) );

     if ( !file ) {
          LOG_ERROR ( "Failed to read %u bytes from '%s' to load game memory.\n",
                      m_game_memory.size ( ), path );
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

     m_game_functions.game_user_input_func ( m_game_memory, m_game_input );
}

Void Application::clear_back_buffer ( )
{
     Uint32    clear_color = SDL_MapRGB ( m_back_buffer_surface->format, 86, 156, 214 );
     SDL_Rect  clear_rect  { 0, 0, m_back_buffer_surface->w, m_back_buffer_surface->h };

     SDL_FillRect ( m_back_buffer_surface, &clear_rect, clear_color );
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
                    LOG_INFO ( "Handling quit event\n" );
                    return false;
               }

               if ( sc == SDL_SCANCODE_0 ) {
                    m_game_functions.load ( m_settings.shared_library_path );
                    continue;
               }

               if ( sc == SDL_SCANCODE_1 ) {
                    if ( !m_input_recorder.is_recording ( ) &&
                         !m_input_recorder.is_playing_back ( ) ) {
                         m_input_recorder.start_recording ( c_record_input_filepath );
                         save_game_memory ( c_game_memory_filepath );
                         continue;
                    }
               }

               if ( sc == SDL_SCANCODE_2 ) {
                    if ( m_input_recorder.is_recording ( ) ) {
                         m_input_recorder.stop_recording ( );
                         load_game_memory ( c_game_memory_filepath );
                         m_input_recorder.start_playing_back ( c_record_input_filepath );
                         continue;
                    }
               }

               if ( sc == SDL_SCANCODE_3 ) {
                    if ( m_input_recorder.is_playing_back ( ) ) {
                         m_input_recorder.stop_playing_back ( );
                         continue;
                    }
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
          } else if ( sdl_event.type == SDL_MOUSEBUTTONDOWN ) {
               auto button = sdl_event.button;
               Int32 x, y;

               translate_window_pos_to_back_buffer ( button.x, button.y, &x, &y );

               if ( !m_game_input.add_mouse_button_change ( button.button, true, x, y ) ) {
                    LOG_WARNING ( "Unable to handle more than %d mouse button clicks per frame\n",
                                  GameInput::c_max_mouse_button_change_count );
               }
          } else if ( sdl_event.type == SDL_MOUSEBUTTONUP ) {
               auto button = sdl_event.button;
               Int32 x, y;

               translate_window_pos_to_back_buffer ( button.x, button.y, &x, &y );

               if ( !m_game_input.add_mouse_button_change ( button.button, false, x, y ) ) {
                    LOG_WARNING ( "Unable to handle more than %d mouse button clicks per frame\n",
                                  GameInput::c_max_mouse_button_change_count );
               }
          } else if ( sdl_event.type == SDL_MOUSEMOTION ) {
               auto button = sdl_event.button;
               translate_window_pos_to_back_buffer ( button.x, button.y,
                                                     &m_game_input.mouse_position_x,
                                                     &m_game_input.mouse_position_y );
          } else if ( sdl_event.type == SDL_MOUSEWHEEL ) {
               m_game_input.mouse_scroll = sdl_event.wheel.y;
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

Bool Application::run_game ( const Settings& settings, void* game_settings )
{
    m_settings = settings;

     if ( !create_window ( settings.window_title, settings.window_width, settings.window_height,
                           settings.back_buffer_width, settings.back_buffer_height ) ) {
          return false;
     }

     if ( !m_game_functions.load ( settings.shared_library_path ) ) {
          return false;
     }

     if ( !allocate_game_memory ( settings.game_memory_allocation_size ) ) {
          return false;
     }

     Real32 time_delta = 0.0f;

     LOG_INFO ( "Initializing game\n" );
     if ( !m_game_functions.game_init_func ( m_game_memory, game_settings ) ) {
          return false;
     }

     LOG_INFO ( "Starting game loop\n" );
     m_current_update_timestamp = high_resolution_clock::now ( );

     while ( true ) {

          time_delta = time_and_limit_loop ( settings.locked_frames_per_second );

          if ( !poll_sdl_events ( ) ) {
               break;
          }

          handle_input ( );

          m_game_functions.game_update_func ( m_game_memory, time_delta );

          clear_back_buffer ( );
          m_game_functions.game_render_func ( m_game_memory, m_back_buffer_surface );
          render_to_window ( );
     }

     LOG_INFO ( "Destroying game\n" );
     m_game_functions.game_destroy_func ( m_game_memory );

     return 0;
}

Int32 Application::window_to_back_buffer ( Int32 pos, Int32 dimension, Int32 back_buffer_dimension )
{
     float pct = static_cast<float>( pos ) / static_cast<float>( dimension );

     return static_cast<Int32>( static_cast<float>( back_buffer_dimension ) * pct );
}

Void Application::translate_window_pos_to_back_buffer ( Int32 sx, Int32 sy, Int32* bx, Int32* by )
{
     Int32 w, h;

     SDL_GetWindowSize ( m_window, &w, &h );

     *bx = window_to_back_buffer ( sx, w, m_back_buffer_surface->w );
     *by = m_back_buffer_surface->h - window_to_back_buffer ( sy, h, m_back_buffer_surface->h );
}

