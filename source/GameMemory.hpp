#ifndef BRYTE_GAME_MEMORY_HPP
#define BRYTE_GAME_MEMORY_HPP

#include "Types.hpp"
#include "Utils.hpp"

#define GAME_PUSH_MEMORY(appMem, type) reinterpret_cast<type*>( appMem.push( sizeof ( type ) ) )
#define GAME_POP_MEMORY(appMem, type) appMem.pop( sizeof ( type ) )

#define GAME_PUSH_MEMORY_ARRAY(appMem, type, count) reinterpret_cast<type*>( appMem.push( sizeof ( type ) * count ) )
#define GAME_POP_MEMORY_ARRAY(appMem, type, count) appMem.pop( sizeof ( type ) * count )

class GameMemory {
public:

     inline GameMemory ( Void* location = nullptr, Uint32 size = 0 );

     // push memory segment
     inline Void* push ( Uint32 size );

     // return memory segment
     inline Void pop ( Uint32 size );

     // clear memory pointer and return it
     inline Void clear ( );

     // accessor for the location of our memory
     inline Void* location ( );

     inline Uint32 size ( ) const;

private:

     Void*  m_memory;
     Uint32 m_size;
     Uint32 m_used;
};

inline GameMemory::GameMemory ( Void* location, Uint32 size ) :
     m_memory ( location ),
     m_size ( size ),
     m_used ( 0 )
{

}

inline Void* GameMemory::push ( Uint32 size )
{
     ASSERT ( m_used + size < m_size );

     Void* ptr = reinterpret_cast<Char8*>( m_memory ) + m_used;

     m_used += size;

     return ptr;
}

inline Void GameMemory::pop ( Uint32 size )
{
     ASSERT ( size <= m_used );

     m_used -= size;
}

inline Void GameMemory::clear ( )
{
     m_memory = nullptr;
     m_size   = 0;
     m_used   = 0;
}

inline Void* GameMemory::location ( )
{
     return m_memory;
}

inline Uint32 GameMemory::size ( ) const
{
     return m_size;
}

#endif

