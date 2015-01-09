#ifndef BRYTE_CAMERA_HPP
#define BRYTE_CAMERA_HPP

#include "Types.hpp"

namespace bryte
{
     extern "C" Real32 calculate_camera_position ( Int32 back_buffer_dimension, Int32 map_dimension,
                                                   Real32 player_position, Real32 player_dimension );
}

#endif

