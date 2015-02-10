#ifndef BRYTE_INTERACTIVE_DISPLAY_HPP
#define BRYTE_INTERACTIVE_DISPLAY_HPP

#include "Interactives.hpp"

namespace bryte
{
     class Map;

     struct InteractivesDisplay {
     public:

          InteractivesDisplay ( );

          Void tick ( );

          Void render ( SDL_Surface* back_buffer, Interactives& interactives,
                        Real32 camera_x, Real32 camera_y );

          Void render_underneath ( SDL_Surface* back_buffer, UnderneathInteractive& underneath,
                                    Int32 position_x, Int32 position_y,
                                    Real32 camera_x, Real32 camera_y );

          Void render_interactive ( SDL_Surface* back_buffer, Interactive& interactive,
                                    Int32 position_x, Int32 position_y,
                                    Real32 camera_x, Real32 camera_y );

     public:

          static const Int32 c_frames_per_update = 12;

          static const Int32 c_torch_frame_count = 3;
          static const Int32 c_light_detector_frame_count = 2;

     public:

          SDL_Surface* interactive_sheet;

          Int32 frame;
          Int32 frame_update_delay;
     };
}

#endif

