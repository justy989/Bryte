#include "title_state.hpp"

using namespace bryte;

title_state::title_state ( surface_man& sman ) :
     m_background ( sman.load ( "title_menu_bg.bmp" ) )
{

}

void title_state::update ( )
{

}

void title_state::draw ( SDL_Surface* back_buffer )
{
     SDL_Rect src { 0, 0,
                    m_background->w, m_background->h };

     SDL_Rect dst { 0, 0,
                    back_buffer->w, back_buffer->h };

     SDL_BlitScaled ( m_background, &src,
                      back_buffer, &dst );
}

void title_state::handle_sdl_event ( const SDL_Event& sdl_event )
{

}

