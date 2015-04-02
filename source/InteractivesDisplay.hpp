#ifndef BRYTE_INTERACTIVE_DISPLAY_HPP
#define BRYTE_INTERACTIVE_DISPLAY_HPP

#include "Interactives.hpp"
#include "Animation.hpp"

class GameMemory;

namespace bryte
{
     class Map;

     struct InteractivesDisplay {
     public:

          Bool load_surfaces ( GameMemory& game_memory, const Char8* exitsheet_filepath );
          Void unload_surfaces ( );

          Void clear ( );

          Void tick ( );

          Void render ( SDL_Surface* back_buffer, Interactives& interactives,
                        const Map& map, Real32 camera_x, Real32 camera_y, Bool invisible );

          Void render_underneath ( SDL_Surface* back_buffer, UnderneathInteractive& underneath,
                                   SDL_Rect* dest_rect );

          Void render_interactive ( SDL_Surface* back_buffer, Interactive& interactive,
                                    SDL_Rect* dest_rect );

     public:

          static const Int32 c_frames_per_update = 12;

          static const Int32 c_torch_frame_count = 3;
          static const Int32 c_light_detector_frame_count = 2;

          static const Int32 c_ice_frame_delay = 10;
          static const Int32 c_ice_frame_count = 4;
          static const Int32 c_ice_frame_sleep = 60;

          static const Int32 c_moving_walkway_frame_delay = 15;
          static const Int32 c_moving_walkway_frame_count = 2;

     public:

          SDL_Surface* interactive_sheet;
          SDL_Surface* moving_walkway_sheet;
          SDL_Surface* torch_element_sheet;
          SDL_Surface* exit_sheet;
          SDL_Surface* light_detector_sheet;
          SDL_Surface* ice_detector_sheet;
          SDL_Surface* portal_sheet;

          Animation animation;
          Animation ice_animation;
          Animation moving_walkway_animation;

          Int32 ice_sleep_counter;
     };
}

#endif

