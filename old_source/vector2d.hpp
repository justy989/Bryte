/* vector2d: Hold information about a 2d vector2d and support operations around
 *           moving it, setting it and checking the distance between vector2ds.
 */

#ifndef BRYTE_VECTOR2D_HPP
#define BRYTE_VECTOR2D_HPP

#include <type_traits>

#include <cmath>
#include <cstdlib>

namespace bryte
{
     template < typename T >
     class vector2d {
     public:

          inline vector2d ( T x = 0, T y = 0 );

          inline void operator+= ( const vector2d& p );
          inline void operator-= ( const vector2d& p );

          inline vector2d operator+ ( const vector2d& p ) const;
          inline vector2d operator- ( const vector2d& p ) const;

          inline bool operator== ( const vector2d& p ) const;

          inline void move ( T dx, T dy );
          inline void move_x ( T dx );
          inline void move_y ( T dy );

          inline void set ( T x, T y );
          inline void set ( const vector2d& p );
          inline void set_x ( T x );
          inline void set_y ( T y );

          inline void negate ( );

          inline T distance_to ( const vector2d& p ) const;
          inline T manhattan_distance_to ( const vector2d& p ) const;

          inline T x ( ) const;
          inline T y ( ) const;

     private:
          T m_x;
          T m_y;
     };

     template < typename T >
     inline vector2d<T>::vector2d ( T x, T y ) :
          m_x ( x ),
          m_y ( y )
     {

     }

     template < typename T >
     inline void vector2d<T>::operator+= ( const vector2d& p )
     {
          m_x += p.m_x;
          m_y += p.m_y;
     }

     template < typename T >
     inline void vector2d<T>::operator-= ( const vector2d& p )
     {
          m_x -= p.m_x;
          m_y -= p.m_y;
     }

     template < typename T >
     inline vector2d<T> vector2d<T>::operator+ ( const vector2d& p ) const
     {
          return vector2d<T> ( m_x + p.m_x, m_y + p.m_y );
     }

     template < typename T >
     inline vector2d<T> vector2d<T>::operator- ( const vector2d& p ) const
     {
          return vector2d<T> ( m_x - p.m_x, m_y - p.m_y );
     }

     template < typename T >
     inline bool vector2d<T>::operator== ( const vector2d& p ) const
     {
          return ( m_x == p.m_x && m_y == p.m_y );
     }

     template < typename T >
     inline void vector2d<T>::move ( T dx, T dy )
     {
          m_x += dx;
          m_y += dy;
     }

     template < typename T >
     inline void vector2d<T>::move_x ( T dx )
     {
          m_x += dx;
     }

     template < typename T >
     inline void vector2d<T>::move_y ( T dy )
     {
          m_y += dy;
     }

     template < typename T >
     inline void vector2d<T>::set ( T x, T y )
     {
          m_x = x;
          m_y = y;
     }

     template < typename T >
     inline void vector2d<T>::set ( const vector2d& p )
     {
          m_x = p.m_x;
          m_y = p.m_y;
     }

     template < typename T >
     inline void vector2d<T>::set_x ( T x )
     {
          m_x = x;
     }

     template < typename T >
     inline void vector2d<T>::set_y ( T y )
     {
          m_y = y;
     }

     template < typename T >
     inline void vector2d<T>::negate ( )
     {
          m_x = -m_x;
          m_y = -m_y;
     }

     template < typename T >
     inline T vector2d<T>::distance_to ( const vector2d& p ) const
     {
          T dx = abs ( m_x - p.m_x );
          T dy = abs ( m_y - p.m_y );

          if ( std::is_integral<T>::value ) {
               dx = static_cast<T>( abs ( static_cast<int>( dx ) ) );
               dy = static_cast<T>( abs ( static_cast<int>( dy ) ) );
          } else {
               dx = static_cast<T>( fabs ( static_cast<double>( dx ) ) );
               dy = static_cast<T>( fabs ( static_cast<double>( dy ) ) );
          }

          double py = static_cast<double>( dx * dx + dy * dy );

          return static_cast<T>( sqrt ( py ) );
     }

     template < typename T >
     inline T vector2d<T>::manhattan_distance_to ( const vector2d& p ) const
     {
          T dx = abs ( m_x - p.m_x );
          T dy = abs ( m_y - p.m_y );

          return dx + dy;
     }

     template < typename T >
     inline T vector2d<T>::x ( ) const
     {
          return m_x;
     }

     template < typename T >
     inline T vector2d<T>::y ( ) const
     {
          return m_y;
     }
}

#endif
