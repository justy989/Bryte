#ifndef BRYTE_LOG_HPP
#define BRYTE_LOG_HPP

#include "Types.hpp"

#ifdef LINUX

class Log {
public:

     static Void info    ( const Char8* format, ... );
     static Void warning ( const Char8* format, ... );
     static Void error   ( const Char8* format, ... );
     static Void debug   ( const Char8* format, ... );

private:

     static Void make_timestamp ( Char8* destination );

};

#define LOG_INFO( ... ) Log::info ( __VA_ARGS__ );
#define LOG_ERROR( ... ) Log::error ( __VA_ARGS__ );
#define LOG_WARNING( ... ) Log::warning ( __VA_ARGS__ );

#ifdef DEBUG
     #define LOG_DEBUG( ... ) Log::debug ( __VA_ARGS__ );
#else
     #define LOG_DEBUG( ... )
#endif

#endif

#if WIN32

#include <cstdio>

#define LOG_INFO( ... ) printf ( "INFO   : " ); printf ( __VA_ARGS__ )
#define LOG_ERROR( ... ) printf ( "ERROR  : " ); printf ( __VA_ARGS__ )
#define LOG_WARNING( ... ) printf ( "WARNING: " ); printf ( __VA_ARGS__ )

#ifdef DEBUG
    #define LOG_DEBUG( ... ) printf ( "DEBUG: " ); printf ( __VA_ARGS__ )
#else
    #define LOG_DEBUG( ... )
#endif

#endif

#endif

