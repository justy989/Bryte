/* title_state: allows starting a new game, loading a saved game, changing
 *               options, running the editor or quitting.
 */

#ifndef BRYTE_TITLE_STATE_HPP
#define BRYTE_TITLE_STATE_HPP

#include "surface_man.hpp"
#include "ui_button.hpp"

namespace bryte
{
     class title_state {
     public:
          title_state ( surface_man& sman );

          void update ( );
          void draw ( SDL_Surface* back_buffer );
          void handle_sdl_event ( const SDL_Event& sdl_event );

     private:

     private:

          SDL_Surface* m_background;
     };
}

#endif
