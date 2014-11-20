/* text: draw text to the back buffer, uses all static methods
 */

#ifndef BRYTE_TEXT_HPP
#define BRYTE_TEXT_HPP

#include <string>

#include "clipped_sprite.hpp"

namespace bryte
{
     class text {
     public:

          enum class color {
               black,
               white,
               red,
               orange,
               yellow,
               green,
               blue,
               purple,
               gray
          };

     public:

          static const int k_character_width = 5;
          static const int k_character_height = 8;

     public:

          static void create_sprite ( );
          static void destroy_sprite ( );

          static void draw ( SDL_Surface* back_buffer, const std::string& message,
                             vector position, color color = color::black );

     private:

          static void draw_character ( SDL_Surface* back_buffer, char character,
                                       vector position, color color );

     private:

          // not constructable
          text ( );

          static SDL_Surface* ms_surface;
          static clipped_sprite* ms_sprite;
     };
}

#endif
