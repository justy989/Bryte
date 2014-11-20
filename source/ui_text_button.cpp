#include "ui_text_button.hpp"
#include "text.hpp"

using namespace bryte;

ui_text_button::ui_text_button ( const std::string& text, vector position ) :
     ui_button ( rectangle ( position.x ( ) - 2, position.y ( ) - 1,
                             position.x ( ) + ( text.length ( ) * ( text::k_character_width + 1 ) ) + 4,
                             position.x ( ) + text::k_character_height + 2 ) ),
     m_text ( text ),
     m_position ( position )
{

}

void ui_text_button::draw ( SDL_Surface* back_buffer )
{
     text::draw ( back_buffer, m_text, m_position );

     draw_border ( back_buffer );
}

