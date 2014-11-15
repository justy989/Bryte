#include "configuration.hpp"

#include <fstream>
#include <stdexcept>
#include <sstream>

using namespace bryte;

configuration::configuration ( const std::string& filepath ) :
     m_filepath ( filepath ),
     m_window_width ( "window_width", k_default_window_width ),
     m_window_height ( "window_height", k_default_window_height )
{
     std::ifstream file ( filepath.c_str ( ) );

     if ( !file.is_open ( ) ) {
          throw std::runtime_error ( std::string ( "Unable to open configuration file: " ) + filepath );
     }

     std::string line;
     std::string name;
     std::string equals;
     std::string value;

     while ( !file.eof ( ) ) {
          // get the current line
          std::getline ( file, line );

          // ignore empty lines
          if ( !line.length ( ) ) {
               continue;
          }

          // parse the 3 tokens of this format:
          // name = value
          std::stringstream ss;
          ss << line;

          ss >> name;
          ss >> equals;
          ss >> value;

          // fail if we don't have name value pairs at this point
          if ( !name.length ( ) && !value.length ( ) ) {
               std::stringstream sse;

               sse << "Configuration options require format: 'name = value'. Unable to parse line: ";
               sse << m_line_number;
               sse << " in file: ";
               sse << m_filepath;

               throw std::runtime_error ( sse.str ( ) );
          }

          // set the option value
          set_option ( name, value );

          m_line_number++;
     }
}

void configuration::set_option ( const std::string& name,
                                 const std::string& value )
{
     // check each setting name and set them accordingly
     if ( name == m_window_width.name ) {
          set_option_above_default ( m_window_width, value );
     } 
     else if ( name == m_window_height.name ) {
          set_option_above_default ( m_window_height, value );
     }
     else {
          std::stringstream ss;

          ss << "Unidentified configuration option name '";
          ss << name;
          ss << "' at line: ";
          ss << m_line_number;
          ss << " in file: ";
          ss << m_filepath;

          throw std::runtime_error ( ss.str ( ) );
     }
}

void configuration::set_option_above_default ( option<int>& opt,
                                               const std::string& value )
{
     int integer = std::stoi ( value );

     if ( integer < opt.value ) {
          std::stringstream ss;

          ss << "Configuration option '";
          ss << opt.name;
          ss << "' set to value below minimum: ";
          ss << opt.value;
          ss << "at line: ";
          ss << m_line_number;
          ss << " in file: ";
          ss << m_filepath;

          throw std::runtime_error ( ss.str ( ) );
     }

     opt.value = integer;
}
