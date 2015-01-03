#ifndef BRYTE_UTILS_HPP
#define BRYTE_UTILS_HPP

#include "Log.hpp"

#define KILOBYTES( bytes ) bytes * 1024
#define MEGABYTES( bytes ) KILOBYTES( bytes ) * 1024
#define GIGABYTES( bytes ) MEGABYTES( bytes ) * 1024

#ifdef DEBUG
     #define ASSERT( condition )\
     if ( !( condition ) ) {\
          LOG_ERROR ( "Assert failed %s:%d ( %s )\n", __FILE__, __LINE__, #condition );\
          *reinterpret_cast<char*>(0) = 0;\
     }
#else
     #define ASSERT( condition )
#endif

#define CLAMP( value, min, max ) if ( value < min ) { value = min; } else if ( value > max ) { value = max; }

#endif

