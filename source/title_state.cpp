#include "title_state.hpp"
#include "sdl_window.hpp"

using namespace bryte;

title_state::title_state ( surface_man& sman, int window_width, int window_height ) :
     m_mouse ( window_width, window_height,
               sdl_window::k_back_buffer_width,
               sdl_window::k_back_buffer_height ),
     m_background ( sman.load ( "title_menu_bg.bmp" ) ),
     m_new_game_text ( "NEW GAME", vector ( 152, 105 ) ),
     m_load_game_text ( "LOAD GAME", vector ( 149, 120 ) ),
     m_editor_text ( "EDITOR", vector ( 158, 135 ) ),
     m_options_text ( "OPTIONS", vector ( 155, 150 ) ),
     m_quit_text ( "QUIT", vector ( 164, 165 ) )
{

}

void title_state::update ( )
{
     m_mouse.update ( );

     m_new_game_text.update ( m_mouse.position ( ), m_mouse.left_clicked ( ) );
     m_load_game_text.update ( m_mouse.position ( ), m_mouse.left_clicked ( ) );
     m_editor_text.update ( m_mouse.position ( ), m_mouse.left_clicked ( ) );
     m_options_text.update ( m_mouse.position ( ), m_mouse.left_clicked ( ) );
     m_quit_text.update ( m_mouse.position ( ), m_mouse.left_clicked ( ) );
}

void title_state::draw ( SDL_Surface* back_buffer )
{
     SDL_Rect src { 0, 0,
                    m_background->w, m_background->h };

     SDL_Rect dst { 0, 0,
                    back_buffer->w, back_buffer->h };

     SDL_BlitScaled ( m_background, &src,
                      back_buffer, &dst );

     m_new_game_text.draw ( back_buffer );
     m_load_game_text.draw ( back_buffer );
     m_editor_text.draw ( back_buffer );
     m_options_text.draw ( back_buffer );
     m_quit_text.draw ( back_buffer );
}

void title_state::handle_sdl_event ( const SDL_Event& sdl_event )
{

}

