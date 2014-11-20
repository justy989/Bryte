/* rotation: holds orientation in increments of 90 degress
 */

#ifndef BRYTE_ROTATION_HPP
#define BRYTE_ROTATION_HPP

#include "types.hpp"

namespace bryte
{
     enum rotation : ubyte {
          zero,
          ninety,
          one_eighty,
          two_seventy,
          count
     };
}

#endif
