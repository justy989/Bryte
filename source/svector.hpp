/* svector: stack vector wrapped around std::array. counts elements, allows
 *          adding and removing.
 */

#ifndef BRYTE_SVECTOR_HPP
#define BRYTE_SVECTOR_HPP

#include <array>
#include <cassert>

namespace bryte
{
     template < typename T, std::size_t TOTAL_SIZE >
     class svector {
     public:

          inline svector ( );

          inline bool add ( const T& elem );
          inline bool remove ( std::size_t index );

          inline T& operator[] ( std::size_t index );

          inline std::size_t size ( ) const;
          inline std::size_t capacity ( ) const;

     private:

          std::array<T, TOTAL_SIZE> m_array;
          std::size_t m_element_count;
     };

     template < typename T, std::size_t TOTAL_SIZE >
     inline svector<T, TOTAL_SIZE>::svector ( ) :
          m_element_count ( 0 )
     {

     }

     template < typename T, std::size_t TOTAL_SIZE >
     inline bool svector<T, TOTAL_SIZE>::add ( const T& elem )
     {
          if ( m_element_count >= TOTAL_SIZE ) {
               return false;
          }

          m_array [ m_element_count ] = elem;
          m_element_count++;
          return true;
     }

     template < typename T, std::size_t TOTAL_SIZE >
     inline bool svector<T, TOTAL_SIZE>::remove ( std::size_t index )
     {
          assert ( index < TOTAL_SIZE );

          if ( index >= m_element_count ) {
               return false;
          }

          if ( m_element_count > 1 ) {
               auto last_element = m_element_count - 2;

               // move each element closer to the origin, overwriting the removed element
               for ( auto i = index; i <= last_element; i++ ) {
                    m_array [ i ] = m_array [ i + 1 ];
               }
          }

          m_element_count--;
          return true;
     }

     template < typename T, std::size_t TOTAL_SIZE >
     inline T& svector<T, TOTAL_SIZE>::operator[] ( std::size_t index )
     {
          assert ( index < m_element_count );
          return m_array [ index ];
     }

     template < typename T, std::size_t TOTAL_SIZE >
     inline std::size_t svector<T, TOTAL_SIZE>::size ( ) const
     {
          return m_element_count;
     }

     template < typename T, std::size_t TOTAL_SIZE >
     inline std::size_t svector<T, TOTAL_SIZE>::capacity ( ) const
     {
          return m_array.size ( );
     }
}

#endif