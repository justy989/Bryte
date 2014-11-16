#include "draw_utils.hpp"

using namespace bryte;

void draw_utils::draw_border ( const rectangle& outline, Uint32 color, SDL_Surface* back_buffer )
{
     // top bar
     SDL_Rect rect { outline.left ( ) - 1,
                     outline.bottom ( ) - 1,
                     outline.width ( ) + 2,
                     1 };
     SDL_FillRect ( back_buffer, &rect, color );

     // bottom bar
     rect = SDL_Rect { outline.left ( ) - 1,
                       outline.top ( ),
                       outline.width ( ) + 2,
                       1 };
     SDL_FillRect ( back_buffer, &rect, color );

     // left bar
     rect = SDL_Rect { outline.left ( ) - 1,
                       outline.bottom ( ) - 1,
                       1,
                       outline.height ( ) + 2 };
     SDL_FillRect ( back_buffer, &rect, color );

     // right bar
     rect = SDL_Rect { outline.right ( ),
                       outline.bottom ( ) - 1,
                       1,
                       outline.height ( ) + 2 };
     SDL_FillRect ( back_buffer, &rect, color );
}
