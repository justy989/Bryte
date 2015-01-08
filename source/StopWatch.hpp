#ifndef BRYTE_STOPWATCH_HPP
#define BRYTE_STOPWATCH_HPP

namespace bryte
{
     class Stopwatch {
     public:

          inline Void reset ( Real32 remaining );

          inline Void tick ( Real32 delta );

          inline Bool expired ( ) const;

     private:

          Real32 m_remaining;
     };

     inline Void Stopwatch::reset ( Real32 remaining )
     {
          this->m_remaining = remaining;
     }

     inline Void Stopwatch::tick ( Real32 time_delta )
     {
          m_remaining -= time_delta;

          if ( m_remaining <= 0.0f ) {
               m_remaining = 0.0f;
          }
     }

     inline Bool Stopwatch::expired ( ) const
     {
          return m_remaining <= 0.0f;
     }
}

#endif

