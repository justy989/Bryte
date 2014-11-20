#include "ui_button.hpp"

#include "draw_utils.hpp"

using namespace bryte;

const Uint32 ui_button::k_state_colors [ 3 ] = {
     k_idle_color,
     k_highlighted_color,
     k_pressed_color
};

ui_button::ui_button ( const rectangle& border ) :
     m_border ( border ),
     m_state ( state::idle ),
     m_clickable ( true )
{

}

void ui_button::update ( vector mouse_position, bool mouse_clicked )
{
     if ( m_border.contains ( mouse_position ) ) {
          m_state = state::highlighted;

          if ( mouse_clicked ) {
               if ( m_clickable ) {
                    m_state = state::pressed;
                    m_clickable = false;
               }
          }
          else {
               m_clickable = true;
          }
     }
     else {
          m_clickable = true;
          m_state = state::idle;
     }
}

void ui_button::draw_border ( SDL_Surface* back_buffer )
{
     draw_utils::draw_border ( m_border,
                               k_state_colors [ static_cast< int >( m_state ) ],
                               back_buffer );
}

