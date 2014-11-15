#include "camera.hpp"

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
     m_viewport += delta;

     m_viewport.restrict_to ( m_bounds );

     calculate_center ( );
}

void camera::look_at ( const vector& location )
{
     m_center = location;

     vector_base_type half_width = m_viewport.width ( ) / 2;
     vector_base_type half_height = m_viewport.height ( ) / 2;;

     m_viewport.set ( m_center.x ( ) - half_width,
                      m_center.y ( ) + half_height,
                      m_center.x ( ) + half_width,
                      m_center.y ( ) - half_height );

     m_viewport.restrict_to ( m_bounds );
}

void camera::calculate_center ( )
{
     m_center.set ( m_viewport.left ( ) + ( m_viewport.width ( ) / 2 ),
                    m_viewport.bottom ( ) + ( m_viewport.height ( ) / 2 ) );
}
