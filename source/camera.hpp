/* camera: holds the viewport to the world and handles moving around within
 *         the given bounds.
 */

#ifndef BRYTE_CAMERA_HPP
#define BRYTE_CAMERA_HPP

#include "types.hpp"

namespace bryte
{
     class camera {
     public:

          camera ( const rectangle& viewport,
                   const rectangle& bounds );

          void reset ( const rectangle& viewport,
                       const rectangle& bounds );

          void move ( const vector& delta );

          void look_at ( const vector& location );

          inline const vector& center ( ) const;
          inline const rectangle& viewport ( ) const;

     private:

          void calculate_center ( );

     private:

          rectangle m_viewport;
          rectangle m_bounds;

          vector m_center;
     };

     inline const vector& camera::center ( ) const { return m_center; }
     inline const rectangle& camera::viewport ( ) const { return m_viewport; }
}

#endif