#ifndef BRYTE_INTERACTIVE_DISPLAY_HPP
#define BRYTE_INTERACTIVE_DISPLAY_HPP

#include "Interactives.hpp"

namespace bryte
{
     class Map;

     struct InteractivesDisplay {
     public:

          InteractivesDisplay ( );

          Void render ( SDL_Surface* back_buffer, Interactives& interactives,
                        Real32 camera_x, Real32 camera_y );

          Void render_interactive ( SDL_Surface* back_buffer, Interactive& interactive,
                                    Int32 position_x, Int32 position_y,
                                    Real32 camera_x, Real32 camera_y );

          SDL_Surface* interactive_sheet;
     };
}

#endif

