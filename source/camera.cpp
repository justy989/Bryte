#include "camera.hpp"

#include <algorithm>
#include <cassert>

using namespace bryte;

camera::camera ( const rectangle& viewport,
                 const rectangle& bounds ) :
     m_viewport ( viewport ),
     m_bounds ( bounds )
{
     reset ( viewport, bounds );
}

void camera::reset ( const rectangle& viewport,
                     const rectangle& bounds )
{
     m_viewport = viewport;
     m_bounds = bounds;

     assert ( m_bounds.encompasses ( m_viewport ) );

     calculate_center ( );
}

void camera::move ( const vector& delta )
{
     // horizontal checking
     if ( delta.x ( ) > 0 ) {

          // check if we are still in bounds
          if ( m_viewport.right ( ) < m_bounds.right ( ) ) {

               // find the min distance we should move
               auto delta_x = std::min ( delta.x ( ),
                    static_cast<vector_base_type>( m_bounds.right ( ) - m_viewport.right ( ) ) );
               m_viewport.move_x ( delta_x );
          }
     }
     else if ( delta.x ( ) < 0 ) {
          if ( m_viewport.left ( ) > m_bounds.left ( ) ) {
               auto delta_x = std::min ( delta.x ( ),
                                         static_cast<vector_base_type>( m_viewport.left ( ) - m_bounds.left ( ) ) );
               m_viewport.move_x ( delta_x );
          }
     }

     if ( delta.y ( ) > 0 ) {
          if ( m_viewport.top ( ) < m_bounds.top ( ) ) {
               auto delta_y = std::min ( delta.y ( ),
                    static_cast<vector_base_type>( m_bounds.top ( ) - m_viewport.top ( ) ) );
               m_viewport.move_y ( delta_y );
          }
     } else if ( delta.y ( ) < 0 ) {
          if ( m_viewport.bottom ( ) > m_bounds.bottom ( ) ) {
               auto delta_y = std::min ( delta.y ( ),
                    static_cast<vector_base_type>( m_viewport.bottom ( ) - m_bounds.bottom ( ) ) );
               m_viewport.move_y ( delta_y );
          }
     }

     calculate_center ( );
}

void camera::look_at ( const vector& location )
{
     m_center = location;

     vector_base_type half_width = m_viewport.width ( ) / 2;
     vector_base_type half_height = m_viewport.height ( ) / 2;;

     m_viewport.set ( m_center.x ( ) - half_width,
                      m_center.y ( ) - half_height,
                      m_center.x ( ) + half_width,
                      m_center.y ( ) + half_height );

     m_viewport.restrict_to ( m_bounds );
}

void camera::calculate_center ( )
{
     m_center.set ( m_viewport.left ( ) + ( m_viewport.width ( ) / 2 ),
                    m_viewport.bottom ( ) + ( m_viewport.height ( ) / 2 ) );
}

