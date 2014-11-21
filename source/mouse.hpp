/* mouse: holds mouse position relative to the back buffer and allows querying
 *        buttons pressed.
 */

#ifndef BRYTE_MOUSE_HPP
#define BRYTE_MOUSE_HPP

#include "types.hpp"
#include "sdl_window.hpp"

#include <SDL2/SDL.h>

namespace bryte
{
     class mouse {
     public:

          mouse ( int window_width, int window_height );

          void update ( );

          inline const vector& position ( ) const;

          inline bool left_clicked ( ) const;
          inline bool right_clicked ( ) const;

     private:

          int m_window_width;
          int m_window_height;

          vector m_position;
          Uint32 m_button_states;
     };

     inline const vector& mouse::position ( ) const { return m_position; }

     inline bool mouse::left_clicked ( ) const
     {
          return m_button_states & SDL_BUTTON ( SDL_BUTTON_LEFT );
     }

     inline bool mouse::right_clicked ( ) const
     {
          return m_button_states & SDL_BUTTON ( SDL_BUTTON_RIGHT );
     }
}

#endif
