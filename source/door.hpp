/* door: Holds location and destination of an door in a room.
 */

#ifndef BRYTE_door_HPP
#define BRYTE_door_HPP

#include "types.hpp"

#include <cstddef>

namespace bryte
{
     class door {
     public:

          inline door ( );
          inline door ( const location& loc, std::size_t dest_index, const location& dest_loc );

          inline const location& loc ( ) const;
          inline const std::size_t destination_index ( ) const;
          inline const location& dest_loc ( ) const;

     private:

          location m_location;

          std::size_t m_destination_index;
          location m_destination_location;
     };

     inline door::door ( ) :
          m_destination_index ( 0 )
     {

     }

     inline door::door ( const location& loc, std::size_t dest_index, const location& dest_loc ) :
          m_location ( loc ),
          m_destination_index ( dest_index ),
          m_destination_location ( dest_loc )
     {

     }

     inline const location& door::loc ( ) const { return m_location; }
     inline const std::size_t door::destination_index ( ) const { return m_destination_index; }
     inline const location& door::dest_loc ( ) const { return m_destination_location; }
}

#endif