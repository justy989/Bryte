/* configuration: loads a config file with options for running the app. Options
 *                such as window dimensions, volume and key bindings are
 *                available.
 */

#ifndef BRYTE_CONFIGURATION_HPP
#define BRYTE_CONFIGURATION_HPP

#include <string>

namespace bryte
{
     class configuration {
     public:

          template < typename T >
          struct option {

               option ( const std::string name, T value );

               std::string name;
               T value;
          };

     public:

          configuration ( const std::string& filepath );

          inline int window_width ( ) const;
          inline int window_height ( ) const;

     private:

          void set_option ( const std::string& name,
                            const std::string& value );

          void set_option_above_default ( option<int>& opt,
                                          const std::string& value );

     private:

          static const int k_default_window_width = 800;
          static const int k_default_window_height = 600;

     private:

          std::string m_filepath;
          int m_line_number;

          option<int> m_window_width;
          option<int> m_window_height;
     };

     template < typename T >
     configuration::option<T>::option ( const std::string name, T value ) :
          name ( name ),
          value ( value )
     {

     }

     inline int configuration::window_width ( ) const { return m_window_width.value; }
     inline int configuration::window_height ( ) const { return m_window_height.value; }
}

#endif