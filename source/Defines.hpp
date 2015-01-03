#ifndef BRYTE_DEFINES_HPP
#define BRYTE_DEFINES_HPP

#include "Types.hpp"

namespace bryte {

     static const Real32 pixels_per_meter = 10.0f;

     inline Int32 meters_to_pixels ( Real32 meters )
     {
          return static_cast<Int32>( meters * pixels_per_meter );
     }

     inline Real32 pixels_to_meters ( Int32 pixels )
     {
          return static_cast<Real32>( pixels ) / pixels_per_meter;
     }
}

#endif

