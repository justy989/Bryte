#include "Dialogue.hpp"
#include "Utils.hpp"

Void Dialogue::render ( SDL_Surface* back_buffer, Text* text,
                        const Char8* message )
{
     if ( state == Dialogue::State::none ) {
          return;
     }

     Int32 message_length = strlen ( message );
     Uint32 black = SDL_MapRGB ( back_buffer->format, 0, 0, 0 );
     Uint32 white = SDL_MapRGB ( back_buffer->format, 255, 255, 255 );

     // NOTE: This is duplicated in Text centering code
     Int32 left = ( back_buffer->w / 2 ) - ( ( message_length / 2 ) * text->character_width );
     Int32 message_width = message_length * ( text->character_width + text->character_spacing );

     // bg
     SDL_Rect left_line { left - 5, c_dialogue_height - 5, message_width + 10, text->character_height + 10 };
     SDL_FillRect ( back_buffer, &left_line, black );

     // border
     render_rect_outline ( back_buffer, left_line, white );

     // text
     text->render_centered_with_shadow ( back_buffer, message, c_dialogue_height,
                                         get_visible_characters ( ) );
}

