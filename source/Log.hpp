#ifndef BRYTE_LOG_HPP
#define BRYTE_LOG_HPP

#include "Types.hpp"

class Log {
public:

     static void info ( const Char8* format, ... );
     static void warning ( const Char8* format, ... );
     static void error ( const Char8* format, ... );
     static void debug ( const Char8* format, ... );

private:

     static void make_timestamp ( Char8* destination );

};

#define LOG_INFO( ... ) Log::info ( __VA_ARGS__ );
#define LOG_ERROR( ... ) Log::error ( __VA_ARGS__ );
#define LOG_WARNING( ... ) Log::warning ( __VA_ARGS__ );

#ifdef DEBUG
     #define LOG_DEBUG( format, ... ) Log::debug ( format, __VA_ARGS__ );
#else
     #define LOG_DEBUG( format, ... )
#endif

#endif

