/* rect: Hold information about a rectangle and support operations around
 *       moving it, setting it, and checking for collisions. It also validates
 *       that left is always left of right and top is always above bottom.
 */

#ifndef BRYTE_RECT_HPP
#define BRYTE_RECT_HPP

#include <cassert>

#include "vector2d.hpp"

namespace bryte
{
     template < typename T >
     class rect {
     public:

          inline rect ( const vector2d<T>& top_left,
                        const vector2d<T>& bottom_right );

          inline rect ( T left, T top, T right, T bottom );

          inline rect& operator+= ( const vector2d<T>& p );
          inline rect& operator-= ( const vector2d<T>& p );

          inline rect operator+ ( const vector2d<T>& p ) const;
          inline rect operator- ( const vector2d<T>& p ) const;

          inline bool contains ( const vector2d<T>& p ) const;
          inline bool intersects ( const rect<T>& r ) const;

          inline void move ( T dx, T dy );
          inline void move_x ( T dx );
          inline void move_y ( T dy );

          inline void set ( T left, T top, T right, T bottom );
          inline void set_left ( T left );
          inline void set_top ( T top );
          inline void set_right ( T right );
          inline void set_bottom ( T bottom );

          inline T left () const;
          inline T top () const;
          inline T right () const;
          inline T bottom () const;

          inline T width ( ) const;
          inline T height ( ) const;

          inline vector2d<T> top_left () const;
          inline vector2d<T> top_right () const;
          inline vector2d<T> bottom_left () const;
          inline vector2d<T> bottom_right () const;

     private:

          inline void validate ( );

     private:

          vector2d<T> m_top_left;
          vector2d<T> m_bottom_right;
     };

     template < typename T >
     inline rect<T>::rect ( const vector2d<T>& top_left,
                            const vector2d<T>& bottom_right ) :
          m_top_left ( top_left ),
          m_bottom_right ( bottom_right )
     {
          validate ( );
     }

     template < typename T >
     inline rect<T>::rect ( T left, T top, T right, T bottom ) :
          m_top_left ( left, top ),
          m_bottom_right ( right, bottom )
     {
          validate ( );
     }

     template < typename T >
     inline rect<T>& rect<T>::operator+= ( const vector2d<T>& p )
     {
          m_top_left += p;
          m_bottom_right += p;

          return *this;
     }

     template < typename T >
     inline rect<T>& rect<T>::operator-= ( const vector2d<T>& p )
     {
          m_top_left -= p;
          m_bottom_right -= p;

          return *this;
     }

     template < typename T >
     inline rect<T> rect<T>::operator+ ( const vector2d<T>& p ) const
     {
          return rect ( m_top_left + p, m_bottom_right + p );
     }

     template < typename T >
     inline rect<T> rect<T>::operator- ( const vector2d<T>& p ) const
     {
          return rect ( m_top_left - p, m_bottom_right - p );
     }

     template < typename T >
     inline bool rect<T>::contains ( const vector2d<T>& p ) const
     {
          return ( p.x ( ) >= m_top_left.x ( ) && p.x ( ) <= m_bottom_right.x ( ) &&
                   p.y ( ) >= m_top_left.y ( ) && p.y ( ) <= m_bottom_right.y ( ) );
     }

     template < typename T >
     inline bool rect<T>::intersects ( const rect<T>& r ) const
     {
          // check if b intersects a
          if ( contains ( r.top_left ( ) ) ||
               contains ( r.top_right ( ) ) ||
               contains ( r.bottom_left ( ) ) ||
               contains ( r.bottom_right ( ) ) ) {
               return true;
          }

          // if not, return a intersects b. a could be completely inside b
          return ( r.contains ( top_left ( ) ) ||
                   r.contains ( top_right ( ) ) ||
                   r.contains ( bottom_left ( ) ) ||
                   r.contains ( bottom_right ( ) ) );
     }

     template < typename T >
     inline void rect<T>::move ( T dx, T dy )
     {
          m_top_left.move ( dx, dy );
          m_bottom_right.move ( dx, dy );
     }

     template < typename T >
     inline void rect<T>::move_x ( T dx )
     {
          m_top_left.move_x ( dx );
          m_bottom_right.move_x ( dx );
     }

     template < typename T >
     inline void rect<T>::move_y ( T dy )
     {
          m_top_left.move_y ( dy );
          m_bottom_right.move_y ( dy );
     }

     template < typename T >
     inline void rect<T>::set ( T left, T top, T right, T bottom )
     {
          m_top_left.set ( left, top );
          m_bottom_right.set ( right, bottom );
          validate ( );
     }

     template < typename T >
     inline void rect<T>::set_left ( T left )
     {
          m_top_left.set_x ( left );
          validate ( );
     }

     template < typename T >
     inline void rect<T>::set_top ( T top )
     {
          m_top_left.set_y ( top );
          validate ( );
     }

     template < typename T >
     inline void rect<T>::set_right ( T right )
     {
          m_bottom_right.set_x ( right );
          validate ( );
     }

     template < typename T >
     inline void rect<T>::set_bottom ( T bottom )
     {
          m_bottom_right.set_x ( bottom );
          validate ( );
     }

     template < typename T >
     inline void rect<T>::validate ( )
     {
          // make sure the values don't cross horizontally and vertically
          assert ( m_top_left.x ( ) <= m_bottom_right.x ( ) );
          assert ( m_top_left.y ( ) >= m_bottom_right.y ( ) );
     }

     template < typename T >
     inline T rect<T>::left () const
     {
          return m_top_left.x ( );
     }

     template < typename T >
     inline T rect<T>::top () const
     {
          return m_top_left.y ( );
     }

     template < typename T >
     inline T rect<T>::right ( ) const
     {
          return m_bottom_right.y ( );
     }

     template < typename T >
     inline T rect<T>::bottom ( ) const
     {
          return m_bottom_right.y ( );
     }

     template < typename T >
     inline T rect<T>::width ( ) const
     {
          return m_bottom_right.x ( ) - m_top_left.x ( );
     }

     template < typename T >
     inline T rect<T>::height ( ) const
     {
          return m_top_left.y ( ) - m_bottom_right.y ( );
     }

     template < typename T >
     inline vector2d<T> rect<T>::top_left ( ) const
     {
          return m_top_left;
     }

     template < typename T >
     inline vector2d<T> rect<T>::top_right ( ) const
     {
          return vector2d<T> ( m_bottom_right.x ( ), m_top_left.y ( ) );
     }

     template < typename T >
     inline vector2d<T> rect<T>::bottom_left ( ) const
     {
          return vector2d<T> ( m_top_left.x ( ), m_bottom_right.y ( ) );
     }

     template < typename T >
     inline vector2d<T> rect<T>::bottom_right ( ) const
     {
          return m_bottom_right;
     }
}

#endif
