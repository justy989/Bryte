#ifndef TEXT_HPP
#define TEXT_HPP

#include "Types.hpp"

#include <SDL2/SDL.h>

class GameMemory;

struct Text {
public:

     Bool load_surfaces ( GameMemory& game_memory );
     Void unload ( );

     Void render ( SDL_Surface* back_buffer, const Char8* message, Int32 position_x, Int32 position_y,
                   Int32 character_count = -1 );
     Void render_with_shadow ( SDL_Surface* back_buffer, const Char8* message, Int32 position_x, Int32 position_y,
                               Int32 character_count = -1 );

private:

     Void render_impl ( SDL_Surface* back_buffer, SDL_Surface* font_surface,
                        const Char8* message, Int32 position_x, Int32 position_y,
                        Int32 character_count = -1 );

public:

     Int32 character_width;
     Int32 character_height;

     Int32 character_spacing;

     SDL_Surface* font_sheet;
     SDL_Surface* shadow_sheet;

};

#endif

