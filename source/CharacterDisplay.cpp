#include "CharacterDisplay.hpp"
#include "Map.hpp"
#include "Utils.hpp"
#include "GameMemory.hpp"
#include "Bitmap.hpp"

using namespace bryte;

Bool CharacterDisplay::load_surfaces ( GameMemory& game_memory )
{
     if ( !load_bitmap_with_game_memory ( enemy_sheets [ Enemy::Type::rat ], game_memory,
                                          "test_rat.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( enemy_sheets [ Enemy::Type::bat ], game_memory,
                                          "test_bat.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( enemy_sheets [ Enemy::Type::goo ], game_memory,
                                          "test_goo.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( enemy_sheets [ Enemy::Type::skeleton ], game_memory,
                                          "test_skeleton.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( enemy_sheets [ Enemy::Type::fairy ], game_memory,
                                          "test_fairy.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( enemy_sheets [ Enemy::Type::knight ], game_memory,
                                          "test_knight.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( enemy_sheets [ Enemy::Type::knight ], game_memory,
                                          "test_spike.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( player_sheet, game_memory, "test_hero.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( vertical_sword_sheet, game_memory, "test_vertical_sword.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( fire_surface, game_memory, "test_effect_fire.bmp" ) ) {
          return false;
     }

     effect_surface = SDL_CreateRGBSurface ( 0, 32, 32, 32, 0, 0, 0, 0 );

     if ( !effect_surface ) {
          LOG_ERROR ( "Failed to create character display blink surface: SDL_CreateRGBSurface(): %s\n",
                      SDL_GetError ( ) );
          return false;
     }

     if ( SDL_SetColorKey ( effect_surface, SDL_TRUE,
                            SDL_MapRGB ( effect_surface->format, 255, 0, 255 ) ) ) {
          LOG_ERROR ( "Failed to set color key for character display blink surface SDL_SetColorKey() failed: %s\n",
                      SDL_GetError ( ) );
          return false;
     }

     if ( !load_bitmap_with_game_memory ( horizontal_sword_sheet, game_memory,
                                          "test_horizontal_sword.bmp" ) ) {
          return false;
     }

     return true;
}

Void CharacterDisplay::unload_surfaces ( )
{
     for ( Int32 i = 0; i < Enemy::Type::count; ++i ) {
          FREE_SURFACE ( enemy_sheets [ i ] );
     }

     FREE_SURFACE ( player_sheet );
     FREE_SURFACE ( horizontal_sword_sheet );
     FREE_SURFACE ( vertical_sword_sheet );
     FREE_SURFACE ( effect_surface );
     FREE_SURFACE ( fire_surface );
}

static Void set_surface_color ( SDL_Surface* surface, Uint8 red, Uint8 green, Uint8 blue )
{
     if ( SDL_LockSurface ( surface ) ) {
          return;
     }

     for ( Int32 y = 0; y < surface->h; ++y ) {
          for ( Int32 x = 0; x < surface->w; ++x ) {
               Uint32* p_pixel = reinterpret_cast< Uint32* >( surface->pixels ) + x + ( y * surface->w );

               Uint8* surface_blue   = reinterpret_cast< Uint8* >( p_pixel );
               Uint8* surface_green = surface_blue + 1;
               Uint8* surface_red  = surface_blue + 2;

               // skip magenta
               if ( *surface_red == 255 && *surface_green == 0 && *surface_blue == 255 ) {
                    continue;
               }

               // make it white
               *surface_red = red;
               *surface_blue = blue;
               *surface_green = green;
          }
     }

     SDL_UnlockSurface ( surface );
}

static void render_effect ( SDL_Surface* back_buffer, SDL_Surface* character_sheet, SDL_Surface* effect_surface,
                           SDL_Rect* dest_rect, SDL_Rect* clip_rect, Uint8 red, Uint8 green, Uint8 blue,
                           Real32 camera_x, Real32 camera_y )
{
     // clear the blink surface
     SDL_Rect clear_rect { 0, 0, effect_surface->w, effect_surface->h };
     Int32 clear_color = SDL_MapRGB ( effect_surface->format, 255, 0, 255 );

     SDL_FillRect ( effect_surface, &clear_rect, clear_color );

     // draw the character sheet onto the blink surface
     SDL_Rect blink_dest_rect { 0, 0, clip_rect->w, clip_rect->h };
     SDL_BlitSurface ( character_sheet, clip_rect, effect_surface, &blink_dest_rect );

     set_surface_color ( effect_surface, red, green, blue );

     // draw the blink surface to the back buffer
     SDL_BlitSurface ( effect_surface, &blink_dest_rect, back_buffer, dest_rect );
}

// NOTE: player only
static Void render_character_attack ( SDL_Surface* back_buffer, SDL_Surface* horizontal_attack_sheet,
                                      SDL_Surface* vertical_attack_sheet, const Character& character,
                                      Real32 camera_x, Real32 camera_y )
{
     SDL_Rect dest_rect = build_world_sdl_rect ( character.attack_x ( ), character.attack_y ( ), 0, 0 );
     SDL_Rect clip_rect { 0, 0, 9, 4 };
     SDL_Surface* attack_sheet = horizontal_attack_sheet;

     switch ( character.facing ) {
     default:
          ASSERT ( 0 );
          break;
     case Direction::left:
          dest_rect = build_world_sdl_rect ( character.position.x ( ) - pixels_to_meters ( clip_rect.w ),
                                             character.position.y ( ) + character.dimension.y ( ) * 0.2f,
                                             0.0f, 0.0f );
          break;
     case Direction::up:
          attack_sheet = vertical_attack_sheet;
          clip_rect.w = 4;
          clip_rect.h = 9;
          dest_rect = build_world_sdl_rect ( character.position.x ( ) + character.dimension.x ( ) * 0.5f,
                                             character.position.y ( ) + character.dimension.y ( ),
                                             0.0f, 0.0f );
          break;
     case Direction::right:
          clip_rect.x = 9;
          dest_rect = build_world_sdl_rect ( character.position.x ( ) + character.dimension.x ( ),
                                             character.position.y ( ) + character.dimension.y ( ) * 0.2f,
                                             0.0f, 0.0f );
          break;
     case Direction::down:
          attack_sheet = vertical_attack_sheet;
          clip_rect.w = 4;
          clip_rect.h = 9;
          clip_rect.y = 9;
          dest_rect = build_world_sdl_rect ( character.position.x ( ) + character.dimension.x ( ) * 0.3f,
                                             character.position.y ( ) - ( pixels_to_meters ( clip_rect.h ) * 0.8f ),
                                             0.0f, 0.0f );
          break;
     }

     dest_rect.w = clip_rect.w;
     dest_rect.h = clip_rect.h;

     world_to_sdl ( dest_rect, back_buffer, camera_x, camera_y );

     SDL_BlitSurface ( attack_sheet, &clip_rect, back_buffer, &dest_rect );
}

static Void render_on_fire ( SDL_Surface* back_buffer, SDL_Surface* fire_surface,
                             const Vector& position, Int32 frame, Real32 camera_x, Real32 camera_y )
{
     Int32 position_x = meters_to_pixels ( position.x ( ) );
     Int32 position_y = meters_to_pixels ( position.y ( ) );

     SDL_Rect dest_rect { position_x, position_y,
                          Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

     SDL_Rect clip_rect { frame * Map::c_tile_dimension_in_pixels, 0,
                          Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

     world_to_sdl ( dest_rect, back_buffer, camera_x, camera_y );

     SDL_BlitSurface ( fire_surface, &clip_rect, back_buffer, &dest_rect );

}

static Void render_character ( SDL_Surface* back_buffer, SDL_Surface* character_sheet,
                               SDL_Surface* effect_surface,
                               const Character& character,
                               Real32 camera_x, Real32 camera_y,
                               Bool blink_on )
{
     // do not draw
     if ( character.is_dead ( ) ) {
          return;
     }

     SDL_Rect dest_rect = build_world_sdl_rect ( character.position.x ( ), character.position.y ( ),
                                                 character.width ( ), character.height ( ) );

     SDL_Rect clip_rect = {
          character.walk_frame * Map::c_tile_dimension_in_pixels,
          0,
          Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels
     };

     if ( character.draw_facing ) {
          clip_rect.y = static_cast<Int32>( character.facing ) * Map::c_tile_dimension_in_pixels;
     }

     if ( character.state == Character::State::attacking ) {
          clip_rect.y += Direction::count * Map::c_tile_dimension_in_pixels;
          clip_rect.x = 0;
     } else if ( character.state == Character::State::blocking ) {
          clip_rect.y += Direction::count * Map::c_tile_dimension_in_pixels;
          clip_rect.x = Map::c_tile_dimension_in_pixels;
     }

     world_to_sdl ( dest_rect, back_buffer, camera_x, camera_y );

     if ( blink_on && character.is_blinking ( ) ) {
          if ( character.is_dying ( ) ) {
               render_effect ( back_buffer, character_sheet, effect_surface, &dest_rect, &clip_rect,
                              255, 255, 255, camera_x, camera_y );
          } else {
               render_effect ( back_buffer, character_sheet, effect_surface, &dest_rect, &clip_rect,
                               255, 0, 0, camera_x, camera_y );
          }
     } else if ( character.effected_by_element == Element::ice ) {
          render_effect ( back_buffer, character_sheet, effect_surface, &dest_rect, &clip_rect,
                          0, 0, 255, camera_x, camera_y );
     } else {
          SDL_BlitSurface ( character_sheet, &clip_rect, back_buffer, &dest_rect );
     }
}

Void CharacterDisplay::tick ( )
{
     static const Int32 blink_length = 7;

     if ( blink_counter <= 0 ) {
          blink_counter = blink_length;
          blink_on = !blink_on;
     } else {
          blink_counter--;
     }

     fire_animation.update_increment ( fire_animation_max_frame, fire_animation_delay );
}

Void CharacterDisplay::render_player ( SDL_Surface* back_buffer, const Character& player,
                                       Real32 camera_x, Real32 camera_y )
{
     if ( player.state == Character::State::attacking ) {
          render_character_attack ( back_buffer, horizontal_sword_sheet, vertical_sword_sheet,
                                    player, camera_x, camera_y );
     }

     render_character ( back_buffer, player_sheet, effect_surface,
                        player, camera_x, camera_y, blink_on );

     if ( player.effected_by_element == Element::fire ) {
          render_on_fire ( back_buffer, fire_surface, player.position, fire_animation.frame,
                           camera_x, camera_y );
     }
}

Void CharacterDisplay::render_enemy ( SDL_Surface* back_buffer, const Enemy& enemy,
                                      Real32 camera_x, Real32 camera_y )
{
     render_character ( back_buffer, enemy_sheets [ enemy.type ], effect_surface,
                        enemy, camera_x, camera_y, blink_on );

     if ( enemy.effected_by_element == Element::fire ) {
          render_on_fire ( back_buffer, fire_surface, enemy.position, fire_animation.frame,
                           camera_x, camera_y );
     }
}

