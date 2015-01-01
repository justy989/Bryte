#include "ui_icon_button.hpp"

using namespace bryte;

ui_icon_button::ui_icon_button ( SDL_Surface* icon_surface, vector position, const rectangle& clip ) :
     ui_button ( rectangle ( position.x ( ), position.y ( ),
                             position.x ( ) + clip.width ( ),
                             position.y ( ) + clip.height ( ) ) ),
     m_icon_sprite ( icon_surface, position, clip )
{

}

void ui_icon_button::draw ( SDL_Surface* back_buffer )
{
     m_icon_sprite.blit_onto ( back_buffer );

     draw_border ( back_buffer );
}

