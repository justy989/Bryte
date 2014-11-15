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

          static const vector_base_type k_tile_width = 16;
          static const vector_base_type k_tile_height = 16;

          static const vector_base_type k_max_width = 32;
          static const vector_base_type k_max_height = 32;
          static const vector_base_type k_max_tiles = k_max_width * k_max_height;

          static const ubyte k_max_doors = 8;

          using doorvector = svector < door, k_max_doors >;

     public:

          room ( vector_base_type width, vector_base_type height );

          void reset ( vector_base_type width, vector_base_type height );

          inline void set_tile ( const vector& location, ubyte tile );
          inline ubyte get_tile ( const vector& location ) const;

          inline void set_solid ( const vector& location, bool solid );
          inline bool get_solid ( const vector& location ) const;

          inline doorvector& doors ( );

          inline vector_base_type width ( ) const;
          inline vector_base_type height ( ) const;

     private:

          vector_base_type location_to_index ( const vector& location ) const;

     private:

          vector_base_type m_width;
          vector_base_type m_height;

          ubyte m_tiles [ k_max_tiles ];
          bool m_solids [ k_max_tiles ];

          svector<door, k_max_doors> m_doors;
     };

     inline void room::set_tile ( const vector& location, ubyte tile )
     {
          auto index = location_to_index ( location );

          m_tiles [ index ] = tile;
     }

     inline ubyte room::get_tile ( const vector& location ) const
     {
          auto index = location_to_index ( location );

          return m_tiles [ index ];
     }

     inline void room::set_solid ( const vector& location, bool solid )
     {
          auto index = location_to_index ( location );

          m_solids [ index ] = solid;
     }

     inline bool room::get_solid ( const vector& location ) const
     {
          auto index = location_to_index ( location );

          return m_solids [ index ];
     }

     inline room::doorvector& room::doors ( )
     {
          return m_doors;
     }

     inline vector_base_type room::width ( ) const { return m_width; }
     inline vector_base_type room::height ( ) const { return m_height; }
}

#endif