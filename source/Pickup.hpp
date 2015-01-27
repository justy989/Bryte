#ifndef BRYTE_PICKUP_HPP
#define BRYTE_PICKUP_HPP

#include "Vector.hpp"

namespace bryte {

     struct Pickup {

          inline Pickup ( );

          enum Type {
               none,
               health,
               key,
               ingredient,
               count
          };

          static const Int32 c_dimension_in_pixels = 10;
          static const Real32 c_dimension_in_meters;

          static const Char8* c_names [ Type::count ];

          Vector position;
          Type   type;
     };

     inline Pickup::Pickup ( ) :
          type ( Type::none )
     {

     }
};

#endif
