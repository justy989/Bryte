/* door: Holds location and destination of an door in a room.
 */

#ifndef BRYTE_door_HPP
#define BRYTE_door_HPP

#include <cstddef>

#include "types.hpp"

namespace bryte
{
     class door {
     public:

          inline door ( );
          inline door ( const vector& location,
                        std::size_t dest_index, const vector& dest_loc );

          inline const vector& loc ( ) const;
          inline const std::size_t destination_index ( ) const;
          inline const vector& dest_loc ( ) const;

     private:

          vector m_location;

          std::size_t m_destination_index;
          vector m_destination_location;
     };

     inline door::door ( ) :
          m_destination_index ( 0 )
     {

     }

     inline door::door ( const vector& loc, std::size_t dest_index, const vector& dest_loc ) :
          m_location ( loc ),
          m_destination_index ( dest_index ),
          m_destination_location ( dest_loc )
     {

     }

     inline const vector& door::loc ( ) const { return m_location; }
     inline const std::size_t door::destination_index ( ) const { return m_destination_index; }
     inline const vector& door::dest_loc ( ) const { return m_destination_location; }
}

#endif