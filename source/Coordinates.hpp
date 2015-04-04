#ifndef BRYTE_COORDINATES_HPP
#define BRYTE_COORDINATES_HPP

#include "Types.hpp"

struct Coordinates {
public:

     inline Coordinates ( );
     inline Coordinates ( Uint8 x, Uint8 y );

     inline Bool operator== ( const Coordinates& coords ) const;
     inline Bool operator!= ( const Coordinates& coords ) const;

public:

     Uint8 x;
     Uint8 y;
};

inline Coordinates::Coordinates ( ) :
     x ( 0 ),
     y ( 0 )
{

}

inline Coordinates::Coordinates ( Uint8 x, Uint8 y ) :
     x ( x ),
     y ( y )
{

}

inline Bool Coordinates::operator== ( const Coordinates& coords ) const
{
     return x == coords.x && y == coords.y;
}

inline Bool Coordinates::operator!= ( const Coordinates& coords ) const
{
     return x != coords.x || y != coords.y;
}

#endif

