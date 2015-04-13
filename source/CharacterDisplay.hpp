#ifndef BRYTE_CHARACTER_DISPLAY_HPP
#define BRYTE_CHARACTER_DISPLAY_HPP

#include "Enemy.hpp"
#include "Animation.hpp"

#include <SDL2/SDL.h>

class GameMemory;

namespace bryte {
     struct CharacterDisplay {
     public:

          Bool load_surfaces ( GameMemory& game_memory );
          Void unload_surfaces ( );

          Void tick ( );

          Void render_player ( SDL_Surface* back_buffer, const Character& character,
                               Real32 camera_x, Real32 camera_y );

          Void render_enemy ( SDL_Surface* back_buffer, const Enemy& enemy,
                              Real32 camera_x, Real32 camera_y );

     private:

          Void render_character ( SDL_Surface* back_buffer, SDL_Surface* character_Sheet,
                                  const Character& character,
                                  SDL_Rect* dest_rect, SDL_Rect* clip_rect,
                                  Real32 camera_x, Real32 camera_y );

     public:

          static const Int32 fire_animation_delay = 5;
          static const Int32 fire_animation_max_frame = 3;

     public:

          SDL_Surface* enemy_sheets [ Enemy::Type::count ];
          SDL_Surface* player_sheet;

          SDL_Surface* horizontal_sword_sheet;
          SDL_Surface* vertical_sword_sheet;

          SDL_Surface* effect_surface;

          SDL_Surface* fire_surface;

          Animation fire_animation;

          Bool  blink_on;
          Int32 blink_counter;
     };
};

#endif

