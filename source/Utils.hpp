#ifndef BRYTE_UTILS_HPP
#define BRYTE_UTILS_HPP

#define KILOBYTES( bytes ) bytes * 1024
#define MEGABYTES( bytes ) KILOBYTES( bytes ) * 1024
#define GIGABYTES( bytes ) MEGABYTES( bytes ) * 1024

#ifdef DEBUG
     #define ASSERT( condition ) if ( !condition ) { *reinterpret_cast<char*>(0) = 0; }
#else
     #define ASSERT( condition )
#endif

#endif

