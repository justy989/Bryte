#include "Log.hpp"

#include <cstdio>
#include <cstdarg>
#include <ctime>
#include <cstring>

#ifdef LINUX

// because I don't know how to pass ... to another func
#define LOG_FUNC_BODY( log_level )\
     va_list arg;\
     Char8 timestamp [ timestamp_max_characters ];\
     make_timestamp ( timestamp );\
     printf ( "%s %s ", timestamp, log_level );\
     va_start( arg, format );\
     vfprintf( stdout, format, arg );\
     va_end( arg );

static const Uint32 timestamp_max_characters = 32;

Void Log::info ( const Char8* format, ... )
{
    LOG_FUNC_BODY ( "INFO:   " );
}

Void Log::warning ( const Char8* format, ... )
{
     LOG_FUNC_BODY ( "WARNING:" );
}

Void Log::error ( const Char8* format, ... )
{
     LOG_FUNC_BODY ( "ERROR:  " );
}

Void Log::debug ( const Char8* format, ... )
{
     LOG_FUNC_BODY ( "DEBUG:  " );
}

Void Log::make_timestamp ( Char8* destination )
{
    time_t raw_time; // time since epoch in seconds

    // grab the raw time and local time
    time ( &raw_time );

    struct tm* time_info = nullptr; // local Time

    time_info = localtime ( &raw_time );

    // format the time string, start after day of the week
    strncpy ( destination, asctime ( time_info ) + 11,
              8 );

    // cutoff formatted time string to not show year
    destination [ 8 ] = '\0';
}

#endif
