#include "ui_button.hpp"

#include "draw_utils.hpp"

using namespace bryte;

const Uint32 ui_button::k_state_colors [ 3 ] = {
     k_idle_color,
     k_highlighted_color,
     k_pressed_color
};

ui_button::ui_button ( SDL_Surface* icon_surface, vector position, const rectangle& clip ) :
     m_icon_sprite ( icon_surface, position, clip ),
     m_state ( state::idle ),
     m_clickable ( true )
{

}

void ui_button::update ( vector mouse_position, bool mouse_clicked )
{
     rectangle screen_rect ( m_icon_sprite.position ( ).x ( ),
                             m_icon_sprite.position ( ).y ( ),
                             m_icon_sprite.position ( ).x ( ) + m_icon_sprite.clip ( ).width ( ),
                             m_icon_sprite.position ( ).y ( ) + m_icon_sprite.clip ( ).height ( ) );

     if ( screen_rect.contains ( mouse_position ) ) {
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

void ui_button::draw ( SDL_Surface* back_buffer )
{
     m_icon_sprite.blit_onto ( back_buffer );

     
     draw_utils::draw_border ( rectangle ( m_icon_sprite.position ( ),
                                           m_icon_sprite.position ( ) +
                                           vector ( m_icon_sprite.clip ( ).width ( ),
                                                    m_icon_sprite.clip ( ).height ( ) ) ),
                               k_state_colors [ static_cast< int >( m_state ) ],
                               back_buffer );
}
