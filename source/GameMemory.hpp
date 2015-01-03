#ifndef BRYTE_GAME_MEMORY_HPP
#define BRYTE_GAME_MEMORY_HPP

#include "Types.hpp"

struct GameMemory {

     GameMemory ( ) : memory ( nullptr ), size ( 0 ), used ( 0 ) { }

     Void*  memory;
     Uint32 size;
     Uint32 used;

     template < typename T >
     T* push_array ( Uint8 count );

     template < typename T >
     T* push_object ( );
};

template < typename T >
T* GameMemory::push_array ( Uint8 count )
{
     auto* ptr = reinterpret_cast<Char8*>( memory ) + used;

     used += sizeof ( T ) * count;

     return reinterpret_cast<T*>( ptr );
}

template < typename T >
T* GameMemory::push_object ( )
{
     return push_array<T>( 1 );
}

#endif

