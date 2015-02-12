#ifndef BRYTE_PICKUP_DISPLAY
#define BRYTE_PICKUP_DISPLAY

#include "Pickup.hpp"
#include "Animation.hpp"

#include <SDL2/SDL.h>

namespace bryte
{
     class PickupDisplay {
     public:

          Void tick ( );

          Void render ( SDL_Surface* back_buffer, const Pickup& pickup,
                        Real32 camera_x, Real32 camera_y );

     public:

          static const Int32 c_pickup_animation_delay = 10;
          static const Int32 c_pickup_animation_max_frame = 4;

     public:

          SDL_Surface* pickup_sheet;

          Animation animation;

     };
}

#endif

