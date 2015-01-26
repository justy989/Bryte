#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include "InputRecorder.hpp"
#include "GameMemory.hpp"
#include "GameFunction.hpp"

#include <SDL2/SDL.h>

#include <chrono>
#include <fstream>

#define PRINT_SDL_ERROR(sdl_api) LOG_ERROR ( "%s() failed: %s\n", sdl_api, SDL_GetError ( ) );

using std::chrono::high_resolution_clock;

// Create's a platform application to run the game code
class Application {
public:

     struct Settings {
          const Char8* window_title;
          Int32        window_width;
          Int32        window_height;

          Int32        back_buffer_height;
          Int32        back_buffer_width;

          const Char8* shared_library_path;

          Uint32       game_memory_allocation_size;

          Uint32       locked_frames_per_second;
     };

     Application ( );
     ~Application ( );

     Bool run_game ( const Settings& settings, void* game_settings );

private:

     struct KeyChange {
          SDL_Scancode scan_code;
          bool down;
     };

private:

     Bool create_window        ( const Char8* window_title, Int32 window_width, Int32 window_height,
                                 Int32 back_buffer_width, Int32 back_buffer_height );
     Bool allocate_game_memory ( Uint32 size );

     Bool save_game_memory      ( const Char8* save_path );
     Bool load_game_memory      ( const Char8* save_path );

     Real32 time_and_limit_loop ( Int32 locked_frames_per_second );
     Bool   poll_sdl_events     ( );
     Void   handle_input        ( );
     Void   clear_back_buffer   ( );
     Void   render_to_window    ( );

     Int32 window_to_back_buffer ( Int32 pos, Int32 dimension, Int32 back_buffer_dimension );
     Void translate_window_pos_to_back_buffer ( Int32 sx, Int32 sy, Int32* bx, Int32* by );

private:

     static const Int32  c_func_count = 5;
     static const Char8* c_game_func_strs [ c_func_count ];

     static const Uint32 c_max_key_changes_per_frame = 8;

private:

     // SDL components required to make window and draw to it
     SDL_Window*   m_window;
     SDL_Renderer* m_renderer;
     SDL_Texture*  m_back_buffer_texture;
     SDL_Surface*  m_back_buffer_surface;

     GameFunctions m_game_functions;

     GameMemory    m_game_memory;
     GameInput     m_game_input;

     InputRecorder m_input_recorder;

     Settings      m_settings;

     // frame timestamps
     high_resolution_clock::time_point m_previous_update_timestamp;
     high_resolution_clock::time_point m_current_update_timestamp;
};

#endif

