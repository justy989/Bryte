#include "CharacterDisplay.hpp"
#include "Map.hpp"
#include "Utils.hpp"

using namespace bryte;

CharacterDisplay::CharacterDisplay ( ) :
     player_sheet ( nullptr )
{
     for ( Int32 i = 0; i < Enemy::Type::count; ++i ) {
          enemy_sheets [ i ] = nullptr;
     }
}

static Void render_character ( SDL_Surface* back_buffer, SDL_Surface* character_sheet,
                               const Character& character,
                               Real32 camera_x, Real32 camera_y )
{
     static const Int32 blink_length  = 4;
     static Bool        blink_on      = false;
     static Int32       blink_count   = 0;

     // do not draw if dead
     if ( character.state == Character::State::dead ) {
          return;
     }

     // update blinking
     if ( blink_count <= 0 ) {
          blink_count = blink_length;
          blink_on = !blink_on;
     } else {
          blink_count--;
     }

     if ( !blink_on && character.state == Character::State::blinking ) {
          return;
     }

     SDL_Rect dest_rect = build_world_sdl_rect ( character.position.x ( ), character.position.y ( ),
                                                 character.width ( ), character.height ( ) );

     SDL_Rect clip_rect = {
          0, static_cast<Int32>( character.facing ) * Map::c_tile_dimension_in_pixels,
          Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels
     };

     world_to_sdl ( dest_rect, back_buffer, camera_x, camera_y );

     SDL_BlitSurface ( character_sheet, &clip_rect, back_buffer, &dest_rect );
}

Void CharacterDisplay::render_player ( SDL_Surface* back_buffer, const Character& player,
                                       Real32 camera_x, Real32 camera_y )
{
     render_character ( back_buffer, player_sheet, player, camera_x, camera_y );
}

Void CharacterDisplay::render_enemy ( SDL_Surface* back_buffer, const Enemy& enemy,
                                      Real32 camera_x, Real32 camera_y )
{
     render_character ( back_buffer, enemy_sheets [ enemy.type ], enemy, camera_x, camera_y );
}

