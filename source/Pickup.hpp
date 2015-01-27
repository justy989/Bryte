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

          static const Real32 c_dimension; // temporary

          Vector position;
          Type   type;
     };

     inline Pickup::Pickup ( ) :
          type ( Type::none )
     {

     }
};

#endif
