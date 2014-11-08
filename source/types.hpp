/* Types to be used throughout the project. Also includes some explicit template
 * instantiation.
 */

#ifndef BRYTE_TYPES_HPP
#define BRYTE_TYPES_HPP

#include "rect.hpp"

namespace bryte
{
     // base unsigned types
     using ubyte = unsigned char;
     using uword = unsigned short;

     // base coordinate system type
     using coordinate_type = short;

     // base point and rect types
     using point_type = point<coordinate_type>;
     using rect_type  = rect<coordinate_type>;

     // aliases for points
     using position = point_type;
     using location = point_type;

     // aliases for rects
     using rectangle    = rect_type;
     using bounding_box = rect_type;

     // explicit instantiation
     template class point_type;
     template class rect_type;
}

#endif
