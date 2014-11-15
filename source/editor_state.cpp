#include "editor_state.hpp"

using namespace bryte;

editor_state::editor_state ( surface_man& sman ) :
     m_room ( 20, 20 ),
     m_camera ( rectangle ( 0, 0, 256, 240 ),
                rectangle ( 0, 0,
                            m_room.width ( ) * room::k_tile_width, 
			             m_room.height ( ) * room::k_tile_height ) )
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
     if ( sdl_event.type == SDL_KEYDOWN ) {
          if ( sdl_event.key.keysym.sym == SDLK_a ) {
               m_camera.move ( vector ( -1, 0 ) );
          }
          else if ( sdl_event.key.keysym.sym == SDLK_d ) {
               m_camera.move ( vector ( 1, 0 ) );
          }
          else if ( sdl_event.key.keysym.sym == SDLK_w ) {
               m_camera.move ( vector ( 0, 1 ) );
          }
          else if ( sdl_event.key.keysym.sym == SDLK_s ) {
               m_camera.move ( vector ( 0, -1 ) );
          }
     }
}
