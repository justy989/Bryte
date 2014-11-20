#ifndef BRYTE_UI_TEXT_BUTTON_HPP
#define BRYTE_UI_TEXT_BUTTON_HPP

#include "ui_button.hpp"

#include <SDL2/SDL.h>

#include <string>

namespace bryte
{
     class ui_text_button : public ui_button {
     public:

          ui_text_button ( const std::string& text, vector position );

          void draw ( SDL_Surface* back_buffer );

     private:

          std::string m_text;

          vector m_position;
     };
}

#endif
