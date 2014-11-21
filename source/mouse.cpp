#include "mouse.hpp"
#include "sdl_window.hpp"

using namespace bryte;

mouse::mouse ( int window_width, int window_height ) :
     m_window_width ( window_width ),
     m_window_height ( window_height ),
     m_button_states ( 0 )
{

}

void mouse::update ( )
{
     int window_x, window_y;

     m_button_states = SDL_GetMouseState ( &window_x, &window_y );

     // calculate the coordinate ratio on the window: 320 / 640 would be 0.5
     float window_x_pct = static_cast< float >( window_x ) / static_cast< float >( m_window_width );
     float window_y_pct = static_cast< float >( window_y ) / static_cast< float >( m_window_height );

     // convert it to the back buffer quantity: 0.5 * 256 = 128
     float screen_x = window_x_pct * static_cast< float >( sdl_window::k_back_buffer_width );
     float screen_y = window_y_pct * static_cast< float >( sdl_window::k_back_buffer_height );

     // find the world position by casting down to our vector type
     m_position.set ( static_cast< vector_base_type >( screen_x ),
                      static_cast< vector_base_type >( screen_y ) );

     if ( left_clicked ( ) ) {
          int blah = 5;
          blah++;
     }
}

