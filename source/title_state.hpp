/* title_state: allows starting a new game, loading a saved game, changing
 *               options, running the editor or quitting.
 */

#ifndef BRYTE_TITLE_STATE_HPP
#define BRYTE_TITLE_STATE_HPP

#include "mouse.hpp"
#include "surface_man.hpp"
#include "ui_text_button.hpp"

namespace bryte
{
     class title_state {
     public:
          title_state ( surface_man& sman, int window_width, int window_height );

          void update ( );
          void draw ( SDL_Surface* back_buffer );
          void handle_sdl_event ( const SDL_Event& sdl_event );

     private:

          mouse m_mouse;

          SDL_Surface* m_background;

          ui_text_button m_new_game_text;
          ui_text_button m_load_game_text;
          ui_text_button m_editor_text;
          ui_text_button m_options_text;
          ui_text_button m_quit_text;
     };
}

#endif

