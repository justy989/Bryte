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
     using coordinate_base_type = short;

     // explicit instantiation of types used in this project
     template class point<coordinate_base_type>;
     template class rect<coordinate_base_type>;

     // base point and rect types
     using point_type = point<coordinate_base_type>;
     using rect_type  = rect<coordinate_base_type>;

     // aliases for points
     using position = point_type;
     using location = point_type;

     // aliases for rects
     using rectangle    = rect_type;
     using bounding_box = rect_type;
}

#endif
