#include "editor_state.hpp"

using namespace bryte;

editor_state::editor_state ( surface_man& sman ) :
m_room ( 12, 12 ),
m_camera ( rectangle ( 0, 240, 256, 0 ),
           rectangle ( 0, 240, 256, 0 ) )
{
     auto* surface = sman.load ( "castle_tilesheet.bmp" );

     m_room_display.change_tilesheet ( surface );
}

void editor_state::update ( )
{

}

void editor_state::draw ( SDL_Surface* back_buffer )
{
     m_room_display.display ( m_room, m_camera, back_buffer );
}

void editor_state::handle_sdl_event ( const SDL_Event& sdl_event )
{

}
