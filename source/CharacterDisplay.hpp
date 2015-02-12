#ifndef BRYTE_CHARACTER_DISPLAY_HPP
#define BRYTE_CHARACTER_DISPLAY_HPP

#include "Enemy.hpp"

#include <SDL2/SDL.h>

namespace bryte {
     struct CharacterDisplay {

          CharacterDisplay ( );

          Void tick ( );

          Void render_player ( SDL_Surface* back_buffer, const Character& character,
                               Real32 camera_x, Real32 camera_y );

          Void render_enemy ( SDL_Surface* back_buffer, const Enemy& enemy,
                              Real32 camera_x, Real32 camera_y );

          SDL_Surface* enemy_sheets [ Enemy::Type::count ];
          SDL_Surface* player_sheet;

          SDL_Surface* horizontal_sword_sheet;
          SDL_Surface* vertical_sword_sheet;

          SDL_Surface* blink_surface;

          Bool  blink_on;
          Int32 blink_counter;

     };
};

#endif

