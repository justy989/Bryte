/* ui_icon_button: button with icon inside
 */

#ifndef BRYTE_UI_ICON_BUTTON_HPP
#define BRYTE_UI_ICON_BUTTON_HPP

#include "ui_button.hpp"
#include "clipped_sprite.hpp"

namespace bryte
{
     class ui_icon_button : public ui_button {
     public:

          ui_icon_button ( SDL_Surface* icon_surface, vector position, const rectangle& clip );

          void draw ( SDL_Surface* back_buffer );

     private:

          clipped_sprite m_icon_sprite;
     };
}

#endif

