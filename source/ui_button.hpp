/* ui_button : base class for button functionality and drawing
 */

#ifndef BRYTE_UI_BUTTON_HPP
#define BRYTE_UI_BUTTON_HPP

#include "types.hpp"

#include <SDL2/SDL.h>

namespace bryte
{
     class ui_button {
     public:

          enum class state {
               idle,
               highlighted,
               pressed
          };

     public:

          ui_button ( const rectangle& border );

          void update ( vector mouse_position, bool mouse_clicked );

          void draw_border ( SDL_Surface* back_buffer );

          inline state get_state ( ) const;

     private:

          static const Uint32 k_idle_color = 0xFFFFFF;
          static const Uint32 k_highlighted_color = 0x999999;
          static const Uint32 k_pressed_color = 0x000000;

          static const Uint32 k_state_colors [ 3 ];

     private:

          rectangle m_border;

          state m_state;

          bool m_clickable;
     };

     inline ui_button::state ui_button::get_state ( ) const { return m_state; }
}

#endif

