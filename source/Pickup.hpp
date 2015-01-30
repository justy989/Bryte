#ifndef BRYTE_PICKUP_HPP
#define BRYTE_PICKUP_HPP

#include "Entity.hpp"

namespace bryte {

     struct Pickup : public Entity {

          enum Type {
               none,
               health,
               key,
               ingredient,
               count
          };

          inline Pickup ( );

          inline Void clear ( );

          static const Int32 c_dimension_in_pixels = 10;
          static const Real32 c_dimension_in_meters;

          static const Char8* c_names [ Type::count ];

          Type   type;
     };

     inline Pickup::Pickup ( ) :
          type ( Type::none )
     {

     }

     inline Void Pickup::clear ( )
     {
          type = Type::none;
     }
};

#endif
