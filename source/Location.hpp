#ifndef BRYTE_LOCATION_HPP
#define BRYTE_LOCATION_HPP

#include "Types.hpp"

struct Location {
public:

     inline Location ( );
     inline Location ( Uint8 x, Uint8 y );
     inline Location ( Int32 x, Int32 y );

     inline Bool operator== ( const Location& loc ) const;
     inline Bool operator!= ( const Location& loc ) const;

public:

     Int32 x;
     Int32 y;
};

inline Location::Location ( ) :
     x ( 0 ),
     y ( 0 )
{

}

inline Location::Location ( Uint8 x, Uint8 y ) :
     x ( x ),
     y ( y )
{

}

inline Location::Location ( Int32 x, Int32 y ) :
     x ( x ),
     y ( y )
{

}

inline Bool Location::operator== ( const Location& loc ) const
{
     return x == loc.x && y == loc.y;
}

inline Bool Location::operator!= ( const Location& loc ) const
{
     return x != loc.x || y != loc.y;
}

#endif

