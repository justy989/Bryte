#include "Editor.hpp"
#include "Log.hpp"
#include "Utils.hpp"
#include "Bitmap.hpp"
#include "MapDisplay.hpp"

using namespace editor;
using namespace bryte;

const Real32 State::c_camera_speed = 20.0f;

static State* get_state ( GameMemory& game_memory )
{
     return reinterpret_cast<MemoryLocations*>( game_memory.location ( ) )->state;
}

void State::mouse_button_changed_down ( bool left )
{
     switch ( mode ) {
     default:
          ASSERT ( 0 );
          break;
     case Mode::tile:
          // on right click, set solids
          if ( !left ) {
               if ( mouse_tile_x >= 0 && mouse_tile_x < map.width ( ) &&
                    mouse_tile_y >= 0 && mouse_tile_y < map.height ( ) ) {
                    current_solid = !map.get_coordinate_solid ( mouse_tile_x, mouse_tile_y );
                    map.set_coordinate_solid ( mouse_tile_x, mouse_tile_y, current_solid );
               }
          }
          break;
     case Mode::decor:
     {
          Map::Fixture* decor = map.check_coordinates_for_decor ( mouse_tile_x, mouse_tile_y );

          if ( left ) {
               if ( decor ) {
                    map.remove_decor ( decor );
               } else {
                    if ( mouse_tile_x >= 0 && mouse_tile_x < map.width ( ) &&
                         mouse_tile_y >= 0 && mouse_tile_y < map.height ( ) ) {
                         map.add_decor ( mouse_tile_x, mouse_tile_y, current_decor );
                    }
               }

               map.reset_light ( );
          }
     } break;
     case Mode::light:
     {
          Map::Fixture* lamp = map.check_coordinates_for_lamp ( mouse_tile_x, mouse_tile_y );

          if ( left ) {
               if ( lamp ) {
                    map.remove_lamp ( lamp );
               } else {
                    if ( mouse_tile_x >= 0 && mouse_tile_x < map.width ( ) &&
                         mouse_tile_y >= 0 && mouse_tile_y < map.height ( ) ) {
                         map.add_lamp ( mouse_tile_x, mouse_tile_y, current_lamp );
                    }
               }

               map.reset_light ( );
          } else {
               current_lamp++;
               current_lamp %= Map::c_unique_lamp_count;
          }
     } break;
     case Mode::exit:
     {
          if ( mouse_tile_x < 0 || mouse_tile_x >= map.width ( ) ||
               mouse_tile_y < 0 || mouse_tile_y >= map.height ( ) ) {
               break;
          }

          Interactive& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );

          if ( left ) {
               if ( interactive.type == Interactive::Type::exit ) {
                    interactive.type = Interactive::Type::none;
               } else {
                    interactive.type = Interactive::Type::exit;
                    interactive.reset ( );
                    interactive.interactive_exit.direction    = static_cast<Direction>( current_exit_direction );
                    interactive.interactive_exit.state        = static_cast<Exit::State>( current_exit_state );
               }
          } else {
               if ( interactive.type == Interactive::Type::exit ) {
                    interactive.interactive_exit.state = static_cast<Exit::State>(
                         ( static_cast<Int32>(interactive.interactive_exit.state) + 1 ) %
                           Exit::State::count );
               } else {
                    current_exit_state++;
                    current_exit_state %= 3;
               }
          }
     } break;
     case Mode::lever:
     {
          if ( mouse_tile_x < 0 || mouse_tile_x >= map.width ( ) ||
               mouse_tile_y < 0 || mouse_tile_y >= map.height ( ) ) {
               break;
          }

          Interactive& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );

          if ( left ) {
               if ( interactive.type == Interactive::Type::lever ) {
                    interactive.type = Interactive::Type::none;
               } else {
                    interactive.type = Interactive::Type::lever;
                    interactive.reset ( );
               }
          } else {
               if ( interactive.type == Interactive::Type::lever ) {
                    current_interactive_x = mouse_tile_x;
                    current_interactive_y = mouse_tile_y;
                    track_current_interactive = true;
               } else {
                    auto& lever = interactives.get_from_tile ( current_interactive_x,
                                                               current_interactive_y );
                    ASSERT ( lever.type == Interactive::Type::lever );
                    lever.interactive_lever.activate_coordinate_x = mouse_tile_x;
                    lever.interactive_lever.activate_coordinate_y = mouse_tile_y;
                    track_current_interactive = false;
               }
          }
     } break;
     case Mode::pushable_block:
     {
          if ( mouse_tile_x < 0 || mouse_tile_x >= map.width ( ) ||
               mouse_tile_y < 0 || mouse_tile_y >= map.height ( ) ) {
               break;
          }

          Interactive& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );

          if ( left ) {
               if ( interactive.type == Interactive::Type::pushable_block ) {
                    interactive.type = Interactive::Type::none;
               } else {
                    interactive.type = Interactive::Type::pushable_block;
                    interactive.reset ( );
               }
          }
     } break;
     case Mode::enemy:
     {
          Map::Fixture* enemy_spawn = map.check_coordinates_for_enemy_spawn ( mouse_tile_x, mouse_tile_y );

          if ( left ) {
               if ( enemy_spawn ) {
                    map.remove_enemy_spawn ( enemy_spawn );
               } else {
                    if ( mouse_tile_x >= 0 && mouse_tile_x < map.width ( ) &&
                         mouse_tile_y >= 0 && mouse_tile_y < map.height ( ) ) {
                         map.add_enemy_spawn ( mouse_tile_x, mouse_tile_y, 1 );
                    }
               }
          } else {
          }
     } break;
     case Mode::torch:
     {
          if ( mouse_tile_x < 0 || mouse_tile_x >= map.width ( ) ||
               mouse_tile_y < 0 || mouse_tile_y >= map.height ( ) ) {
               break;
          }

          Interactive& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );

          if ( left ) {
               if ( interactive.type == Interactive::Type::torch ) {
                    interactive.type = Interactive::Type::none;
               } else {
                    interactive.type = Interactive::Type::torch;
                    interactive.reset ( );
                    interactive.interactive_torch.on = current_torch;
               }
          } else {
               current_torch++;
               current_torch %= 2;
          }
     } break;
     case Mode::pushable_torch:
     {
          if ( mouse_tile_x < 0 || mouse_tile_x >= map.width ( ) ||
               mouse_tile_y < 0 || mouse_tile_y >= map.height ( ) ) {
               break;
          }

          Interactive& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );

          if ( left ) {
               if ( interactive.type == Interactive::Type::pushable_torch ) {
                    interactive.type = Interactive::Type::none;
               } else {
                    interactive.type = Interactive::Type::pushable_torch;
                    interactive.reset ( );
                    interactive.interactive_torch.on = current_pushable_torch;
               }
          } else {
               current_pushable_torch++;
               current_pushable_torch %= 2;
          }
     } break;
     }
}

void State::option_button_changed_down ( bool up )
{
     switch ( mode ) {
     default:
          ASSERT ( 0 );
          break;
     case Mode::tile:
          if ( up ) {
               if ( current_tile > 0 ) {
                    current_tile--;
               }
          } else {
               if ( current_tile < tilesheet->w / Map::c_tile_dimension_in_pixels ) {
                    current_tile++;
               }
          }
          break;
     case Mode::decor:
          if ( up ) {
               if ( current_decor > 0 ) {
                    current_decor--;
               }
          } else {
               if ( current_decor < decorsheet->w / Map::c_tile_dimension_in_pixels ) {
                    current_decor++;
               }
          }
          break;
     case Mode::light:
          if ( up ) {
               map.subtract_from_base_light ( 4 );
          } else {
               map.add_to_base_light ( 4 );
          }

          map.reset_light ( );
          break;
     case Mode::exit:
     {
          if ( mouse_tile_x < 0 || mouse_tile_x >= map.width ( ) ||
               mouse_tile_y < 0 || mouse_tile_y >= map.height ( ) ) {
               break;
          }

          auto& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );

          if ( interactive.type == Interactive::Type::exit ) {
               if ( up ) {
                    switch ( current_field ) {
                    case 0:
                         interactive.interactive_exit.map_index--;
                         break;
                    case 1:
                         interactive.interactive_exit.exit_index_x--;
                         break;
                    case 2:
                         interactive.interactive_exit.exit_index_y--;
                         break;
                    }
               } else {
                    switch ( current_field ) {
                    case 0:
                         interactive.interactive_exit.map_index++;
                         break;
                    case 1:
                         interactive.interactive_exit.exit_index_x++;
                         break;
                    case 2:
                         interactive.interactive_exit.exit_index_y++;
                         break;
                    }
               }
          } else {
               if ( up ) {
                    current_exit_direction++;
               } else {
                    current_exit_direction++;
               }

               current_exit_direction %= 4;
          }
     } break;
     case Mode::lever:
     {
          if ( mouse_tile_x < 0 || mouse_tile_x >= map.width ( ) ||
               mouse_tile_y < 0 || mouse_tile_y >= map.height ( ) ) {
               break;
          }

          auto& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );

          if ( interactive.type == Interactive::Type::lever ) {
               if ( up ) {
                    switch ( current_field ) {
                    case 0:
                         interactive.interactive_lever.activate_coordinate_x--;
                         break;
                    case 1:
                         interactive.interactive_lever.activate_coordinate_y--;
                         break;
                    }
               } else {
                    switch ( current_field ) {
                    case 0:
                         interactive.interactive_lever.activate_coordinate_x++;
                         break;
                    case 1:
                         interactive.interactive_lever.activate_coordinate_y++;
                         break;
                    }
               }
          }
     } break;
     }
}

Void State::option_scroll ( Int32 scroll )
{

}

extern "C" Bool game_init ( GameMemory& game_memory, void* settings )
{
     MemoryLocations* memory_locations = GAME_PUSH_MEMORY ( game_memory, MemoryLocations );
     State* state = GAME_PUSH_MEMORY ( game_memory, State);

     state->settings = reinterpret_cast<Settings*>( settings );

     memory_locations->state = state;

     FileContents text_contents    = load_entire_file ( "text.bmp", &game_memory );

     state->text.fontsheet         = load_bitmap ( &text_contents );
     state->text.character_width   = 5;
     state->text.character_height  = 8;
     state->text.character_spacing = 1;

     if ( !state->text.fontsheet ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( state->mode_icons_surface, game_memory,
                                          "editor_mode_icons.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( state->tilesheet,  game_memory,
                                          state->settings->map_tilesheet_filename ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( state->decorsheet, game_memory,
                                          state->settings->map_decorsheet_filename ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( state->lampsheet, game_memory,
                                          state->settings->map_lampsheet_filename ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( state->rat_surface, game_memory,
                                          state->settings->map_rat_filename ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( state->interactives_display.interactive_sheets [ Interactive::Type::exit ],
                                          game_memory,
                                          "castle_exitsheet.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( state->interactives_display.interactive_sheets [ Interactive::Type::lever ],
                                          game_memory,
                                          "castle_leversheet.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( state->interactives_display.interactive_sheets [ Interactive::Type::pushable_block ],
                                          game_memory,
                                          "castle_pushableblocksheet.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( state->interactives_display.interactive_sheets [ Interactive::Type::torch ],
                                          game_memory,
                                          "castle_torchsheet.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( state->interactives_display.interactive_sheets [ Interactive::Type::pushable_torch ],
                                          game_memory,
                                          "castle_pushabletorchsheet.bmp" ) ) {
          return false;
     }

     if ( state->settings->map_load_filename ) {
          if ( !state->settings->map_save_filename ) {
               state->settings->map_save_filename = state->settings->map_load_filename;
          }

          state->map.load ( state->settings->map_load_filename, state->interactives );
     } else {
          state->map.initialize ( state->settings->map_width, state->settings->map_height );

          state->interactives.reset ( state->map.width ( ), state->map.height ( ) );
     }

     state->current_tile           = 1;
     state->current_decor          = 0;
     state->current_exit_direction = 0;
     state->current_exit_state     = 0;
     state->current_lamp           = 0;
     state->current_solid          = false;

     state->camera.set ( 0.0f, 0.0f );

     state->left_button_down = false;
     state->right_button_down = false;

     state->mode = Mode::tile;

     state->draw_solids = false;
     state->draw_light  = false;

     return true;
}

extern "C" Void game_destroy ( GameMemory& game_memory )
{
     State* state = get_state ( game_memory );

     SDL_FreeSurface ( state->mode_icons_surface );

     SDL_FreeSurface ( state->tilesheet );
     SDL_FreeSurface ( state->decorsheet );
     SDL_FreeSurface ( state->lampsheet );

     SDL_FreeSurface ( state->rat_surface );
}

extern "C" Void game_user_input ( GameMemory& game_memory, const GameInput& game_input )
{
     State* state = get_state ( game_memory );

     for ( Uint32 i = 0; i < game_input.mouse_button_change_count; ++i ) {
          auto change = game_input.mouse_button_changes [ i ];

          switch ( change.button ) {
          default:
               break;
          case SDL_BUTTON_LEFT:
               state->left_button_down = change.down;

               if ( state->left_button_down ) {
                    state->mouse_button_changed_down ( true );
               }
               break;
          case SDL_BUTTON_RIGHT:
               state->right_button_down = change.down;

               if ( state->right_button_down ) {
                    state->mouse_button_changed_down ( false );
               }
               break;
          }
     }

     state->mouse_x = game_input.mouse_position_x;
     state->mouse_y = game_input.mouse_position_y;
     if ( game_input.mouse_scroll ) {
          state->option_scroll ( game_input.mouse_scroll );
     }

     for ( Uint32 i = 0; i < game_input.key_change_count; ++i ) {
          const GameInput::KeyChange& key_change = game_input.key_changes [ i ];

          switch ( key_change.scan_code ) {
          default:
               break;
          case SDL_SCANCODE_Q:
               if ( key_change.down ) {
                    state->option_button_changed_down ( true );
               }
               break;
          case SDL_SCANCODE_E:
               if ( key_change.down ) {
                    state->option_button_changed_down ( false );
               }
               break;
          case SDL_SCANCODE_M:
               if ( key_change.down ) {
                    state->mode = static_cast<Mode>( static_cast<int>( state->mode ) + 1 );
                    state->mode = static_cast<Mode>( static_cast<int>( state->mode ) % Mode::count );
                    state->current_field = 0;
                    state->track_current_interactive = false;
               }
               break;
          case SDL_SCANCODE_W:
               state->camera_direction_keys [ 0 ] = key_change.down;
               break;
          case SDL_SCANCODE_S:
               state->camera_direction_keys [ 1 ] = key_change.down;
               break;
          case SDL_SCANCODE_D:
               state->camera_direction_keys [ 2 ] = key_change.down;
               break;
          case SDL_SCANCODE_A:
               state->camera_direction_keys [ 3 ] = key_change.down;
               break;
          case SDL_SCANCODE_F:
               if ( key_change.down ) {
                    switch ( state->mode ) {
                    default:
                         break;
                    case Mode::exit:
                         state->current_field++;
                         state->current_field %= 3;
                         break;
                    case Mode::lever:
                         state->current_field++;
                         state->current_field %= 2;
                         break;
                    }
               }
               break;
          case SDL_SCANCODE_O:
               if ( key_change.down ) {
                    state->map.save ( state->settings->map_save_filename, state->interactives );
               }
               break;
          case SDL_SCANCODE_I:
               if ( key_change.down ) {
                    state->map.load ( state->settings->map_save_filename, state->interactives );
               }
               break;
          case SDL_SCANCODE_C:
               if ( key_change.down ) {
                    state->draw_solids = !state->draw_solids;
               }
               break;
          case SDL_SCANCODE_L:
               if ( key_change.down ) {
                    state->draw_light = !state->draw_light;
               }
               break;
          }
     }
}

extern "C" Void game_update ( GameMemory& game_memory, Real32 time_delta )
{
     State* state = get_state ( game_memory );
     Vector camera_velocity;

     // reset the message buffer
     state->message_buffer [ 0 ] = '\0';

     if ( state->camera_direction_keys [ 0 ] ) {
          camera_velocity.set_y ( -State::c_camera_speed );
     }

     if ( state->camera_direction_keys [ 1 ] ) {
          camera_velocity.set_y ( State::c_camera_speed );
     }

     if ( state->camera_direction_keys [ 2 ] ) {
          camera_velocity.set_x ( -State::c_camera_speed );
     }

     if ( state->camera_direction_keys [ 3 ] ) {
          camera_velocity.set_x ( State::c_camera_speed );
     }

     state->camera += camera_velocity * time_delta;

     state->mouse_screen_x = state->mouse_x - meters_to_pixels ( state->camera.x ( ) );
     state->mouse_screen_y = state->mouse_y - meters_to_pixels ( state->camera.y ( ) );
     state->mouse_tile_x   = state->mouse_screen_x / Map::c_tile_dimension_in_pixels;
     state->mouse_tile_y   = state->mouse_screen_y / Map::c_tile_dimension_in_pixels;

     switch ( state->mode ) {
     default:
          ASSERT ( 0 );
          break;
     case Mode::tile:
          if ( state->mouse_tile_x >= 0 && state->mouse_tile_x < state->map.width ( ) &&
               state->mouse_tile_y >= 0 && state->mouse_tile_y < state->map.height ( ) ) {
               if ( state->left_button_down ) {
                    state->map.set_coordinate_value ( state->mouse_tile_x, state->mouse_tile_y,
                                                      state->current_tile );
               } else if ( state->right_button_down ) {
                    state->map.set_coordinate_solid ( state->mouse_tile_x, state->mouse_tile_y,
                                                      state->current_solid );
               }
          }
          break;
     case Mode::decor:
          break;
     case Mode::light:
          sprintf ( state->message_buffer, "BASE %d", state->map.base_light_value ( ) );
          break;
     case Mode::exit:
          if ( state->mouse_tile_x >= 0 && state->mouse_tile_x < state->map.width ( ) &&
               state->mouse_tile_y >= 0 && state->mouse_tile_y < state->map.height ( ) ) {
               auto& interactive = state->interactives.get_from_tile ( state->mouse_tile_x, state->mouse_tile_y );

               if ( interactive.type == Interactive::Type::exit ) {
                    auto& exit = interactive.interactive_exit;
                    sprintf ( state->message_buffer, "MAP %d EXIT %d %d", exit.map_index,
                              exit.exit_index_x, exit.exit_index_y );
               }
          }
     break;
     case Mode::lever:
          if ( state->track_current_interactive ) {
               sprintf ( state->message_buffer, "C ACT %d %d",
                         state->current_interactive_x, state->current_interactive_y );
               break;
          }

          if ( state->mouse_tile_x >= 0 && state->mouse_tile_x < state->map.width ( ) &&
               state->mouse_tile_y >= 0 && state->mouse_tile_y < state->map.height ( ) ) {
               auto& interactive = state->interactives.get_from_tile ( state->mouse_tile_x, state->mouse_tile_y );

               if ( interactive.type == Interactive::Type::lever ) {
                    auto& lever = interactive.interactive_lever;
                    sprintf ( state->message_buffer, "ACT %d %d",
                              lever.activate_coordinate_x, lever.activate_coordinate_y );
               }
          }
     break;
     }
}

static Void render_map_solids ( SDL_Surface* back_buffer, Map& map, Real32 camera_x, Real32 camera_y )
{
     Uint32 red_color = SDL_MapRGB ( back_buffer->format, 255, 0, 0 );

     for ( Int32 y = 0; y < static_cast<Int32>( map.height ( ) ); ++y ) {
          for ( Int32 x = 0; x < static_cast<Int32>( map.width ( ) ); ++x ) {

               bool is_solid = map.get_coordinate_solid ( x, y );

               if ( !is_solid ) {
                    continue;
               }

               SDL_Rect solid_rect_b { 0, 0, Map::c_tile_dimension_in_pixels, 1 };
               SDL_Rect solid_rect_l { 0, 0, 1, Map::c_tile_dimension_in_pixels };
               SDL_Rect solid_rect_t { 0, 0, Map::c_tile_dimension_in_pixels, 1 };
               SDL_Rect solid_rect_r { 0, 0, 1, Map::c_tile_dimension_in_pixels };

               solid_rect_b.x = x * Map::c_tile_dimension_in_pixels;
               solid_rect_b.y = y * Map::c_tile_dimension_in_pixels;
               solid_rect_l.x = x * Map::c_tile_dimension_in_pixels;
               solid_rect_l.y = y * Map::c_tile_dimension_in_pixels;
               solid_rect_t.x = x * Map::c_tile_dimension_in_pixels;
               solid_rect_t.y = y * Map::c_tile_dimension_in_pixels +
                                Map::c_tile_dimension_in_pixels - 1;
               solid_rect_r.x = x * Map::c_tile_dimension_in_pixels +
                                Map::c_tile_dimension_in_pixels - 1;
               solid_rect_r.y = y * Map::c_tile_dimension_in_pixels;

               world_to_sdl ( solid_rect_b, back_buffer, camera_x, camera_y );
               world_to_sdl ( solid_rect_l, back_buffer, camera_x, camera_y );
               world_to_sdl ( solid_rect_t, back_buffer, camera_x, camera_y );
               world_to_sdl ( solid_rect_r, back_buffer, camera_x, camera_y );

               SDL_FillRect ( back_buffer, &solid_rect_b, red_color );
               SDL_FillRect ( back_buffer, &solid_rect_l, red_color );
               SDL_FillRect ( back_buffer, &solid_rect_t, red_color );
               SDL_FillRect ( back_buffer, &solid_rect_r, red_color );
          }
     }
}

static Void render_enemy_spawns ( SDL_Surface* back_buffer, SDL_Surface* enemy_sheet, Map& map,
                                  Real32 camera_x, Real32 camera_y )
{
     for ( Uint8 i = 0; i < map.enemy_spawn_count ( ); ++i ) {
          auto& enemy_spawn = map.enemy_spawn ( i );

          SDL_Rect enemy_spawn_rect { enemy_spawn.location.x * Map::c_tile_dimension_in_pixels,
                                      enemy_spawn.location.y * Map::c_tile_dimension_in_pixels,
                                      Map::c_tile_dimension_in_pixels,
                                      Map::c_tile_dimension_in_pixels };
          SDL_Rect clip_rect { 0, 0,
                               Map::c_tile_dimension_in_pixels,
                               Map::c_tile_dimension_in_pixels };

          enemy_spawn_rect.x = enemy_spawn.location.x * Map::c_tile_dimension_in_pixels;
          enemy_spawn_rect.y = enemy_spawn.location.y * Map::c_tile_dimension_in_pixels;

          world_to_sdl ( enemy_spawn_rect, back_buffer, camera_x, camera_y );

          SDL_BlitSurface ( enemy_sheet, &clip_rect, back_buffer, &enemy_spawn_rect );
     }
}

static Void render_current_icon ( SDL_Surface* back_buffer, SDL_Surface* sheet,
                                  Int32 mouse_x, Int32 mouse_y, Int32 current_x_id, Int32 current_y_id )
{
     SDL_Rect dest_rect { mouse_x, back_buffer->h - mouse_y,
                          Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };
     SDL_Rect clip_rect { current_x_id * Map::c_tile_dimension_in_pixels,
                          current_y_id * Map::c_tile_dimension_in_pixels,
                          Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

     SDL_BlitSurface ( sheet, &clip_rect, back_buffer, &dest_rect );
}

static Void render_mode_icons ( SDL_Surface* back_buffer, SDL_Surface* icon_surface, Mode mode )
{
     static const Int32 c_icon_dimension = 11;
     static const Int32 c_icon_start_x   = c_icon_dimension + 2;

     for ( int i = 0; i < Mode::count; ++i ) {
          SDL_Rect dest_rect { i * c_icon_start_x + 1, 3, c_icon_dimension, c_icon_dimension };
          SDL_Rect clip_rect { i * c_icon_dimension, 0, c_icon_dimension, c_icon_dimension };

          SDL_BlitSurface ( icon_surface, &clip_rect, back_buffer, &dest_rect );
     }

     static const Int32 c_border_dimension = c_icon_dimension + 1;

     Int32 x = ( static_cast<Int32>( mode ) * c_icon_start_x );
     Int32 y = 2;
     Uint32 red_color = SDL_MapRGB ( back_buffer->format, 255, 0, 0 );

     SDL_Rect mode_rect_b { 0, 0, c_border_dimension, 1 };
     SDL_Rect mode_rect_l { 0, 0, 1, c_border_dimension };
     SDL_Rect mode_rect_t { 0, 0, c_border_dimension + 1, 1 };
     SDL_Rect mode_rect_r { 0, 0, 1, c_border_dimension };

     mode_rect_b.x = x;
     mode_rect_b.y = y;
     mode_rect_l.x = x;
     mode_rect_l.y = y;
     mode_rect_t.x = x;
     mode_rect_t.y = y + c_border_dimension;
     mode_rect_r.x = x + c_border_dimension;
     mode_rect_r.y = y;

     SDL_FillRect ( back_buffer, &mode_rect_b, red_color );
     SDL_FillRect ( back_buffer, &mode_rect_l, red_color );
     SDL_FillRect ( back_buffer, &mode_rect_t, red_color );
     SDL_FillRect ( back_buffer, &mode_rect_r, red_color );
}

extern "C" Void game_render ( GameMemory& game_memory, SDL_Surface* back_buffer )
{
     State* state = get_state ( game_memory );

     render_map ( back_buffer, state->tilesheet, state->map, state->camera.x ( ), state->camera.y ( ) );
     render_map_decor ( back_buffer, state->decorsheet, state->map, state->camera.x ( ), state->camera.y ( ) );
     render_map_lamps ( back_buffer, state->lampsheet, state->map, state->camera.x ( ), state->camera.y ( ) );
     render_enemy_spawns ( back_buffer, state->rat_surface, state->map, state->camera.x ( ), state->camera.y ( ) );

     state->interactives_display.render_interactives ( back_buffer, state->interactives,
                                                       state->camera.x ( ), state->camera.y ( ) );

     state->map.reset_light ( );

     state->interactives_display.contribute_light ( state->interactives, state->map );

     if ( state->draw_light ) {
          render_light ( back_buffer, state->map, state->camera.x ( ), state->camera.y ( ) );
     }

     if ( state->draw_solids ) {
          render_map_solids ( back_buffer, state->map, state->camera.x ( ), state->camera.y ( ) );
     }

     SDL_Rect hud_rect { 0, 0, back_buffer->w, 34 };
     SDL_FillRect ( back_buffer, &hud_rect, SDL_MapRGB ( back_buffer->format, 0, 0, 0 ) );

     render_mode_icons ( back_buffer, state->mode_icons_surface, state->mode );

     switch ( state->mode ) {
     default:
          ASSERT ( 0 );
          break;
     case Mode::tile:
          render_current_icon ( back_buffer, state->tilesheet, state->mouse_x, state->mouse_y,
                                state->current_tile, 0 );
          break;
     case Mode::decor:
          render_current_icon ( back_buffer, state->decorsheet, state->mouse_x, state->mouse_y,
                                state->current_decor, 0 );
          break;
     case Mode::light:
          render_current_icon ( back_buffer, state->lampsheet, state->mouse_x, state->mouse_y,
                                state->current_lamp, 0 );
          break;
     case Mode::enemy:
          render_current_icon ( back_buffer, state->rat_surface, state->mouse_x, state->mouse_y, 0, 0 );
          break;
     case Mode::exit:
          render_current_icon ( back_buffer,
                                state->interactives_display.interactive_sheets [ Interactive::Type::exit ],
                                state->mouse_x, state->mouse_y,
                                state->current_exit_direction, state->current_exit_state );
          break;
     case Mode::lever:
          render_current_icon ( back_buffer,
                                state->interactives_display.interactive_sheets [ Interactive::Type::lever ],
                                state->mouse_x, state->mouse_y,
                                0, 0 );
          break;
     case Mode::pushable_block:
          render_current_icon ( back_buffer,
                                state->interactives_display.interactive_sheets [ Interactive::Type::pushable_block ],
                                state->mouse_x, state->mouse_y,
                                0, 0 );
          break;
     case Mode::torch:
          render_current_icon ( back_buffer,
                                state->interactives_display.interactive_sheets [ Interactive::Type::torch ],
                                state->mouse_x, state->mouse_y,
                                state->current_torch, 0 );
          break;
     case Mode::pushable_torch:
          render_current_icon ( back_buffer,
                                state->interactives_display.interactive_sheets [ Interactive::Type::pushable_torch ],
                                state->mouse_x, state->mouse_y,
                                state->current_pushable_torch, 0 );
          break;
     }

     char buffer [ 64 ];
     sprintf ( buffer, "T %d %d", state->mouse_tile_x, state->mouse_tile_y );
     state->text.render ( back_buffer, buffer, 210, 4 );

     sprintf ( buffer, "FIELD %d", state->current_field );
     state->text.render ( back_buffer, buffer, 210, 20 );

     state->text.render ( back_buffer, state->message_buffer,
                          1 * ( state->text.character_width + state->text.character_spacing ), 20 );
}

