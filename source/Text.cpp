#include "Text.hpp"
#include "Utils.hpp"
#include "GameMemory.hpp"
#include "Bitmap.hpp"

#include <ctype.h>

Bool Text::load_surfaces ( GameMemory& game_memory )
{
     if ( !load_bitmap_with_game_memory ( font_sheet, game_memory, "content/images/text.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( shadow_sheet, game_memory, "content/images/text_shadow.bmp" ) ) {
          return false;
     }

     character_width   = 5;
     character_height  = 8;
     character_spacing = 1;

     return true;
}

Void Text::unload ( )
{
     FREE_SURFACE ( font_sheet );
     FREE_SURFACE ( shadow_sheet );
}

Void Text::render ( SDL_Surface* back_buffer, const Char8* message, Int32 position_x, Int32 position_y,
                    Int32 character_count )
{
     render_impl ( back_buffer, font_sheet, message, position_x, position_y,
                   character_count );
}

Void Text::render_with_shadow ( SDL_Surface* back_buffer, const Char8* message,
                                Int32 position_x, Int32 position_y,
                                Int32 character_count )
{
     render_impl ( back_buffer, shadow_sheet, message, position_x + 1, position_y + 1,
                   character_count );
     render_impl ( back_buffer, font_sheet, message, position_x, position_y,
                   character_count );
}

Void Text::render_impl ( SDL_Surface* back_buffer, SDL_Surface* font_surface,
                         const Char8* message, Int32 position_x, Int32 position_y,
                         Int32 character_count )
{
     Char8 c;
     SDL_Rect dest { position_x, position_y, character_width, character_height };
     SDL_Rect clip { 0, 0, character_width, character_height };

     while ( ( c = *message ) && character_count != 0 ) {

          if ( isalpha ( c ) ) {
               clip.x = ( c - 'A' ) * character_width;
          } else if ( isdigit ( c ) ) {
               clip.x = ( ( c - '0' ) * character_width ) + ( ( ( 'Z' - 'A' ) + 1 ) * character_width );
          } else if ( c == '.' ) {
               clip.x = ( ( 'Z' - 'A' ) + ( '9' - '0' ) + 3 ) * character_width;
          } else if ( c == '+' ) {
               clip.x = ( ( 'Z' - 'A' ) + ( '9' - '0' ) + 5 ) * character_width;
          } else if ( c == ' ' ) {
               message++;
               dest.x += ( character_width + character_spacing );
               continue;
          }

          SDL_BlitSurface ( font_surface, &clip, back_buffer, &dest );

          message++;
          character_count--;
          dest.x += ( character_width + character_spacing );
     }
}

