#ifndef BRYTE_ELEMENT_HPP
#define BRYTE_ELEMENT_HPP

#include "Log.hpp"

namespace bryte {
     enum Element {
          none,
          fire,
          ice
     };

     inline Element transition_element ( Element a, Element b )
     {
          switch ( a ) {
          case none:
               a = b;
               break;
          case fire:
               switch ( b ) {
               case none:
               case fire:
                    break;
                    break;
               case ice:
                    a = none;
                    break;
               }
               break;
          case ice:
               switch ( b ) {
               case none:
               case ice:
                    break;
               case fire:
                    a = none;
                    break;
               }
               break;
          }

          return a;
     }
}

#endif

