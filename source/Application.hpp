#ifndef BRYTE_PLATFORM_HPP
#define BRYTE_PLATFORM_HPP

#include "Bryte.hpp"

#include <SDL2/SDL.h>

#include <chrono>
#include <fstream>

#define PRINT_SDL_ERROR(sdl_api) LOG_ERROR ( "%s() failed: %s\n", sdl_api, SDL_GetError ( ) );
#define PRINT_DL_ERROR(dl_api) LOG_ERROR ( "%s() failed: %s\n", dl_api, dlerror ( ) );

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

     Bool run_game ( const Settings& settings );

private:

     struct KeyChange {
          SDL_Scancode scan_code;
          bool down;
     };

private:

     Bool create_window        ( const Char8* window_title, Int32 window_width, Int32 window_height,
                                 Int32 back_buffer_width, Int32 back_buffer_height );
     Bool load_game_code       ( const Char8* shared_library_path );
     Bool allocate_game_memory ( );

     Bool save_game_memory      ( const Char8* save_path );
     Bool load_game_memory      ( const Char8* save_path );

     Bool start_recording_input    ( const Char8* path );
     Bool stop_recording_input     ( );
     Bool start_playing_back_input ( const Char8* path );
     Bool stop_playing_back_input  ( );

     Real32 time_and_limit_loop ( Int32 locked_frames_per_second );
     Bool   poll_sdl_events     ( );
     Void   handle_input        ( );
     Void   clear_back_buffer   ( );
     Void   render_to_window    ( );

private:

     static const Int32  c_func_count = 6;
     static const Char8* c_game_func_strs [ c_func_count ];

     static const Uint32 c_max_key_changes_per_frame = 8;

private:

     // SDL components required to make window and draw to it
     SDL_Window*   m_window;
     SDL_Renderer* m_renderer;
     SDL_Texture*  m_back_buffer_texture;
     SDL_Surface*  m_back_buffer_surface;

     // loaded share library attributes
     Char8* m_shared_library_path;
     void*  m_shared_library_handle;

     // functions loaded from game shared library
     GameInitFunc         m_game_init_func;
     GameDestroyFunc      m_game_destroy_func;
     GameReloadMemoryFunc m_game_reload_memory_func;
     GameUserInputFunc    m_game_user_input_func;
     GameUpdateFunc       m_game_update_func;
     GameRenderFunc       m_game_render_func;

     GameMemory           m_game_memory;

     // timer timestamps
     std::chrono::high_resolution_clock::time_point m_previous_update_timestamp;
     std::chrono::high_resolution_clock::time_point m_current_update_timestamp;

     // input recorder file handles
     std::ifstream m_input_record_reader_file;
     std::ofstream m_input_record_writer_file;

     KeyChange m_key_changes [ c_max_key_changes_per_frame ];
     Uint32    m_key_change_count;

     Bool m_recording_input;
     Bool m_playing_back_input;
};

#endif

