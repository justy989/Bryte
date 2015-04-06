#include "Region.hpp"
#include "Log.hpp"

#include <fstream>
#include <sstream>
#include <cstring>

using namespace bryte;

const Char8* Region::c_region_list_filepath = "content/region_list.txt";

Bool Region::load_info ( Int32 index )
{
     static const Int32 c_max_line_length = 256;
     char line [ c_max_line_length ];
     std::ifstream file ( c_region_list_filepath );

     LOG_INFO ( "Loading region %d\n", index );

     if ( !file.is_open ( ) ) {
          LOG_ERROR ( "Failed to load region info file: '%s'\n", c_region_list_filepath );
          return false;
     }

     Int32 region_count = 0;

     std::stringstream ss;

     while ( !file.eof ( ) ) {
          file.getline ( line, c_max_line_length );

          if ( region_count == index ) {
               break;
          }

          region_count++;
     }

     if ( region_count < index ) {
          LOG_ERROR ( "" );
          return false;
     }

     ss << line;

     std::string tmp_string;

     // TODO: load all into tmp vars and validate before setting them all
     ss >> tmp_string;
     strncpy ( name, tmp_string.c_str ( ), c_max_filepath_length );

     ss >> tmp_string;
     strncpy ( map_list_filepath, tmp_string.c_str ( ), c_max_filepath_length );

     ss >> tmp_string;
     strncpy ( tilesheet_filepath, tmp_string.c_str ( ), c_max_filepath_length );

     ss >> tmp_string;
     strncpy ( decorsheet_filepath, tmp_string.c_str ( ), c_max_filepath_length );

     ss >> tmp_string;
     strncpy ( lampsheet_filepath, tmp_string.c_str ( ), c_max_filepath_length );

     ss >> tmp_string;
     strncpy ( exitsheet_filepath, tmp_string.c_str ( ), c_max_filepath_length );

     // do a bit of error checking
     if ( !strlen ( name ) ) {
          LOG_ERROR ( "Empty name\n" );
          return false;
     }

     if ( !strlen ( map_list_filepath ) ) {
          LOG_ERROR ( "Empty map list\n" );
          return false;
     }

     if ( !strlen ( tilesheet_filepath ) ) {
          LOG_ERROR ( "Empty tilesheet\n" );
          return false;
     }

     if ( !strlen ( decorsheet_filepath ) ) {
          LOG_ERROR ( "Empty decorsheet\n" );
          return false;
     }

     if ( !strlen ( lampsheet_filepath ) ) {
          LOG_ERROR ( "Empty lampsheet\n" );
          return false;
     }

     if ( !strlen ( exitsheet_filepath ) ) {
          LOG_ERROR ( "Empty exitsheet\n" );
          return false;
     }

     LOG_INFO ( " name: '%s'\n", name );
     LOG_INFO ( " map list: '%s'\n", map_list_filepath );
     LOG_INFO ( " tilesheet: '%s'\n", tilesheet_filepath );
     LOG_INFO ( " decorsheet: '%s'\n", decorsheet_filepath );
     LOG_INFO ( " lampsheet: '%s'\n", lampsheet_filepath );
     LOG_INFO ( " exitsheet: '%s'\n", exitsheet_filepath );

     current_index = index;

     return true;
}

