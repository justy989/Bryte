/* room: Holds various layers as part of a room. Tiles, collision, doors, decor, etc.
 *       Allows getting and setting all aspects.
 */

#ifndef BRYTE_ROOM_HPP
#define BRYTE_ROOM_HPP

#include "door.hpp"
#include "svector.hpp"

namespace bryte
{
     class room {
     public:

          static const coordinate_base_type k_max_width = 32;
          static const coordinate_base_type k_max_height = 32;
          static const coordinate_base_type k_max_tiles = k_max_width * k_max_height;

          static const ubyte k_max_doors = 8;

          using doorvector = svector < door, k_max_doors >;

     public:

          room ( coordinate_base_type width, coordinate_base_type height );

          void reset ( coordinate_base_type width, coordinate_base_type height );

          inline void set_tile ( const location& loc, ubyte tile );
          inline ubyte get_tile ( const location& loc );

          inline void set_solid ( const location& loc, bool solid );
          inline bool get_solid ( const location& loc );

          inline doorvector& doors ( );

     private:

          coordinate_base_type location_to_index ( const location& loc );

     private:

          coordinate_base_type m_width;
          coordinate_base_type m_height;

          ubyte m_tiles [ k_max_tiles ];
          bool m_solids [ k_max_tiles ];

          svector<door, k_max_doors> m_doors;
     };

     inline void room::set_tile ( const location& loc, ubyte tile )
     {
          auto index = location_to_index ( loc );

          m_tiles [ index ] = tile;
     }

     inline ubyte room::get_tile ( const location& loc )
     {
          auto index = location_to_index ( loc );

          return m_tiles [ index ];
     }

     inline void room::set_solid ( const location& loc, bool solid )
     {
          auto index = location_to_index ( loc );

          m_solids [ index ] = solid;
     }

     inline bool room::get_solid ( const location& loc )
     {
          auto index = location_to_index ( loc );

          return m_solids [ index ];
     }

     inline room::doorvector& room::doors ( )
     {
          return m_doors;
     }
}

#endif