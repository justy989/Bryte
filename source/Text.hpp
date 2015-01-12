#ifndef TEXT_HPP
#define TEXT_HPP

#include "Types.hpp"

#include <SDL2/SDL.h>

struct Text {
     Int32 character_width;
     Int32 character_height;

     Int32 character_spacing;

     SDL_Surface* fontsheet;

     Void render ( SDL_Surface* back_buffer, const Char8* message, Int32 position_x, Int32 position_y );
};

#endif

