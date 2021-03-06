#ifndef BRYTE_REGION_HPP
#define BRYTE_REGION_HPP

#include "Types.hpp"

namespace bryte {
     struct Region {

     public:

          Bool init ( );
          Bool load_info ( Int32 index );

     public:

          static const Char8* c_region_list_filepath;
          static const Int32 c_max_filepath_length = 64;

     public:

          Int32 current_index;

          Char8 name [ c_max_filepath_length ];
          Char8 map_list_filepath [ c_max_filepath_length ];
          Char8 tilesheet_filepath [ c_max_filepath_length ];
          Char8 decorsheet_filepath [ c_max_filepath_length ];
          Char8 lampsheet_filepath [ c_max_filepath_length ];
          Char8 exitsheet_filepath [ c_max_filepath_length ];
          Char8 destructablesheet_filepath [ c_max_filepath_length ];
     };
}

#endif

