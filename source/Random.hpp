#ifndef BRYTE_RANDOM_HPP
#define BRYTE_RANDOM_HPP

namespace bryte
{
     class Random {
     public:

          inline Random ( );

          inline Void   seed     ( Uint32 value );
          inline Uint32 generate ( Uint32 min, Uint32 max ); // not inclusive!

     private:

          Uint32 m_i_f;
          Uint32 m_i_s;
     };

     inline Random::Random ( ) :
          m_i_f ( 0 ),
          m_i_s ( 0 )
     {

     }

     inline Void Random::seed ( Uint32 value )
     {
          m_i_f = value;
          m_i_s = value;
     }

     inline Uint32 Random::generate ( Uint32 min, Uint32 max )
     {
          if ( min == max ) {
               return min;
          }

          // simple RNG by George Marsaglia
          m_i_f = 36969 * ( m_i_f & 65535 ) + ( m_i_f >> 16 );
          m_i_s = 18000 * ( m_i_s & 65535 ) + ( m_i_s >> 16 );

          return ( ( ( m_i_f << 16 ) + m_i_s ) % ( max - min ) ) + min;
     }
}

#endif

