/* point: Hold information about a 2d point and support operations around
 *        moving it, setting it and checking the distance between points.
 */

#ifndef BRYTE_POINT_HPP
#define BRYTE_POINT_HPP

#include <cmath>

namespace bryte
{
     template < typename T >
     class point {
     public:

          inline point ( T x = 0, T y = 0 );

          inline void operator+= ( const point& p );
          inline void operator-= ( const point& p );

          inline point operator+ ( const point& p ) const;
          inline point operator- ( const point& p ) const;

          inline bool operator== ( const point& p ) const;

          inline void move ( T dx, T dy );
          inline void move_x ( T dx );
          inline void move_y ( T dy );

          inline void set ( T x, T y );
          inline void set_x ( T x );
          inline void set_y ( T y );

          inline void negate ( );

          inline T distance_to ( const point& p ) const;
          inline T manhattan_distance_to ( const point& p ) const;

          inline T x ( ) const;
          inline T y ( ) const;

     private:
          T m_x;
          T m_y;
     };

     template < typename T >
     inline point<T>::point ( T x = 0, T y = 0 ) :
          m_x ( x ),
          m_y ( y )
     {

     }

     template < typename T >
     inline void point<T>::operator+= ( const point& p )
     {
          m_x += p.m_x;
          m_y += p.m_y;
     }

     template < typename T >
     inline void point<T>::operator-= ( const point& p )
     {
          m_x -= p.m_x;
          m_y -= p.m_y;
     }

     template < typename T >
     inline point<T> point<T>::operator+ ( const point& p ) const
     {
          return point<T> ( m_x + p.m_x, m_y + p.m_y );
     }

     template < typename T >
     inline point<T> point<T>::operator- ( const point& p ) const
     {
          return point<T> ( m_x - p.m_x, m_y - p.m_y );
     }

     template < typename T >
     inline bool point<T>::operator== ( const point& p ) const
     {
          return ( m_x == p.m_x && m_y == p.m_y );
     }

     template < typename T >
     inline void point<T>::move ( T dx, T dy )
     {
          m_x += dx;
          m_y += dy;
     }

     template < typename T >
     inline void point<T>::move_x ( T dx )
     {
          m_x += dx;
     }

     template < typename T >
     inline void point<T>::move_y ( T dy )
     {
          m_y += dy;
     }

     template < typename T >
     inline void point<T>::set ( T x, T y )
     {
          m_x = x;
          m_y = y;
     }

     template < typename T >
     inline void point<T>::set_x ( T x )
     {
          m_x = x;
     }

     template < typename T >
     inline void point<T>::set_y ( T y )
     {
          m_y = y;
     }

     template < typename T >
     inline void point<T>::negate ( )
     {
          m_x = -m_x;
          m_y = -m_y;
     }

     template < typename T >
     inline T point<T>::distance_to ( const point& p ) const
     {
          T dx = abs ( m_x - p.m_x );
          T dy = abs ( m_y - p.m_y );

          return static_cast<T>( sqrt ( dx * dx + dy * dy ) );
     }

     template < typename T >
     inline T point<T>::manhattan_distance_to ( const point& p ) const
     {
          T dx = abs ( m_x - p.m_x );
          T dy = abs ( m_y - p.m_y );

          return dx + dy;
     }

     template < typename T >
     inline T point<T>::x ( ) const
     {
          return m_x;
     }

     template < typename T >
     inline T point<T>::y ( ) const
     {
          return m_y;
     }
}

#endif
