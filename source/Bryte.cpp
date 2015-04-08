#include "Bryte.hpp"
#include "Utils.hpp"
#include "Bitmap.hpp"
#include "Camera.hpp"
#include "MapDisplay.hpp"

#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <cstring>

using namespace bryte;

static Void render_bomb ( SDL_Surface* back_buffer, SDL_Surface* bomb_sheet, const Bomb& bomb,
                           Real32 camera_x, Real32 camera_y );
static Void render_particle ( SDL_Surface* back_buffer, const Particle& particle, Uint32 color,
                              Real32 camera_x, Real32 camera_y );
static void render_damage_number ( Text& text, SDL_Surface* back_buffer, const DamageNumber& damage_number,
                                   Real32 camera_x, Real32 camera_y );
static Void render_shown_pickup ( SDL_Surface* back_buffer, SDL_Surface* pickup_sheet,
                                  Character& player, Pickup::Type pickup_type,
                                  Real32 camera_x, Real32 camera_y );
static Void render_icon ( SDL_Surface* back_buffer, SDL_Surface* icon_sheet, Int32 frame, Int32 x, Int32 y );
static Void render_hearts ( SDL_Surface* back_buffer, SDL_Surface* heart_sheet, Int32 health, Int32 max_health,
                            Int32 x, Int32 y );

const Real32 State::c_pickup_show_time = 2.0f;

static State* get_state ( GameMemory& game_memory )
{
     return reinterpret_cast<MemoryLocations*>( game_memory.location ( ) )->state;
}

static Location character_adjacent_tile ( const Character& character )
{
     Location center_tile = Map::vector_to_location ( character.collision_center ( ) );

     switch ( character.facing ) {
     default:
          ASSERT ( 0 );
          break;
     case Direction::left:
          center_tile.x--;
          break;
     case Direction::right:
          center_tile.x++;
          break;
     case Direction::up:
          center_tile.y++;
          break;
     case Direction::down:
          center_tile.y--;
          break;
     }

     return center_tile;
}

static Bool character_touching_tile ( const Character& character, const Location& tile )
{
     static const Int32 corner_count = 4;

     Real32 left = character.position.x ( );
     Real32 bottom = character.position.y ( );
     Real32 right = left + Map::c_tile_dimension_in_meters;
     Real32 top = bottom + Map::c_tile_dimension_in_meters;

     Vector corners [ corner_count ] = {
          Vector { left, top },
          Vector { right, top },
          Vector { left, bottom },
          Vector { right, bottom },
     };

     for ( Int32 i = 0; i < corner_count; ++i ) {
          Auto corner_tile = Map::vector_to_location ( corners [ i ] );

          if ( corner_tile == tile ) {
               return true;
          }
     }

     return false;
}

static Location adjacent_tile ( Location loc, Direction dir )
{
     switch ( dir ) {
          default:
               break;
          case Direction::left:
               loc.x--;
               break;
          case Direction::right:
               loc.x++;
               break;
          case Direction::up:
               loc.y++;
               break;
          case Direction::down:
               loc.y--;
               break;
     }

     return loc;
}

Void UITextMenu::init ( Int32 x, Int32 y )
{
     top_left_x = x;
     top_left_y = y;

     option_count = 0;
     selected = 0;
}

Bool UITextMenu::add_option ( const Char8* text )
{
     if ( option_count >= c_max_option_count ) {
          LOG_ERROR ( "Failed to add option to text menu: '%s', too many options.\n", text );
          return false;
     }

     strncpy ( options [ option_count ], text, c_max_option_length );

     option_count++;

     return true;
}

Void UITextMenu::next_option ( )
{
     selected++;
     selected %= option_count;
}

Void UITextMenu::prev_option ( )
{
     selected--;
     if ( selected < 0 ) {
          selected = option_count - 1;
     }
}

Void UITextMenu::render ( SDL_Surface* back_buffer, Text* text )
{
     Int32 y = top_left_y;
     Int32 option_height = text->character_height + 4;
     Uint32 white = SDL_MapRGB ( back_buffer->format, 255, 255, 255 );

     for ( Int32 i = 0; i < option_count; ++i ) {
          text->render ( back_buffer, options [ i ], top_left_x, y );

          if ( i == selected ) {
               Int32 outline_width = ( static_cast<Int32>( strlen ( options [ i ] ) + 1 ) * text->character_width ) + 4;
               SDL_Rect outline_rect { top_left_x - 2, y - 2,
                                       outline_width,
                                       option_height };
               render_rect_outline ( back_buffer, outline_rect, white );
          }

          y += option_height;
     }
}

Bool State::initialize ( GameMemory& game_memory, Settings* settings )
{
     game_state = GameState::intro;

     this->settings = settings;

     random.seed ( 13371 );

     current_region = settings->region_index;

     player_spawn_tile.x = settings->player_spawn_tile_x;
     player_spawn_tile.y = settings->player_spawn_tile_y;

     player.clear ( );

     player.position = Map::location_to_vector ( player_spawn_tile );

     player.life_state = Entity::LifeState::alive;

     // reset keys
     for ( Int32 i = 0; i < 4; ++i ) {
          direction_keys [ i ] = false;
     }

     attack_key = false;
     item_key = false;
     switch_item_key = false;

     // clear entity managers
     pickups.clear ( );
     projectiles.clear ( );
     bombs.clear ( );
     emitters.clear ( );
     enemies.clear ( );
     damage_numbers.clear ( );

     // projectile collision for various directions
     Projectile::collision_points [ Direction::left ].set ( pixels_to_meters ( 1 ), pixels_to_meters ( 7 ) );
     Projectile::collision_points [ Direction::up ].set ( pixels_to_meters ( 7 ), pixels_to_meters ( 14 ) );
     Projectile::collision_points [ Direction::right ].set ( pixels_to_meters ( 14 ), pixels_to_meters ( 7 ) );
     Projectile::collision_points [ Direction::down ].set ( pixels_to_meters ( 7 ), pixels_to_meters ( 1 ) );

     // clear animations
     character_display.fire_animation.clear ( );
     interactives_display.animation.clear ( );
     pickup_display.animation.clear ( );
     projectile_display.animation.clear ( );
     interactives_display.clear ( );

     // clear pickup queue
     for ( Int32 i = 0; i < c_pickup_queue_size; ++i ) {
          pickup_queue [ i ] = Pickup::Type::none;
     }

     pickup_stopwatch.reset ( c_pickup_show_time );

     // load title sheet
     if ( !load_bitmap_with_game_memory ( title_surface, game_memory, "content/images/title_screen.bmp" ) ) {
          return false;
     }

     // load font
     if ( !text.load_surfaces ( game_memory ) ) {
          return false;
     }

     // NOTE: init map display textures to null so we don't clean them up
     // if they aren't loaded
     map_display.clear ( );

     // Load our necessary surfaces
     if ( !character_display.load_surfaces ( game_memory ) ) {
          return false;
     }

     if ( !pickup_display.load_surfaces ( game_memory ) ) {
          return false;
     }

     if ( !projectile_display.load_surfaces ( game_memory ) ) {
          return false;
     }

     // load misc surfaces
     if ( !load_bitmap_with_game_memory ( bomb_sheet, game_memory,
                                          "content/images/test_bomb.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( attack_icon_sheet, game_memory,
                                          "content/images/test_attack_icon.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( player_heart_sheet, game_memory,
                                          "content/images/player_heart.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( upgrade_sheet, game_memory,
                                          "content/images/player_upgrade.bmp" ) ) {
          return false;
     }

     // load sound effects
     if ( !sound.load_effects ( ) ) {
          return false;
     }

     back_buffer_format = *bomb_sheet->format;

     slot_menu.init ( 154, 122 );
     slot_menu.add_option ( "SLOT 0" );
     slot_menu.add_option ( "SLOT 1" );
     slot_menu.add_option ( "SLOT 2" );
     slot_menu.add_option ( "QUIT" );

     pause_menu.init ( 154, 122 );
     pause_menu.add_option ( "RESUME" );
     pause_menu.add_option ( "SAVE" );
     pause_menu.add_option ( "MENU" );

#ifdef DEBUG
     enemy_think = true;
     invincible = false;
     debug_text = true;
#endif

     return true;
}

Void State::destroy ( )
{
     sound.unload_effects ( );

     map_display.unload_surfaces ( );
     character_display.unload_surfaces ( );
     interactives_display.unload_surfaces ( );
     pickup_display.unload_surfaces ( );
     projectile_display.unload_surfaces ( );

     SDL_FreeSurface ( bomb_sheet );
     SDL_FreeSurface ( attack_icon_sheet );
     SDL_FreeSurface ( player_heart_sheet );
}

Void State::update ( GameMemory& game_memory, Real32 time_delta )
{
     switch ( game_state ) {
     default:
          break;
     case GameState::intro:
          update_intro ( game_memory, time_delta );
          break;
     case GameState::game:
          update_game ( game_memory, time_delta );
          break;
     case GameState::pause:
          update_pause ( game_memory, time_delta );
          break;
     }
}

Void State::handle_input ( GameMemory& game_memory, const GameInput& game_input )
{
     switch ( game_state ) {
     default:
          break;
     case GameState::intro:
          handle_intro_input ( game_memory, game_input );
          break;
     case GameState::game:
          handle_game_input ( game_memory, game_input );
          break;
     case GameState::pause:
          handle_pause_input ( game_memory, game_input );
          break;
     }
}

Void State::render ( GameMemory& game_memory, SDL_Surface* back_buffer )
{
     switch ( game_state ) {
     default:
          break;
     case GameState::intro:
          render_intro ( game_memory, back_buffer );
          break;
     case GameState::game:
          render_game ( game_memory, back_buffer );
          break;
     case GameState::pause:
          render_pause ( game_memory, back_buffer );
          break;
     }
}

Void State::quit_game ( )
{
     LOG_INFO ( "Quitting\n" );

     SDL_Event sdl_event;
     sdl_event.type = SDL_QUIT;
     SDL_PushEvent ( &sdl_event );
}

Void State::update_intro ( GameMemory& game_memory, Real32 time_delta )
{

}

Void State::update_game ( GameMemory& game_memory, Real32 time_delta )
{
     if ( dialogue.get_state ( ) == Dialogue::State::none ) {
          update_player ( game_memory, time_delta );
          update_enemies ( time_delta );
          update_interactives ( time_delta );
          update_projectiles ( time_delta );
          update_bombs ( time_delta );
          update_pickups ( time_delta );
          update_emitters ( time_delta );
          update_damage_numbers ( time_delta );
     } else {
          dialogue.tick ( map.dialogue ( ) );
     }

     update_light ( );
}

Void State::update_pause ( GameMemory& game_memory, Real32 time_delta )
{

}

Void State::handle_intro_input ( GameMemory& game_memory, const GameInput& game_input )
{
     // handle keyboard
     for ( Uint32 i = 0; i < game_input.key_change_count; ++i ) {
          const GameInput::KeyChange& key_change = game_input.key_changes [ i ];

          switch ( key_change.scan_code ) {
          default:
               break;
          case SDL_SCANCODE_W:
          case SDL_SCANCODE_UP:
               if ( key_change.down ) {
                    slot_menu.prev_option ( );
               }
               break;
          case SDL_SCANCODE_S:
          case SDL_SCANCODE_DOWN:
               if ( key_change.down ) {
                    slot_menu.next_option ( );
               }
               break;
          case SDL_SCANCODE_RETURN:
               if ( key_change.down ) {
                    start_game ( game_memory );
               }
               break;
          }
     }

     // handle controller
     for ( Uint32 i = 0; i < game_input.controller_button_change_count; ++i ) {
          const GameInput::ButtonChange& btn_change = game_input.controller_button_changes [ i ];

          switch ( btn_change.button ) {
               default:
                    break;
               case SDL_CONTROLLER_BUTTON_DPAD_UP:
                    if ( btn_change.down ) {
                         slot_menu.prev_option ( );
                    }
                    break;
               case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                    if ( btn_change.down ) {
                         slot_menu.next_option ( );
                    }
                    break;
               case SDL_CONTROLLER_BUTTON_A:
                    if ( btn_change.down ) {
                         start_game ( game_memory );
                    }
                    break;
          }
     }
}

Void State::handle_game_input ( GameMemory& game_memory, const GameInput& game_input )
{
     // handle keyboard
     for ( Uint32 i = 0; i < game_input.key_change_count; ++i ) {
          const GameInput::KeyChange& key_change = game_input.key_changes [ i ];

          switch ( key_change.scan_code ) {
          default:
               break;
          case SDL_SCANCODE_W:
               direction_keys [ Direction::up ]    = key_change.down;
               break;
          case SDL_SCANCODE_S:
               direction_keys [ Direction::down ]  = key_change.down;
               break;
          case SDL_SCANCODE_A:
               direction_keys [ Direction::left ]  = key_change.down;
               break;
          case SDL_SCANCODE_D:
               direction_keys [ Direction::right ] = key_change.down;
               break;
          case SDL_SCANCODE_Q:
               switch_item_key = key_change.down;
               break;
          case SDL_SCANCODE_SPACE:
               if ( dialogue.get_state ( ) == Dialogue::State::printing ) {
                    if ( key_change.down ) {
                         dialogue.force_done ( map.dialogue ( ) );
                    }
               } else if ( dialogue.get_state ( ) == Dialogue::State::done ) {
                    if ( key_change.down ) {
                         dialogue.reset ( );
                    }
               } else {
                    attack_key = key_change.down;
               }
               break;
          case SDL_SCANCODE_LCTRL:
               item_key = key_change.down;
               break;
          case SDL_SCANCODE_E:
               activate_key = key_change.down;
               break;
          case SDL_SCANCODE_P:
               game_state = GameState::pause;
               return;

// NOTE: Debug only keys for cheating!
#ifdef DEBUG
          case SDL_SCANCODE_7:
               if ( key_change.down ) {
                    Vector spawn_pos = player.position - Vector ( Map::c_tile_dimension_in_meters, 0.0f );
                    spawn_enemy ( spawn_pos, 0, player.facing, Pickup::Type::health );
               }
               break;
          case SDL_SCANCODE_X:
               if ( key_change.down ) {
                    enemy_think = !enemy_think;
               }
               break;
          case SDL_SCANCODE_Z:
               if ( key_change.down ) {
                    invincible = !invincible;
               }
               break;
          case SDL_SCANCODE_C:
               if ( key_change.down ) {
                    player.key_count++;
               }
               break;
          case SDL_SCANCODE_V:
               if ( key_change.down ) {
                    player.give_arrow ( );
               }
               break;
          case SDL_SCANCODE_B:
               if ( key_change.down ) {
                    player.give_bomb ( );
               }
               break;
          case SDL_SCANCODE_T:
               if ( key_change.down ) {
                    debug_text = !debug_text;
               }
               break;
          case SDL_SCANCODE_8:
               if ( key_change.down ) {
                    player_save ( );
               }
               break;
          case SDL_SCANCODE_9:
               if ( key_change.down ) {
                    player_load ( );
               }
               break;
#endif
          }
     }

     // handle controller
     for ( Uint32 i = 0; i < game_input.controller_button_change_count; ++i ) {
          const GameInput::ButtonChange& btn_change = game_input.controller_button_changes [ i ];

          switch ( btn_change.button ) {
               default:
                    break;
               case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                    direction_keys [ Direction::left ] = btn_change.down;
                    break;
               case SDL_CONTROLLER_BUTTON_DPAD_UP:
                    direction_keys [ Direction::up ] = btn_change.down;
                    break;
               case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                    direction_keys [ Direction::right ] = btn_change.down;
                    break;
               case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                    direction_keys [ Direction::down ] = btn_change.down;
                    break;
               case SDL_CONTROLLER_BUTTON_A:
                    if ( dialogue.get_state ( ) == Dialogue::State::printing ) {
                         if ( btn_change.down ) {
                              dialogue.force_done ( map.dialogue ( ) );
                         }
                    } else if ( dialogue.get_state ( ) == Dialogue::State::done ) {
                         if ( btn_change.down ) {
                              dialogue.reset ( );
                         }
                    } else {
                         attack_key = btn_change.down;
                    }
                    break;
               case SDL_CONTROLLER_BUTTON_B:
                    item_key = btn_change.down;
                    break;
               case SDL_CONTROLLER_BUTTON_X:
                    activate_key = btn_change.down;
                    break;
               case SDL_CONTROLLER_BUTTON_Y:
                    switch_item_key = btn_change.down;
                    break;
               case SDL_CONTROLLER_BUTTON_START:
                    game_state = GameState::pause;
                    return;
          }
     }

// NOTE: Clicking in debug mode places the player
#ifdef DEBUG
     for ( Uint32 i = 0; i < game_input.mouse_button_change_count; ++i ) {
          Auto change = game_input.mouse_button_changes [ i ];

          switch ( change.button ) {
          default:
               break;
          case SDL_BUTTON_LEFT:
          {
               if ( change.down ) {
                    Int32 mouse_screen_x = game_input.mouse_position_x - meters_to_pixels ( camera.x ( ) );
                    Int32 mouse_screen_y = game_input.mouse_position_y - meters_to_pixels ( camera.y ( ) );

                    player.set_collision_center ( pixels_to_meters ( mouse_screen_x ),
                                                  pixels_to_meters ( mouse_screen_y ) );
               }
          } break;
          }
     }
#endif
}

Void State::handle_pause_input ( GameMemory& game_memory, const GameInput& game_input )
{
     // handle keyboard
     for ( Uint32 i = 0; i < game_input.key_change_count; ++i ) {
          const GameInput::KeyChange& key_change = game_input.key_changes [ i ];

          switch ( key_change.scan_code ) {
          default:
               break;
          case SDL_SCANCODE_W:
          case SDL_SCANCODE_UP:
               if ( key_change.down ) {
                    pause_menu.prev_option ( );
               }
               break;
          case SDL_SCANCODE_S:
          case SDL_SCANCODE_DOWN:
               if ( key_change.down ) {
                    pause_menu.next_option ( );
               }
               break;
          case SDL_SCANCODE_RETURN:
               if ( key_change.down ) {
                    switch ( pause_menu.selected ) {
                    default:
                         break;
                    case 0:
                         game_state = GameState::game;
                         break;
                    case 1:
                         player_save ( );
                         game_state = GameState::game;
                         break;
                    case 2:
                         game_state = GameState::intro;
                         break;
                    }
               }
               break;
          }
     }

     // handle controller
     for ( Uint32 i = 0; i < game_input.controller_button_change_count; ++i ) {
          const GameInput::ButtonChange& btn_change = game_input.controller_button_changes [ i ];

          switch ( btn_change.button ) {
          default:
               break;
          case SDL_CONTROLLER_BUTTON_DPAD_UP:
               if ( btn_change.down ) {
                    pause_menu.prev_option ( );
               }
               break;
          case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
               if ( btn_change.down ) {
                    pause_menu.next_option ( );
               }
               break;
          case SDL_CONTROLLER_BUTTON_A:
               if ( btn_change.down ) {
                    switch ( pause_menu.selected ) {
                         default:
                              break;
                         case 0:
                              game_state = GameState::game;
                              break;
                         case 1:
                              player_save ( );
                              game_state = GameState::game;
                              break;
                         case 2:
                              game_state = GameState::intro;
                              break;
                    }
               }
               break;
          }
     }
}

Void State::render_intro ( GameMemory& game_memory, SDL_Surface* back_buffer )
{
     SDL_Rect title_src { 0, 0, 128, 120 };
     SDL_Rect title_dst { 0, 0, back_buffer->w, back_buffer->h };

     SDL_BlitScaled ( title_surface, &title_src, back_buffer, &title_dst );

     slot_menu.render ( back_buffer, &text );
}

Void State::render_game ( GameMemory& game_memory, SDL_Surface* back_buffer )
{
     Uint32 black  = SDL_MapRGB ( back_buffer->format, 0, 0, 0 );

     back_buffer_format = *back_buffer->format;

     // calculate camera
     camera.set_x ( calculate_camera_position ( back_buffer->w, map.width ( ),
                                                       player.position.x ( ), player.width ( ) ) );
     camera.set_y ( calculate_camera_position ( back_buffer->h - Map::c_tile_dimension_in_pixels, map.height ( ),
                                                       player.position.y ( ), player.height ( ) ) );

     // map
     map_display.tick ( );
     map_display.render ( back_buffer, map, camera.x ( ), camera.y ( ),
                          map.found_secret ( ) );

     // interactives
     interactives_display.tick ( );
     interactives_display.render ( back_buffer, interactives, map,
                                          camera.x ( ), camera.y ( ), map.found_secret ( ) );

     character_display.tick ( );

     // enemies in 2 passes, non-flying and flying
     for ( Uint32 i = 0; i < enemies.max ( ); ++i ) {
          Auto& enemy = enemies [ i ];
          if ( enemy.is_dead ( ) || enemy.flies ) {
               continue;
          }

          character_display.render_enemy ( back_buffer, enemy,
                                                  camera.x ( ), camera.y ( ) );
     }

     // player
     character_display.render_player ( back_buffer, player,
                                              camera.x ( ), camera.y ( ) );

     for ( Uint32 i = 0; i < enemies.max ( ); ++i ) {
          Auto& enemy = enemies [ i ];
          if ( enemy.is_dead ( ) || !enemy.flies ) {
               continue;
          }

          character_display.render_enemy ( back_buffer, enemy,
                                                  camera.x ( ), camera.y ( ) );
     }

     // pickups
     pickup_display.tick ( );
     for ( Uint32 i = 0; i < pickups.max ( ); ++i ) {
          Auto& pickup = pickups [ i ];

          if ( pickup.is_dead ( ) ) {
               continue;
          }

          pickup_display.render ( back_buffer, pickup, camera.x ( ), camera.y ( ) );
     }

     // projectiles
     projectile_display.tick ( );

     for ( Uint32 i = 0; i < projectiles.max ( ); ++i ) {
          Auto& projectile = projectiles [ i ];

          if ( projectile.is_dead ( ) ) {
               continue;
          }

          projectile_display.render ( back_buffer, projectile,
                                             camera.x ( ), camera.y ( ) );
     }

     // bombs
     for ( Uint32 i = 0; i < bombs.max ( ); ++i ) {
          Auto& bomb = bombs [ i ];

          if ( bomb.is_dead ( ) ) {
               continue;
          }

          render_bomb ( back_buffer, bomb_sheet, bomb, camera.x ( ), camera.y ( ) );
     }

     // emitters
     for ( Uint32 i = 0; i < emitters.max ( ); ++i ) {
          Auto& emitter = emitters [ i ];
          if ( emitter.is_alive ( ) ) {
               for ( Uint8 i = 0; i < Emitter::c_max_particles; ++i ) {
                    Auto& particle_lifetime_watch = emitter.particle_lifetime_watches [ i ];
                    if ( !particle_lifetime_watch.expired ( ) ) {
                         render_particle ( back_buffer, emitter.particles [ i ], emitter.color,
                                           camera.x ( ), camera.y ( ) );
                    }
               }
          }
     }

     render_upgrade ( back_buffer );

     // pickup queue
     if ( pickup_queue [ 0 ] ) {
          render_shown_pickup ( back_buffer, pickup_display.pickup_sheet, player,
                                pickup_queue [ 0 ], camera.x ( ), camera.y ( ) );
     }

     // light
     render_light ( back_buffer, map, camera.x ( ), camera.y ( ) );

     // damage numbers
     for ( Uint32 i = 0; i < damage_numbers.max ( ); ++i ) {
          Auto& damage_number = damage_numbers [ i ];

          if ( damage_number.is_dead ( ) ) {
               continue;
          }

          render_damage_number ( text, back_buffer, damage_number,
                                 camera.x ( ), camera.y ( ) );
     }

     if ( dialogue.get_state ( ) == Dialogue::State::printing ||
          dialogue.get_state ( ) == Dialogue::State::done  ) {
          text.render ( back_buffer, map.dialogue ( ), 50, 230,
                        dialogue.get_visible_characters ( ) );
     }

     // ui
     SDL_Rect hud_rect { 0, 0, back_buffer->w, Map::c_tile_dimension_in_pixels };
     SDL_FillRect ( back_buffer, &hud_rect, black );

     render_hearts ( back_buffer, player_heart_sheet, player.health, player.max_health,
                     2, 2 );

     render_icon ( back_buffer, attack_icon_sheet, 0,
                   ( back_buffer->w / 2 ) - Map::c_tile_dimension_in_pixels, 1 );

     render_icon ( back_buffer, attack_icon_sheet, player.item_mode + 1,
                   ( back_buffer->w / 2 ) + 3, 1 );

     char buffer [ 64 ];

     sprintf ( buffer, "%d", player.key_count );
     text.render ( back_buffer, buffer, 235, 4 );

     sprintf ( buffer, "%d", player.bomb_count );
     text.render ( back_buffer, buffer, 210, 4 );

     sprintf ( buffer, "%d", player.arrow_count );
     text.render ( back_buffer, buffer, 185, 4 );

     SDL_Rect pickup_dest_rect { 225, 3, Pickup::c_dimension_in_pixels, Pickup::c_dimension_in_pixels };
     SDL_Rect pickup_clip_rect { 0, Pickup::c_dimension_in_pixels, Pickup::c_dimension_in_pixels, Pickup::c_dimension_in_pixels };

     SDL_BlitSurface ( pickup_display.pickup_sheet, &pickup_clip_rect, back_buffer, &pickup_dest_rect );

     SDL_Rect bomb_dest_rect { 200, 3, Pickup::c_dimension_in_pixels, Pickup::c_dimension_in_pixels };
     SDL_Rect bomb_clip_rect { 0, Pickup::c_dimension_in_pixels * 3, Pickup::c_dimension_in_pixels, Pickup::c_dimension_in_pixels };

     SDL_BlitSurface ( pickup_display.pickup_sheet, &bomb_clip_rect, back_buffer, &bomb_dest_rect );

     SDL_Rect arrow_dest_rect { 175, 3, Pickup::c_dimension_in_pixels, Pickup::c_dimension_in_pixels };
     SDL_Rect arrow_clip_rect { 0, Pickup::c_dimension_in_pixels * 2, Pickup::c_dimension_in_pixels, Pickup::c_dimension_in_pixels };

     SDL_BlitSurface ( pickup_display.pickup_sheet, &arrow_clip_rect, back_buffer, &arrow_dest_rect );

#ifdef DEBUG
     if ( debug_text ) {
          Auto player_loc = Map::vector_to_location ( player.position );

          sprintf ( buffer, "P %.2f %.2f  T %d %d  M %d  AI %s  INV %s",
                    player.position.x ( ), player.position.y ( ),
                    player_loc.x, player_loc.y,
                    map.current_master_map ( ),
                    enemy_think ? "ON" : "OFF",
                    invincible ? "ON" : "OFF" );

          text.render ( back_buffer, buffer, 0, 230 );
     }
#endif
}

Void State::render_pause ( GameMemory& game_memory, SDL_Surface* back_buffer )
{
     SDL_Rect title_src { 0, 0, 128, 120 };
     SDL_Rect title_dst { 0, 0, back_buffer->w, back_buffer->h };

     SDL_BlitScaled ( title_surface, &title_src, back_buffer, &title_dst );

     pause_menu.render ( back_buffer, &text );
}

Bool State::spawn_enemy ( const Vector& position, Uint8 id, Direction facing, Pickup::Type drop )
{
     Enemy* enemy = enemies.spawn ( position );

     if ( !enemy ) {
          return false;
     }

     ASSERT ( id < Enemy::Type::count );

     LOG_DEBUG ( "Spawning enemy %s at: %f, %f\n", Enemy::c_names [ id ], position.x ( ), position.y ( ) );

     enemy->init ( static_cast<Enemy::Type>( id ), position.x ( ), position.y ( ), facing, drop );

     return true;
}

Bool State::spawn_pickup ( const Vector& position, Pickup::Type type )
{
     Auto* pickup = pickups.spawn ( position );

     if ( !pickup ) {
          return false;
     }

     pickup->type = type;

     LOG_DEBUG ( "Spawn pickup %s at %f, %f\n", Pickup::c_names [ type ], position.x ( ), position.y ( ) );

     return true;
}

bool State::spawn_projectile ( Projectile::Type type, const Vector& position, Direction facing,
                               Projectile::Alliance alliance )
{
     static const Real32 c_projectile_offset_scale = Map::c_tile_dimension_in_meters * 0.5f;
     Vector offset = vector_from_direction ( facing ) * c_projectile_offset_scale;
     Auto* projectile = projectiles.spawn ( position + offset );

     if ( !projectile ) {
          return false;
     }

     projectile->type = type;
     projectile->facing = facing;
     projectile->effected_by_element = Element::none;
     projectile->alliance = alliance;

     LOG_DEBUG ( "spawning projectile at %f, %f\n", position.x ( ), position.y ( ) );

#if 0
     Auto* emitter = emitters.spawn ( position );

     if ( emitter ) {
          emitter->setup_to_track_entity ( arrow, Projectile::collision_points [ facing ],
                                           SDL_MapRGB ( &back_buffer_format, 255, 255, 255 ),
                                           0.0f, 0.0f, 0.5f, 0.5f, 0.1f, 0.1f, 1, 2 );
     }
#endif

     return true;
}

Bool State::spawn_bomb ( const Vector& position )
{
     Auto* bomb = bombs.spawn ( position );

     if ( !bomb ) {
          return false;
     }

     bomb->explode_watch.reset ( Bomb::c_explode_time );

     LOG_DEBUG ( "spawning bomb at %f, %f\n", position.x ( ), position.y ( ) );

     Auto* emitter = emitters.spawn ( position );

     if ( emitter ) {
          Vector offset { Map::c_tile_dimension_in_meters * 0.5f, Map::c_tile_dimension_in_meters };
          emitter->setup_to_track_entity ( bomb, offset,
                                           SDL_MapRGB ( &back_buffer_format, 255, 255, 0 ),
                                           0.785f, 2.356f, 1.0f, 1.0f, 0.5f, 0.5f, 1, 10 );
     }

     return true;
}

Bool State::spawn_damage_number ( const Vector& position, Int32 value )
{
     Auto* damage_number = damage_numbers.spawn ( position );

     if ( !damage_number ) {
          return false;
     }

     damage_number->value = -value;
     damage_number->starting_y = position.y ( );

     // NOTE: seriously?
     LOG_DEBUG ( "spawning damage number at %f, %f\n", position.x ( ), position.y ( ) );

     return true;
}

Bool State::spawn_healing_number ( const Vector& position, Int32 value )
{
     Auto* damage_number = damage_numbers.spawn ( position );

     if ( !damage_number ) {
          return false;
     }

     damage_number->value = value;
     damage_number->starting_y = position.y ( );

     // NOTE: seriously?
     LOG_DEBUG ( "spawning damage number at %f, %f\n", position.x ( ), position.y ( ) );

     return true;
}

Void State::start_game ( GameMemory& game_memory )
{
     if ( slot_menu.selected == 3 ) {
          quit_game ( );
          return;
     }

     game_state = GameState::game;

     if ( !load_region ( game_memory ) ) {
          quit_game ( );
          return;
     }

     player.clear ( );
     player.save_slot = static_cast<Uint8>( slot_menu.selected );
     player_load ( );

     player.position = Map::location_to_vector ( player_spawn_tile );

     player.life_state = Entity::LifeState::alive;

     change_map ( settings->map_index, false );
}

Bool State::load_region ( GameMemory& game_memory )
{
     LOG_INFO ( "Loading region %d\n", current_region );

     // load the region info
     if ( !region.load_info( current_region ) ) {
          return false;
     }

     // load diplay surfaces
     if ( !map_display.load_surfaces ( game_memory,
                                       region.tilesheet_filepath,
                                       region.decorsheet_filepath,
                                       region.lampsheet_filepath ) ) {
          return false;
     }

     interactives_display.unload_surfaces ( );
     if ( !interactives_display.load_surfaces  ( game_memory, region.exitsheet_filepath ) ) {
          return false;
     }

     // load map
     map.load_master_list ( region.map_list_filepath );

     return true;
}

Void State::persist_map ( )
{
     // persist map exits
     LOG_DEBUG ( "Persisting exits for map: %d\n", map.current_master_map ( ) );

     for ( Uint8 y = 0; y < interactives.height ( ); ++y ) {
          for ( Uint8 x = 0; x < interactives.width ( ); ++x ) {
               Location tile ( x, y );

               const Auto& exit = interactives.cget_from_tile ( tile );

               if ( exit.type != Interactive::Type::exit ) {
                    continue;
               }

               map.persist_exit ( exit, Coordinates ( x, y ) );
          }
     }

     // persist map enemies
     for ( Uint32 c = 0; c < enemies.max ( ); ++c ) {
          Auto& enemy = enemies [ c ];

          map.persist_enemy ( enemy, c );
     }

     map.persist_secret ( );
     map.persist_killed_all_enemies ( );
}

Void State::spawn_map_enemies ( )
{
     for ( Int32 i = 0; i < map.enemy_spawn_count ( ); ++i ) {
          Auto& enemy_spawn = map.enemy_spawn ( i );

          Vector position = Map::location_to_vector ( Location ( enemy_spawn.coordinates.x,
                                                                 enemy_spawn.coordinates.y ) );

          spawn_enemy ( position, enemy_spawn.id, enemy_spawn.facing, enemy_spawn.drop );
     }
}

Void State::player_death ( )
{
     player.clear ( );

     player.health = 6;
     player.life_state = Entity::LifeState::alive;
     player.position = Map::location_to_vector ( player_spawn_tile );

     pickups.clear ( );
     projectiles.clear ( );
     emitters.clear ( );
     enemies.clear ( );
     damage_numbers.clear ( );

     map.load_from_master_list ( 0, interactives );

     setup_emitters_from_map_lamps ( );
     spawn_map_enemies ( );
}

Bool State::check_player_block_projectile ( Projectile& projectile )
{
     if ( player.is_blocking ( ) &&
          player.facing == opposite_direction ( projectile.facing ) ) {
          switch ( projectile.type ) {
          default:
               break;
          case Projectile::goo:
               projectile.facing = opposite_direction ( projectile.facing );
               projectile.alliance = Projectile::Alliance::good;
               break;
          case Projectile::arrow:
               projectile.track_entity.entity = &player;
               projectile.track_entity.offset = projectile.position - player.position;
               projectile.stuck_watch.reset ( Projectile::c_stuck_time );
               break;
          }

          return true;
     }

     return false;
}

Void State::enemy_death ( const Enemy& enemy )
{
     if ( enemy.drop ) {
          spawn_pickup ( enemy.position, enemy.drop );
     }

     Auto* emitter = emitters.spawn ( enemy.collision_center ( ) );

     if ( emitter ) {
          Real32 explosion_size = enemy.collision_width ( ) > enemy.collision_height ( ) ?
                                  enemy.collision_width ( ) : enemy.collision_height ( );
          explosion_size *= 2.0f;
          emitter->setup_limited_time ( enemy.collision_center ( ), 0.7f,
                                        SDL_MapRGB ( &back_buffer_format, 255, 0, 0 ),
                                        0.0f, 6.28f, 0.3f, 0.7f, 0.25f, explosion_size,
                                        Emitter::c_max_particles, 0 );
     }

     // TODO: track entity count so we don't have to do this linear check
     bool all_dead = true;

     for ( Uint8 i = 0; i < enemies.max ( ); ++i ) {
          if ( enemies [ i ].is_alive ( ) ) {
               all_dead = false;
               break;
          }
     }

     // if all the entities are dead, activate an the map's trigger
     if ( all_dead ) {
          Location loc ( map.activate_on_all_enemies_killed ( ) );
          interactives.activate ( loc );
          map.killed_all_enemies ( );
     }
}

Void State::setup_emitters_from_map_lamps ( )
{
     for ( Uint8 i = 0; i < map.lamp_count ( ); ++i ) {
          Auto& lamp = map.lamp ( i );

          Vector position = map.location_to_vector ( Location ( lamp.coordinates.x,
                                                                lamp.coordinates.y ) );

          Vector offset { Map::c_tile_dimension_in_meters * 0.4f,
                          Map::c_tile_dimension_in_meters * 0.7f };

          Auto* emitter = emitters.spawn ( position );

          if ( !emitter ) {
               break;
          }

          Real32 min_angle = 0.78f;
          Real32 max_angle = 2.35f;

          switch ( lamp.id ) {
          default:
               break;
          case 1:
               offset.set_x ( Map::c_tile_dimension_in_meters * 0.3f );
               offset.set_y ( Map::c_tile_dimension_in_meters * 0.5f );
               min_angle = 2.35f;
               max_angle = 3.92f;
               break;
          case 3:
               offset.set_x ( Map::c_tile_dimension_in_meters * 0.7f );
               offset.set_y ( Map::c_tile_dimension_in_meters * 0.5f );
               min_angle = 5.49f;
               max_angle = 7.06f;
               break;
          case 4:
               offset.set_x ( Map::c_tile_dimension_in_meters * 0.5f );
               offset.set_y ( Map::c_tile_dimension_in_meters * 0.3f );
               min_angle = 3.92f;
               max_angle = 5.49f;
               break;
          }

          emitter->setup_immortal ( position + offset, SDL_MapRGB ( &back_buffer_format, 255, 187, 0 ),
                                    min_angle, max_angle, 0.5f, 0.75f, 0.5f, 1.0f, 1, 10 );
     }
}

Void State::tick_character_element ( Character& character )
{
     switch ( character.effected_by_element ) {
     default:
          break;
     case Element::fire:

          if ( character.element_watch.expired ( ) ) {
               character.fire_tick_count++;

               if ( character.fire_tick_count >= Character::c_fire_tick_max ) {
                    // effect has expired
                    character.effected_by_element = Element::none;
               } else {
                    character.element_watch.reset ( Character::c_fire_tick_rate );
               }

               // push character in a random direction
               Direction dir = static_cast<Direction>( random.generate ( 0, Direction::count ) );

               damage_character ( character, c_burn_damage, dir );
          }
          break;
     }
}

Void State::damage_character ( Character& character, Int32 amount, Direction direction )
{
     character.damage ( amount, direction );
     spawn_damage_number ( character.collision_center ( ), amount );
}

Void State::update_player ( GameMemory& game_memory, float time_delta )
{
     if ( direction_keys [ Direction::up ] ) {
          player.walk ( Direction::up );
     }

     if ( direction_keys [ Direction::down ] ) {
          player.walk ( Direction::down );
     }

     if ( direction_keys [ Direction::right ] ) {
          player.walk ( Direction::right );
     }

     if ( direction_keys [ Direction::left ] ) {
          player.walk ( Direction::left );
     }

     if ( switch_item_key ) {
          switch_item_key = false;
          Bool done = false;

          Int32 new_item_mode = static_cast< Int32 >( player.item_mode );

          while ( !done ) {
               new_item_mode++;
               new_item_mode %= Player::ItemMode::count;

               switch ( new_item_mode ) {
                    case Player::ItemMode::shield:
                         done = true;
                         break;
                    case Player::ItemMode::arrow:
                         if ( player.has_bow && player.arrow_count ) {
                              done = true;
                         }
                         break;
                    case Player::ItemMode::bomb:
                         if ( player.bomb_count ) {
                              done = true;
                         }
                         break;
               }
          }

          player.item_mode = static_cast<Player::ItemMode>( new_item_mode );
     }

     if ( attack_key ) {
          attack_key = false;

          if ( player.attack ( ) ) {
               sound.play_effect ( Sound::Effect::player_sword_attack );
          }

          Location adjacent_tile = character_adjacent_tile ( player );

          interactives.attack ( adjacent_tile );
     }

     player.update ( time_delta, map, interactives );

     tick_character_element ( player );

     if ( player.is_dead ( ) ) {
          player_death ( );
     }

     player.item_cooldown.tick ( time_delta );

     if ( item_key ) {
          switch ( player.item_mode ) {
          default:
               ASSERT ( 0 );
               break;
          case Player::ItemMode::shield:
               player.block ( );
               break;
          case Player::ItemMode::arrow:
               if ( player.use_bow ( ) ) {
                    spawn_projectile ( Projectile::Type::arrow, player.position, player.facing,
                                       Projectile::Alliance::good );
               }
               break;
          case Player::ItemMode::bomb:
               if ( player.use_bomb ( ) ) {
                    spawn_bomb ( player.position );
                    sound.play_effect ( Sound::Effect::place_bomb );
                    player.item_cooldown.reset ( Player::c_item_cooldown );
               }
               break;
          }
     }

     Location player_center_tile = Map::vector_to_location ( player.collision_center ( ) );

     if ( map.tile_location_is_valid ( player_center_tile ) ) {
          Auto& interactive = interactives.get_from_tile ( player_center_tile );
          Direction border_side = player_on_border ( );

          // check if player is trying to exit the map
          if ( interactive.type == Interactive::Type::exit &&
               interactive.interactive_exit.state == Exit::State::open &&
               interactive.interactive_exit.direction == opposite_direction ( player.facing ) ) {
               Int32 map_index = interactive.interactive_exit.map_index;
               Int32 region_index = interactive.interactive_exit.region_index;
               Location exit_index ( interactive.interactive_exit.exit_index_x,
                                     interactive.interactive_exit.exit_index_y );
               Vector new_position = Map::location_to_vector ( exit_index );

               new_position += Vector ( Map::c_tile_dimension_in_meters * 0.5f,
                                        Map::c_tile_dimension_in_meters * 0.5f );

               if ( region_index != region.current_index ) {
                    if ( !change_region ( game_memory, region_index ) ) {
                         quit_game ( );
                         return;
                    }

                    change_map ( map_index, false );
               } else {
                    change_map ( map_index );
               }

               player.set_collision_center ( new_position.x ( ), new_position.y ( ) );

               LOG_DEBUG ( "Setting player to %f, %f\n",
                           player.position.x ( ),
                           player.position.y ( ) );

               // no need to finish this update
               return;
          } else if ( border_side != Direction::count &&
                      border_side == player.facing ) {
               Auto& border_exit = map.get_border_exit ( border_side );
               Int32 map_index = border_exit.map_index;
               Location border_exit_bottom_left ( border_exit.bottom_left.x,
                                                  border_exit.bottom_left.y );
               Auto player_offset = player.collision_center ( ) -
                                    Map::location_to_vector ( border_exit_bottom_left );
               Auto new_player_pos = Map::location_to_vector ( border_exit_bottom_left ) + player_offset;

               change_map ( map_index );

               player.set_collision_center ( new_player_pos.x ( ), new_player_pos.y ( ) );

               LOG_DEBUG ( "Changing to map %d, setting player to %d, %d\n",
                           map_index,
                           player.position.x ( ),
                           player.position.y ( ) );

               return;
          }
     }

     if ( player.is_pushing ( ) ) {
          Location push_tile = character_adjacent_tile ( player );
          push_interactive ( push_tile, player.facing, map );
     } else {
          // check if player wants to activate any interactives
          if ( activate_key ) {
               activate_key = false;

               Location activate_tile = character_adjacent_tile ( player );

               if ( map.tile_location_is_valid ( activate_tile ) ) {
                    Auto& interactive = interactives.get_from_tile ( activate_tile );

                    if ( interactive.underneath.type != UnderneathInteractive::Type::popup_block ) {
                         switch ( interactive.type ) {
                         default:
                              LOG_DEBUG ( "Activate: %d, %d\n", activate_tile.x, activate_tile.y );

                              if ( interactives.activate ( activate_tile ) ) {
                                   sound.play_effect ( Sound::Effect::activate_interactive );
                              }
                              break;
                         case Interactive::Type::exit:
                              if ( interactive.interactive_exit.state == Exit::State::locked &&
                                   player.key_count > 0 ) {
                                   LOG_DEBUG ( "Unlock Door: %d, %d\n", activate_tile.x, activate_tile.y );
                                   interactives.activate ( activate_tile );
                                   player.key_count--;
                                   sound.play_effect ( Sound::Effect::activate_interactive );
                              }
                              break;
                         case Interactive::Type::torch:
                         case Interactive::Type::pushable_torch:
                              break;
                         }
                    }

                    Auto& secret_location = map.secret ( ).coordinates;

                    if ( activate_tile.x == secret_location.x &&
                         activate_tile.y == secret_location.y ) {
                         sound.play_effect ( Sound::Effect::activate_interactive );
                         map.find_secret ( );
                         LOG_DEBUG ( "Found secret!\n" );
                    }
               }
          }
     }

     Auto& upgrade = map.upgrade ( );

     if ( upgrade.id ) {
          Real32 upgrade_x = pixels_to_meters ( upgrade.coordinates.x * Map::c_tile_dimension_in_pixels );
          Real32 upgrade_y = pixels_to_meters ( upgrade.coordinates.y * Map::c_tile_dimension_in_pixels );

          if ( rect_collides_with_rect ( player.collision_x ( ), player.collision_y ( ),
                                         player.collision_width ( ), player.collision_width ( ),
                                         upgrade_x, upgrade_y,
                                         Map::c_tile_dimension_in_meters, Map::c_tile_dimension_in_meters ) ) {
               player.give_upgrade ( static_cast<Player::Upgrade>( upgrade.id ) );
               upgrade.id = 0;
               sound.play_effect ( Sound::Effect::player_pickup );
          }
     }

#ifdef DEBUG
     if ( invincible ) {
          player.health = player.max_health;
     }
#endif
}

Void State::update_enemies ( float time_delta )
{
     Vector player_center = player.collision_center ( );

     for ( Uint32 i = 0; i < enemies.max ( ); ++i ) {
          Auto& enemy = enemies [ i ];

          if ( enemy.is_dead ( ) ) {
               continue;
          }

          Vector enemy_center = enemy.collision_center ( );

#ifdef DEBUG
          if ( enemy_think ) {
               enemy.think ( enemies.entities, enemies.max ( ), player, random, time_delta );
          }
#else
          enemy.think ( enemies.entities, enemies.max ( ), player, random, time_delta );
#endif

          enemy.update ( time_delta, map, interactives );

          if ( enemy.type == Enemy::Type::fairy &&
               enemy.fairy_state.heal_timer.expired ( ) ) {
               heal_enemies_in_range_of_fairy ( enemy.position );
          }

          // check if the enemy has died after updating
          if ( enemy.is_dead ( ) ) {
               enemy_death ( enemy );
               continue;
          }

          // spawn a projectile if the goo is shooting
          if ( enemy.type == Enemy::Type::goo &&
               enemy.goo_state.state == Enemy::GooState::State::shooting ) {
               spawn_projectile ( Projectile::Type::goo, enemy.position, enemy.facing,
                                  Projectile::Alliance::evil );
          }

          tick_character_element ( enemy );

          // check collision between player and enemy
          if ( !player.is_blinking ( ) && player.is_alive ( ) &&
               !enemy.is_blinking ( ) && player.collides_with ( enemy ) ) {
               Direction damage_dir = direction_between ( enemy_center, player_center, random );

               // check if player blocked the attack
               if ( player.is_blocking ( ) &&
                    damage_dir == opposite_direction ( player.facing ) ) {
                    damage_character ( enemy, c_block_damage, opposite_direction ( damage_dir ) ) ;
                    spawn_pickup ( enemy.position, enemy.drop );
                    enemy.drop = Pickup::Type::none;
               } else {
                    damage_character ( player, c_enemy_damage, damage_dir );
                    sound.play_effect ( Sound::Effect::player_damaged );
                    enemy.hit_player = true;
               }

#ifdef DEBUG
               if ( invincible ) {
                    player.health = player.max_health;
               }
#endif

               player.effect_with_element ( enemy.effected_by_element );
          }

          // check if player's attack hits enemy
          if ( player.is_attacking ( ) && !enemy.is_blinking ( ) &&
               player.attack_collides_with ( enemy ) ) {

               if ( enemy.type == Enemy::Type::knight &&
                    player.facing == opposite_direction ( enemy.facing ) ) {
                    Direction damage_dir = direction_between ( player_center, enemy_center, random );
                    damage_character ( enemy, 0, damage_dir );
               } else {
                    Direction damage_dir = direction_between ( player_center, enemy_center, random );
                    damage_character ( enemy, c_attack_damage, damage_dir );
                    sound.play_effect ( Sound::Effect::player_damaged );
               }
          }
     }
}

Void State::push_interactive ( const Location& tile, Direction dir, const Map& map )
{
     if ( !map.tile_location_is_valid ( tile ) ) {
          return;
     }

     Bool enemy_on_tile = false;
     Auto dest = adjacent_tile ( tile, dir );

     for ( Uint8 i = 0; i < enemies.max ( ); ++i ) {
          if ( enemies [ i ].is_dead ( ) ) {
               continue;
          }

          if ( character_touching_tile ( enemies [ i ], dest ) ) {
               enemy_on_tile = true;
               break;
          }
     }

     if ( !enemy_on_tile ) {
          // make sure the player isn't on that tile either
          if ( character_touching_tile ( player, dest ) ) {

               // if the player is pushing the tile, stop it!
               Auto& interactive = interactives.get_from_tile ( tile );

               if ( interactive.type == Interactive::Type::pushable_block ) {
                    interactive.interactive_pushable_block.pushed_last_update = false;
               } else if ( interactive.type == Interactive::Type::pushable_torch ) {
                    interactive.interactive_pushable_torch.pushable_block.pushed_last_update = false;
               }

               return;
          }

          Bool pushed = interactives.push ( tile, dir, map );

          if ( pushed ) {
               sound.play_effect ( Sound::Effect::activate_interactive );
          }
     }
}

Void State::update_interactives ( float time_delta )
{
     Int32 count = map.width ( ) * map.height ( );

     for ( Int32 i = 0; i < count; ++i ) {
          Auto& interactive = interactives.m_interactives [ i ];

          if ( interactive.type ) {
               if ( interactive.underneath.type == UnderneathInteractive::Type::ice &&
                    interactive.underneath.underneath_ice.force_dir != Direction::count ) {
                    Auto tile = map.tile_index_to_location ( i );
                    push_interactive ( tile,
                                       interactive.underneath.underneath_ice.force_dir,
                                       map );
               }

               if ( interactive.underneath.type == UnderneathInteractive::Type::ice_detector &&
                    interactive.underneath.underneath_ice_detector.detected &&
                    interactive.underneath.underneath_ice_detector.force_dir != Direction::count ) {
                    Auto tile = map.tile_index_to_location ( i );
                    push_interactive ( tile,
                                       interactive.underneath.underneath_ice_detector.force_dir, map );
               }

               if ( interactive.underneath.type == UnderneathInteractive::Type::moving_walkway &&
                    interactive.underneath.underneath_moving_walkway.facing != Direction::count ) {
                    Auto tile = map.tile_index_to_location ( i );
                    push_interactive ( tile,
                                       interactive.underneath.underneath_moving_walkway.facing,
                                       map );
               }
          }

          switch ( interactive.type ) {
          default:
          case Interactive::Type::none:
          case Interactive::Type::bombable_block:
               break;
          case Interactive::Type::torch:
               if ( interactive.interactive_torch.element == Element::ice ) {
                    Auto tile = map.tile_index_to_location ( i );
                    interactives.spread_ice ( tile, map, false );
               }
               break;
          case Interactive::Type::pushable_torch:
               if ( interactive.interactive_pushable_torch.torch.element == Element::ice ) {
                    Auto tile = map.tile_index_to_location ( i );

                    interactives.spread_ice ( tile, map, false );
               }

               interactive.update ( time_delta, interactives );
               break;
          case Interactive::Type::lever:
               interactive.update ( time_delta, interactives );
               break;
          case Interactive::Type::pushable_block:
               interactive.update ( time_delta, interactives );
               break;
          case Interactive::Type::exit:
               interactive.update ( time_delta, interactives );
               break;
          case Interactive::Type::turret:
               if ( interactive.interactive_turret.wants_to_shoot ) {
                    Auto tile = map.tile_index_to_location ( i );
                    Vector interactive_pos = Map::location_to_vector ( tile );

                    spawn_projectile ( Projectile::Type::arrow,
                                       interactive_pos,
                                       interactive.interactive_turret.facing,
                                       Projectile::Alliance::neutral );

               }

               interactive.update ( time_delta, interactives );
               break;
          }
     }

     // NOTE: doing a second pass to check for fire so it will take precedence
     //       over any spread ice
     for ( Int32 i = 0; i < count; ++i ) {
          Auto& interactive = interactives.m_interactives [ i ];

          switch ( interactive.type ) {
          default:
               break;
          case Interactive::Type::torch:
               if ( interactive.interactive_torch.element == Element::fire ) {
                    Auto tile = map.tile_index_to_location ( i );
                    interactives.spread_ice ( tile, map, true );
               }
               break;
          case Interactive::Type::pushable_torch:
               if ( interactive.interactive_pushable_torch.torch.element == Element::fire ) {
                    Auto tile = map.tile_index_to_location ( i );
                    interactives.spread_ice ( tile, map, true );
               }
               break;
          }
     }
}

Void State::update_projectiles ( float time_delta )
{
     for ( Uint32 i = 0; i < projectiles.max ( ); ++i ) {
          Auto& projectile = projectiles [ i ];

          if ( projectile.is_dead ( ) ) {
               continue;
          }

          projectile.update ( time_delta, map, interactives );

          if ( !projectile.stuck_watch.expired ( ) ) {
               continue;
          }

          Vector projectile_collision_point = projectile.position + projectile.collision_points [ projectile.facing ];

          switch ( projectile.alliance ) {
          default:
               break;
          case Projectile::Alliance::good:
               for ( Uint32 c = 0; c < enemies.max ( ); ++c ) {
                    Auto& enemy = enemies [ c ];

                    if ( enemy.is_dead ( ) || enemy.is_blinking ( ) ) {
                         continue;
                    }

                    if ( point_inside_rect ( projectile_collision_point.x ( ),
                                             projectile_collision_point.y ( ),
                                             enemy.collision_x ( ), enemy.collision_y ( ),
                                             enemy.collision_x ( ) + enemy.collision_width ( ),
                                             enemy.collision_y ( ) + enemy.collision_height ( ) ) ) {
                         Int32 damage = projectile.hit_character ( enemy );
                         spawn_damage_number ( projectile_collision_point, damage );
                         break;
                    }
               }
               break;
         case Projectile::Alliance::evil:
               if ( !player.is_blinking ( ) &&
                    point_inside_rect ( projectile_collision_point.x ( ),
                                        projectile_collision_point.y ( ),
                                        player.collision_x ( ), player.collision_y ( ),
                                        player.collision_x ( ) + player.collision_width ( ),
                                        player.collision_y ( ) + player.collision_height ( ) ) ) {
                    if ( !check_player_block_projectile ( projectile ) ) {
                         Int32 damage = projectile.hit_character ( player );
                         spawn_damage_number ( projectile_collision_point, damage );
                         sound.play_effect ( Sound::Effect::player_damaged );
                    }
               }
               break;
         case Projectile::Alliance::neutral:
               for ( Uint32 c = 0; c < enemies.max ( ); ++c ) {
                    Auto& enemy = enemies [ c ];

                    if ( enemy.is_dead ( ) || enemy.is_blinking ( ) ) {
                         continue;
                    }

                    if ( point_inside_rect ( projectile_collision_point.x ( ),
                                             projectile_collision_point.y ( ),
                                             enemy.collision_x ( ), enemy.collision_y ( ),
                                             enemy.collision_x ( ) + enemy.collision_width ( ),
                                             enemy.collision_y ( ) + enemy.collision_height ( ) ) ) {
                         Int32 damage = projectile.hit_character ( enemy );
                         spawn_damage_number ( projectile_collision_point, damage );
                         break;
                    }
               }

               if ( !player.is_blinking ( ) &&
                    point_inside_rect ( projectile_collision_point.x ( ),
                                        projectile_collision_point.y ( ),
                                        player.collision_x ( ), player.collision_y ( ),
                                        player.collision_x ( ) + player.collision_width ( ),
                                        player.collision_y ( ) + player.collision_height ( ) ) ) {
                    if ( !check_player_block_projectile ( projectile ) ) {
                         Int32 damage = projectile.hit_character ( player );
                         spawn_damage_number ( projectile_collision_point, damage );
                         sound.play_effect ( Sound::Effect::player_damaged );
                    }
               }

               break;
         }
     }
}

Void State::update_bombs ( float time_delta )
{
     for ( Uint32 i = 0; i < bombs.max ( ); ++i ) {
          Auto& bomb = bombs [ i ];

          if ( bomb.is_dead ( ) ) {
               continue;
          }

          bomb.update ( time_delta );

          Vector bomb_center { bomb.position.x ( ) + Map::c_tile_dimension_in_meters * 0.5f,
                               bomb.position.y ( ) + Map::c_tile_dimension_in_meters * 0.5f };
          Location bomb_tile ( meters_to_pixels ( bomb_center.x ( ) ) / Map::c_tile_dimension_in_pixels,
                               meters_to_pixels ( bomb_center.y ( ) ) / Map::c_tile_dimension_in_pixels );

          Auto& current_interactive = interactives.get_from_tile ( bomb_tile );

          if ( current_interactive.underneath.type == UnderneathInteractive::Type::moving_walkway ) {
               static const Real32 c_bomb_moving_walkway_speed = 0.05f;
               Direction moving = current_interactive.underneath.underneath_moving_walkway.facing;
               bomb.position += vector_from_direction ( moving ) * c_bomb_moving_walkway_speed;
          }

          if ( bomb.life_state == Entity::LifeState::dying ) {
               // damage nearby enemies
               for ( Uint32 c = 0; c < enemies.max ( ); ++c ) {
                    Auto& enemy = enemies [ c ];

                    if ( enemy.is_dead ( ) ) {
                         continue;
                    }

                    Vector enemy_center = enemy.collision_center ( );

                    if ( enemy_center.distance_to ( bomb.position ) < Bomb::c_explode_radius ) {
                         damage_character ( enemy, c_bomb_damage, direction_between ( bomb.position,
                                                                                      enemy_center,
                                                                                      random ) );
                    }
               }

               // determine nearby objects
               Int32 tile_radius = meters_to_pixels ( Bomb::c_explode_radius ) / Map::c_tile_dimension_in_pixels;
               Int32 tile_min_x = bomb_tile.x - tile_radius;
               Int32 tile_min_y = bomb_tile.y - tile_radius;
               Int32 tile_max_x = tile_min_x + tile_radius * 2;
               Int32 tile_max_y = tile_min_y + tile_radius * 2;

               CLAMP ( tile_min_x, 0, interactives.width ( ) - 1 );
               CLAMP ( tile_min_y, 0, interactives.height ( ) - 1 );
               CLAMP ( tile_max_x, 0, interactives.width ( ) - 1 );
               CLAMP ( tile_max_y, 0, interactives.height ( ) - 1 );

               // loop over nearby objects
               for ( Int32 y = tile_min_y; y <= tile_max_y; ++y ) {
                    for ( Int32 x = tile_min_x; x <= tile_max_x; ++x ) {
                         Location current_tile ( x, y );
                         interactives.explode ( current_tile );
                    }
               }

               // create quick emitter
               Auto* emitter = emitters.spawn ( bomb.position );

               if ( emitter ) {
                    Vector offset { Map::c_tile_dimension_in_meters * 0.5f,
                                    Map::c_tile_dimension_in_meters * 0.5f };
                    // TODO: Make fire color
                    emitter->setup_limited_time ( bomb.position + offset, 0.5f,
                                                  SDL_MapRGB ( &back_buffer_format, 200, 200, 200 ),
                                                  0.0f, 6.28f, 0.5f, 0.5f, 6.0f, 6.0f,
                                                  Emitter::c_max_particles, 0 );
               }

               sound.play_effect ( Sound::Effect::bomb_exploded );
          }
     }
}

Void State::update_pickups ( float time_delta )
{
     for ( Uint32 i = 0; i < pickups.max ( ); ++i ) {
          Pickup& pickup = pickups [ i ];

          if ( pickup.is_dead ( ) ) {
               continue;
          }

          if ( rect_collides_with_rect ( player.collision_x ( ), player.collision_y ( ),
                                         player.collision_width ( ), player.collision_height ( ),
                                         pickup.position.x ( ), pickup.position.y ( ),
                                         Pickup::c_dimension_in_meters,
                                         Pickup::c_dimension_in_meters ) ||
               ( player.is_attacking ( ) &&
                 rect_collides_with_rect ( player.attack_x ( ), player.attack_y ( ),
                                           player.attack_width ( ), player.attack_height ( ),
                                           pickup.position.x ( ), pickup.position.y ( ),
                                           Pickup::c_dimension_in_meters,
                                           Pickup::c_dimension_in_meters ) ) ) {
               LOG_DEBUG ( "Player got pickup %s\n", Pickup::c_names [ pickup.type ] );

               switch ( pickup.type ) {
               default:
                    break;
               case Pickup::Type::health:
                    player.health += 2;

                    if ( player.health > player.max_health ) {
                         player.health = player.max_health;
                    }
                    break;
               case Pickup::Type::key:
                    player.key_count++;
                    break;
               case Pickup::Type::arrow:
                    player.give_arrow ( );
                    break;
               case Pickup::Type::bomb:
                    player.give_bomb ( );
                    break;
               }

               enqueue_pickup ( pickup.type );

               pickup.type = Pickup::Type::none;
               pickup.life_state = Entity::LifeState::dead;

               sound.play_effect ( Sound::Effect::player_pickup );
          }
     }

     if ( pickup_queue [ 0 ] ) {
          pickup_stopwatch.tick ( time_delta );

          if ( pickup_stopwatch.expired ( ) ) {
               dequeue_pickup ( );
          }
     }
}

Void State::update_emitters ( float time_delta )
{
     for ( Uint32 i = 0; i < emitters.max ( ); ++i ) {
          Auto& emitter = emitters [ i ];

          if ( emitter.is_dead ( ) ) {
               continue;
          }

          emitter.update ( time_delta, random );
     }
}

Void State::update_damage_numbers ( float time_delta )
{
     for ( Uint32 i = 0; i < damage_numbers.max ( ); ++i ) {
          Auto& damage_number = damage_numbers [ i ];

          if ( damage_number.is_dead ( ) ) {
               continue;
          }

          damage_number.update ( time_delta );
     }
}


Void State::update_light ( )
{
     map.reset_light ( );

     interactives.contribute_light ( map );

     // projectiles on fire contribute light
     for ( Uint32 i = 0; i < projectiles.max ( ); ++i ) {
          Auto& projectile = projectiles [ i ];

          if ( projectile.is_dead ( ) ) {
               continue;
          }

          if ( projectile.effected_by_element == Element::fire ) {
               Vector collision_position ( projectile.position.x ( ) +
                                           Projectile::collision_points [ projectile.facing ].x ( ),
                                           projectile.position.y ( ) +
                                           Projectile::collision_points [ projectile.facing ].y ( ) );
               Location collision_loc ( meters_to_pixels ( collision_position.x ( ) ),
                                        meters_to_pixels ( collision_position.y ( ) ) );
               map.illuminate ( collision_loc, LightDetector::c_bryte_value - 1 );
          }
     }

     // give interactives the light values on their respective tiles
     for ( Location tile; tile.y < interactives.height ( ); ++tile.y ) {
          for ( tile.x = 0; tile.x < interactives.width ( ); ++tile.x ) {
               interactives.light ( tile, map.get_tile_location_light ( tile ) );
          }
     }
}

Void State::enqueue_pickup ( Pickup::Type type )
{
     for( Int32 i = 0; i < c_pickup_queue_size; ++i ) {
          if ( pickup_queue [ i ] == Pickup::Type::none ) {
               pickup_queue [ i ] = type;
               break;
          }
     }
}

Void State::dequeue_pickup ( )
{
     Int32 last_index = c_pickup_queue_size - 1;

     for( Int32 i = 0; i < last_index; ++i ) {
          pickup_queue [ i ] = pickup_queue [ i + 1 ];
     }

     pickup_stopwatch.reset ( c_pickup_show_time );
}

Void State::heal_enemies_in_range_of_fairy ( const Vector& position )
{
     for ( Uint32 i = 0; i < enemies.max ( ); ++i ) {
          Auto& enemy = enemies [ i ];

          if ( enemy.is_dead ( ) || enemy.type == Enemy::Type::fairy ) {
               continue;
          }

          if ( enemy.position.distance_to ( position ) < Enemy::FairyState::c_heal_radius ) {
               enemy.heal ( 1 );
               spawn_healing_number ( enemy.collision_center ( ), 1 );
          }
     }
}

Void State::change_map ( Int32 map_index, Bool persist )
{
     LOG_DEBUG ( "Changing map to %d\n", map_index );

     if ( persist ) {
          persist_map ( );
     }

     if ( !map.load_from_master_list ( map_index, interactives ) ) {
          quit_game ( );
          return;
     }

     pickups.clear ( );
     projectiles.clear ( );
     enemies.clear ( );
     emitters.clear ( );

     spawn_map_enemies ( );

     setup_emitters_from_map_lamps ( );

     if ( strlen ( map.dialogue ( ) ) ) {
          dialogue.begin ( );
     }
}

Direction State::player_on_border ( )
{
     Auto player_tile = Map::vector_to_location ( player.collision_center ( ) );

     Auto border = map.get_border_exit ( Direction::left );
     if ( border.bottom_left.x || border.bottom_left.y ) {
          if ( player_tile.x == border.bottom_left.x ) {
               LOG_DEBUG ( "Player on left border\n" );
               return Direction::left;
          }
     }

     border = map.get_border_exit ( Direction::right );
     if ( border.bottom_left.x || border.bottom_left.y ) {
          if ( player_tile.x == border.bottom_left.x ) {
               LOG_DEBUG ( "Player on right border\n" );
               return Direction::right;
          }
     }

     border = map.get_border_exit ( Direction::up );
     if ( border.bottom_left.x || border.bottom_left.y ) {
          if ( player_tile.y == border.bottom_left.y ) {
               LOG_DEBUG ( "Player on up border\n" );
               return Direction::up;
          }
     }

     border = map.get_border_exit ( Direction::down );
     if ( border.bottom_left.x || border.bottom_left.y ) {
          if ( player_tile.y == border.bottom_left.y ) {
               LOG_DEBUG ( "Player on down border\n" );
               return Direction::down;
          }
     }

     return Direction::count;
}

Bool State::change_region ( GameMemory& game_memory, Int32 region_index )
{
     LOG_INFO ( "Changing to region %d\n", region_index );

     // persist the current map then save it
     persist_map ( );
     map.save_persistence ( region.name, player.save_slot );

     // load new region info
     if ( !region.load_info ( region_index ) ) {
          return false;
     }

     // load the new master list and it's corresponding persistence if it exists
     if ( !map.load_master_list ( region.map_list_filepath ) ) {
          return false;
     }

     map.load_persistence ( region.name, player.save_slot );

     // unload and re-load surfaces
     map_display.unload_surfaces ( );
     if ( !map_display.load_surfaces ( game_memory,
                                       region.tilesheet_filepath,
                                       region.decorsheet_filepath,
                                       region.lampsheet_filepath ) ) {
          return false;
     }

     interactives_display.unload_surfaces ( );
     if ( !interactives_display.load_surfaces  ( game_memory, region.exitsheet_filepath ) ) {
          return false;
     }

     return true;
}

Void State::player_save ( )
{
     persist_map ( );
     player.save ( );
     map.save_persistence ( region.name, player.save_slot );
}

Void State::player_load ( )
{
     player.load ( );
     map.load_persistence ( region.name, player.save_slot );
}

Void State::render_upgrade ( SDL_Surface* back_buffer )
{
     Auto& upgrade = map.upgrade ( );

     if ( !upgrade.id ) {
          return;
     }

     SDL_Rect src { ( upgrade.id - 1 ) * Map::c_tile_dimension_in_pixels, 0,
                    Map::c_tile_dimension_in_pixels, 14 };
     SDL_Rect dst { upgrade.coordinates.x * Map::c_tile_dimension_in_pixels,
                    upgrade.coordinates.y * Map::c_tile_dimension_in_pixels,
                    Map::c_tile_dimension_in_pixels, 14 };

     world_to_sdl ( dst, back_buffer, camera.x ( ), camera.y ( ) );

     SDL_BlitSurface ( upgrade_sheet, &src, back_buffer, &dst );
}

extern "C" Bool game_init ( GameMemory& game_memory, Void* settings )
{
     MemoryLocations* memory_locations = GAME_PUSH_MEMORY ( game_memory, MemoryLocations );
     State* state = GAME_PUSH_MEMORY ( game_memory, State );

     memory_locations->state = state;

     if ( !state->initialize ( game_memory, reinterpret_cast<Settings*>( settings ) ) ) {
          return false;
     }

     return true;
}

extern "C" Void game_destroy ( GameMemory& game_memory )
{
     Auto* state = get_state ( game_memory );

     state->destroy ( );
}

extern "C" Void game_user_input ( GameMemory& game_memory, const GameInput& game_input )
{
     Auto* state = get_state ( game_memory );

     state->handle_input ( game_memory, game_input );
}

extern "C" Void game_update ( GameMemory& game_memory, Real32 time_delta )
{
     Auto* state = get_state ( game_memory );

     state->update ( game_memory, time_delta );
}

static Void render_bomb ( SDL_Surface* back_buffer, SDL_Surface* bomb_sheet, const Bomb& bomb,
                           Real32 camera_x, Real32 camera_y )
{

     SDL_Rect dest_rect = build_world_sdl_rect ( bomb.position.x ( ), bomb.position.y ( ),
                                                 Map::c_tile_dimension_in_meters,
                                                 Map::c_tile_dimension_in_meters );

     SDL_Rect clip_rect { 0, 0, Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

     world_to_sdl ( dest_rect, back_buffer, camera_x, camera_y );

     SDL_BlitSurface ( bomb_sheet, &clip_rect, back_buffer, &dest_rect );
}

static Void render_particle ( SDL_Surface* back_buffer, const Particle& particle, Uint32 color,
                              Real32 camera_x, Real32 camera_y )
{
     SDL_Rect dest_rect = build_world_sdl_rect ( particle.position.x ( ), particle.position.y ( ), 0.0f, 0.0f );

     world_to_sdl ( dest_rect, back_buffer, camera_x, camera_y );

     dest_rect.w = 1;
     dest_rect.h = 1;

     SDL_FillRect ( back_buffer, &dest_rect, color );
}

static void render_damage_number ( Text& text, SDL_Surface* back_buffer, const DamageNumber& damage_number,
                                   Real32 camera_x, Real32 camera_y )
{
     char buffer [ 64 ];
     Int32 value = abs ( damage_number.value );
     const char* positive_format = "+%d";
     const char* negative_format = "%d";
     const char* format = damage_number.value > 0 ? positive_format : negative_format;

     sprintf ( buffer, format, value );

     SDL_Rect dest_rect = build_world_sdl_rect ( damage_number.position.x ( ), damage_number.position.y ( ),
                                                 pixels_to_meters ( text.character_width ),
                                                 pixels_to_meters ( text.character_height ) );

     dest_rect.x -= ( text.character_width / 2 );
     dest_rect.y -= ( text.character_height / 2 );

     world_to_sdl ( dest_rect, back_buffer, camera_x, camera_y );

     text.render ( back_buffer, buffer, dest_rect.x, dest_rect.y);
}

static Void render_shown_pickup ( SDL_Surface* back_buffer, SDL_Surface* pickup_sheet,
                                  Character& player, Pickup::Type pickup_type,
                                  Real32 camera_x, Real32 camera_y )
{
     SDL_Rect dest_rect = build_world_sdl_rect ( player.position.x ( ) + Pickup::c_dimension_in_meters * 0.4f,
                                                 player.position.y ( ) + player.height ( ) + pixels_to_meters ( 1 ),
                                                 Pickup::c_dimension_in_meters,
                                                 Pickup::c_dimension_in_meters );

     SDL_Rect clip_rect { 0, ( static_cast<Int32>( pickup_type ) - 1 ) * Pickup::c_dimension_in_pixels,
                          Pickup::c_dimension_in_pixels, Pickup::c_dimension_in_pixels };

     world_to_sdl ( dest_rect, back_buffer, camera_x, camera_y );

     SDL_BlitSurface ( pickup_sheet, &clip_rect, back_buffer, &dest_rect );
}

static Void render_icon ( SDL_Surface* back_buffer, SDL_Surface* icon_sheet, Int32 frame, Int32 x, Int32 y )
{
     Uint32 white  = SDL_MapRGB ( back_buffer->format, 255, 255, 255 );

     SDL_Rect attack_dest { x, y, Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };
     SDL_Rect attack_clip { frame * Map::c_tile_dimension_in_pixels, 0,
                            Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

     SDL_BlitSurface ( icon_sheet, &attack_clip, back_buffer, &attack_dest );

     SDL_Rect attack_outline { attack_dest.x - 1, attack_dest.y - 1,
                               Map::c_tile_dimension_in_pixels + 2,
                               Map::c_tile_dimension_in_pixels + 2 };

     render_rect_outline ( back_buffer, attack_outline, white );
}

static Void render_hearts ( SDL_Surface* back_buffer, SDL_Surface* heart_sheet, Int32 health, Int32 max_health,
                            Int32 x, Int32 y )
{
     Int32 heart_count = max_health / 2;
     Int32 heart_state = 0;

     SDL_Rect clip_rect { 0, 0, Pickup::c_dimension_in_pixels, Pickup::c_dimension_in_pixels };
     SDL_Rect dest_rect { x, y, Pickup::c_dimension_in_pixels, Pickup::c_dimension_in_pixels };

     for ( Int32 i = 0; i < heart_count; ++i ) {
          heart_state += 2;

          if ( heart_state == health + 1 ) {
               clip_rect.x = Pickup::c_dimension_in_pixels;
          } else if ( heart_state > health ) {
               clip_rect.x = 2 * Pickup::c_dimension_in_pixels;
          }

          SDL_BlitSurface ( heart_sheet, &clip_rect, back_buffer, &dest_rect );

          dest_rect.x += Pickup::c_dimension_in_pixels + 2;
          clip_rect.x = 0;
     }
}

extern "C" Void game_render ( GameMemory& game_memory, SDL_Surface* back_buffer )
{
     Auto* state = get_state ( game_memory );

     state->render ( game_memory, back_buffer );
}
