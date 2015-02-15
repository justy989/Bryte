#ifndef BRYTE_ANIMATION_HPP
#define BRYTE_ANIMATION_HPP

#include "Types.hpp"

struct Animation {
     Int32 frame;
     Int32 delay_tracker;

     inline Void clear ( );

     inline Void update_increment ( Int32 delay );
     inline Void update_increment ( Int32 delay, Int32 max_frame );
};

inline Void Animation::clear ( )
{
     frame = 0;
     delay_tracker = 0;
}

inline Void Animation::update_increment ( Int32 delay )
{
     delay_tracker--;

     if ( delay_tracker <= 0 ) {
          frame++;
          delay_tracker = delay;
     }
}

inline Void Animation::update_increment ( Int32 delay, Int32 max_frame )
{
     delay_tracker--;

     if ( delay_tracker <= 0 ) {
          frame++;
          frame %= max_frame;
          delay_tracker = delay;
     }
}

#endif

