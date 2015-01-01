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
     using vector_base_type = short;

     // explicit instantiation of types used in this project
     template class vector2d<vector_base_type>;
     template class rect<vector_base_type>;

     // base point and rect types
     using vector_type = vector2d<vector_base_type>;
     using rect_type   = rect<vector_base_type>;

     // standard x-y quantity
     using vector = vector_type;

     // aliases for rects
     using rectangle    = rect_type;
     using bounding_box = rect_type;
}

#endif
