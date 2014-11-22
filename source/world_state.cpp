#include "world_state.hpp"
#include "sdl_window.hpp"

using namespace bryte;

world_state::world_state ( surface_man& sman ) :
     m_room ( 20, 20 ),
     m_camera ( rectangle ( 0, 0,
                            sdl_window::k_back_buffer_width,
                            sdl_window::k_back_buffer_height ),
                rectangle ( 0, 0,
                            m_room.width ( ) * room::k_tile_width,
                            m_room.height ( ) * room::k_tile_height ) )
{
     m_room_display.change_tilesheet ( sman.load ( "castle_tilesheet.bmp" ) );
}

game_state world_state::update ( )
{
     return game_state::world;
}

void world_state::draw ( SDL_Surface* back_buffer )
{
     m_room_display.display ( m_room, m_camera, vector ( 0, 0 ), back_buffer );
}

void world_state::handle_sdl_event ( const SDL_Event& sdl_event )
{

}

