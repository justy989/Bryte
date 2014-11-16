/* text_raii: Only used to wrap create/destroy text funcs
 */

#ifndef BRYTE_TEXT_RAII_HPP
#define BRYTE_TEXT_RAII_HPP

#include "text.hpp"

namespace bryte
{
     class text_raii {
     public:

          text_raii ( )
          {
               text::create_sprite ( );
          }

          ~text_raii ( )
          {
               text::destroy_sprite ( );
          }
     };
}

#endif