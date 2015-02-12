#ifdef LINUX

#include "Editor.hpp"
#include "Log.hpp"
#include "Utils.hpp"
#include "Bitmap.hpp"
#include "MapDisplay.hpp"

#include <cstdio>

using namespace editor;
using namespace bryte;

const Real32 State::c_camera_speed = 20.0f;

static State* get_state ( GameMemory& game_memory )
{
     return reinterpret_cast<MemoryLocations*>( game_memory.location ( ) )->state;
}

Bool State::mouse_on_map ( )
{
     return ( mouse_tile_x >= 0 && mouse_tile_x < map.width ( ) &&
              mouse_tile_y >= 0 && mouse_tile_y < map.height ( ) );
}

Void State::mouse_button_left_clicked ( )
{
     if ( !mouse_on_map ( ) ) {
          return;
     }

     switch ( mode ) {
     default:
          break;
     case Mode::tile:
          break;
     case Mode::decor:
     {
          Map::Fixture* decor = map.check_coordinates_for_decor ( mouse_tile_x, mouse_tile_y );

          if ( decor ) {
               map.remove_decor ( decor );
          } else {
               if ( mouse_tile_x >= 0 && mouse_tile_x < map.width ( ) &&
                    mouse_tile_y >= 0 && mouse_tile_y < map.height ( ) ) {
                    map.add_decor ( mouse_tile_x, mouse_tile_y, current_decor );
               }
          }

          map.reset_light ( );
     } break;
     case Mode::light:
     {
          Map::Fixture* lamp = map.check_coordinates_for_lamp ( mouse_tile_x, mouse_tile_y );

          if ( lamp ) {
               map.remove_lamp ( lamp );
          } else {
               if ( mouse_tile_x >= 0 && mouse_tile_x < map.width ( ) &&
                    mouse_tile_y >= 0 && mouse_tile_y < map.height ( ) ) {
                    map.add_lamp ( mouse_tile_x, mouse_tile_y, current_lamp );
               }
          }

          map.reset_light ( );
     } break;
     case Mode::exit:
     {
          Interactive& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );

          if ( interactive.type == Interactive::Type::exit ) {
               interactive.type = Interactive::Type::none;
          } else {
               interactive.type = Interactive::Type::exit;
               interactive.reset ( );
               interactive.interactive_exit.direction    = static_cast<Direction>( current_exit_direction );
               interactive.interactive_exit.state        = static_cast<Exit::State>( current_exit_state * 2 );
          }
     } break;
     case Mode::lever:
     {
          Interactive& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );

          if ( interactive.type == Interactive::Type::lever ) {
               interactive.type = Interactive::Type::none;
          } else {
               interactive.type = Interactive::Type::lever;
               interactive.reset ( );
          }
     } break;
     case Mode::pushable_block:
     {
          Interactive& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );

          if ( interactive.type == Interactive::Type::pushable_block ) {
               interactive.type = Interactive::Type::none;
          } else {
               interactive.type = Interactive::Type::pushable_block;
               interactive.reset ( );
               interactive.interactive_pushable_block.one_time = true;
          }
     } break;
     case Mode::enemy:
     {
          if ( !mouse_on_map ( ) ) {
               break;
          }

          Map::EnemySpawn* enemy_spawn = map.check_coordinates_for_enemy_spawn ( mouse_tile_x, mouse_tile_y );

          if ( enemy_spawn ) {
               map.remove_enemy_spawn ( enemy_spawn );
          } else {
               map.add_enemy_spawn ( mouse_tile_x, mouse_tile_y, current_enemy,
                                     static_cast<Direction>( current_enemy_direction ),
                                     static_cast<Pickup::Type>( current_enemy_drop ) );
          }
     } break;
     case Mode::torch:
     {
          Interactive& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );

          if ( interactive.type == Interactive::Type::torch ) {
               interactive.type = Interactive::Type::none;
               interactive.reset ( );
          } else {
               interactive.type = Interactive::Type::torch;
               interactive.reset ( );
               interactive.interactive_torch.on = current_torch;
          }
     } break;
     case Mode::pushable_torch:
     {
          Interactive& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );

          if ( interactive.type == Interactive::Type::pushable_torch ) {
               interactive.reset ( );
               interactive.type = Interactive::Type::none;
          } else {
               interactive.type = Interactive::Type::pushable_torch;
               interactive.reset ( );
               interactive.interactive_pushable_torch.torch.on = current_pushable_torch;
          }
     } break;
     case Mode::light_detector:
     {
          Interactive& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );

          if ( interactive.type == Interactive::Type::light_detector ) {
               interactive.type = Interactive::Type::none;
               interactive.reset ( );
          } else {
               interactive.type = Interactive::Type::light_detector;
               interactive.reset ( );
               interactive.interactive_light_detector.type = static_cast<bryte::LightDetector::Type>(
                                                                  current_light_detector_bryte );
          }
     } break;
     case Mode::pressure_plate:
     {
          Interactive& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );
          UnderneathInteractive& underneath = interactive.underneath;

          if ( underneath.type == UnderneathInteractive::Type::pressure_plate ) {
               underneath.type = UnderneathInteractive::Type::none;
               interactive.reset ( );
          } else {
               underneath.type = UnderneathInteractive::Type::pressure_plate;
               interactive.reset ( );
               underneath.underneath_pressure_plate.activate_coordinate_x = 0;
               underneath.underneath_pressure_plate.activate_coordinate_y = 0;
          }
     } break;
     case Mode::popup_block:
     {
          Interactive& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );
          UnderneathInteractive& underneath = interactive.underneath;

          if ( underneath.type == UnderneathInteractive::Type::popup_block ) {
               underneath.type = UnderneathInteractive::Type::none;
               interactive.reset ( );
          } else {
               underneath.type = UnderneathInteractive::Type::popup_block;
               interactive.reset ( );
               underneath.underneath_popup_block.up = static_cast<Bool>( current_popup_block );
          }
     } break;
     }
}

Void State::mouse_button_right_clicked ( )
{
     if ( !mouse_on_map ( ) ) {
          return;
     }

     switch ( mode ) {
     default:
          break;
     case Mode::tile:
          current_solid = !map.get_coordinate_solid ( mouse_tile_x, mouse_tile_y );
          map.set_coordinate_solid ( mouse_tile_x, mouse_tile_y, current_solid );
          break;
     case Mode::decor:
          break;
     case Mode::light:
          current_lamp++;
          current_lamp %= Map::c_unique_lamp_count;
          break;
     case Mode::exit:
     {
          Interactive& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );

          if ( interactive.type == Interactive::Type::exit ) {
               interactive.interactive_exit.state = static_cast<Exit::State>(
                    ( static_cast<Int32>(interactive.interactive_exit.state) + 2 ) %
                      ( Exit::State::locked + 2 ) );
          } else {
               current_exit_state++;
               current_exit_state %= 3;
          }
     } break;
     case Mode::lever:
     {
          Interactive& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );

          if ( interactive.type == Interactive::Type::lever ) {
               current_interactive_x = mouse_tile_x;
               current_interactive_y = mouse_tile_y;
               track_current_interactive = true;
          } else {
               if ( track_current_interactive ) {
                    Auto& lever = interactives.get_from_tile ( current_interactive_x,
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
          Interactive& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );

          if ( interactive.type == Interactive::Type::pushable_block ) {
               current_interactive_x = mouse_tile_x;
               current_interactive_y = mouse_tile_y;
               track_current_interactive = true;
          } else {
               if ( track_current_interactive ) {
                    Auto& pushable_block = interactives.get_from_tile ( current_interactive_x,
                                                               current_interactive_y );
                    ASSERT ( pushable_block.type == Interactive::Type::pushable_block );
                    pushable_block.interactive_pushable_block.activate_coordinate_x = mouse_tile_x;
                    pushable_block.interactive_pushable_block.activate_coordinate_y = mouse_tile_y;
                    track_current_interactive = false;
               }
          }
     } break;
     case Mode::enemy:
          current_enemy_direction++;
          current_enemy_direction %= Direction::count;
          break;
     case Mode::torch:
          current_torch++;
          current_torch %= 2;
          break;
     case Mode::pushable_torch:
          current_pushable_torch++;
          current_pushable_torch %= 2;
          break;
     case Mode::light_detector:
     {
          Interactive& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );

          if ( interactive.type == Interactive::Type::light_detector ) {
               current_interactive_x = mouse_tile_x;
               current_interactive_y = mouse_tile_y;
               track_current_interactive = true;
          } else {
               if ( track_current_interactive ) {
                    Auto& light_detector = interactives.get_from_tile ( current_interactive_x,
                                                                        current_interactive_y );
                    ASSERT ( light_detector.type == Interactive::Type::light_detector );
                    light_detector.interactive_light_detector.activate_coordinate_x = mouse_tile_x;
                    light_detector.interactive_light_detector.activate_coordinate_y = mouse_tile_y;
                    track_current_interactive = false;
               }
          }
     } break;
     case Mode::pressure_plate:
     {
          Interactive& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );
          UnderneathInteractive underneath = interactive.underneath;

          if ( underneath.type == UnderneathInteractive::Type::pressure_plate ) {
               current_interactive_x = mouse_tile_x;
               current_interactive_y = mouse_tile_y;
               track_current_interactive = true;
          } else {
               if ( track_current_interactive ) {
                    Auto& interactive = interactives.get_from_tile ( current_interactive_x,
                                                                        current_interactive_y );
                    ASSERT ( interactive.underneath.type == UnderneathInteractive::Type::pressure_plate );
                    Auto& pressure_plate = interactive.underneath.underneath_pressure_plate;
                    pressure_plate.activate_coordinate_x = mouse_tile_x;
                    pressure_plate.activate_coordinate_y = mouse_tile_y;
                    track_current_interactive = false;
               }
          }
     } break;
     case Mode::popup_block:
     {
          Interactive& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );
          UnderneathInteractive underneath = interactive.underneath;

          if ( underneath.type == UnderneathInteractive::Type::popup_block ) {
               underneath.underneath_popup_block.up = !underneath.underneath_popup_block.up;
          } else {
               current_popup_block = !current_popup_block;
          }
     } break;
     }
}

Void State::option_button_up_pressed ( )
{
     switch ( mode ) {
     default:
          break;
     case Mode::tile:
          if ( current_tile > 0 ) {
               current_tile--;
          }
          break;
     case Mode::decor:
          if ( current_decor > 0 ) {
               current_decor--;
          }
          break;
     case Mode::light:
          map.subtract_from_base_light ( 4 );
          map.reset_light ( );
          break;
     case Mode::enemy:
          current_enemy++;
          current_enemy %= Enemy::Type::count;
          break;
     case Mode::exit:
     {
          if ( !mouse_on_map ( ) ) {
               break;
          }

          Auto& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );

          if ( interactive.type == Interactive::Type::exit ) {
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
               current_exit_direction++;
               current_exit_direction %= 4;
          }
     } break;
     case Mode::lever:
          break;
     case Mode::pushable_block:
          break;
     case Mode::light_detector:
          current_light_detector_bryte++;
          current_light_detector_bryte %= 2;
          break;
     }
}

Void State::option_button_down_pressed ( )
{
     switch ( mode ) {
     default:
          break;
     case Mode::tile:
          if ( current_tile < map_display.tilesheet->w / Map::c_tile_dimension_in_pixels ) {
               current_tile++;
          }
          break;
     case Mode::decor:
          if ( current_decor < map_display.decorsheet->w / Map::c_tile_dimension_in_pixels ) {
               current_decor++;
          }
          break;
     case Mode::light:
          map.add_to_base_light ( 4 );
          map.reset_light ( );
          break;
     case Mode::enemy:
          current_enemy++;
          current_enemy %= Enemy::Type::count;
          break;
     case Mode::exit:
     {
          if ( !mouse_on_map ( ) ) {
               break;
          }

          Auto& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );

          if ( interactive.type == Interactive::Type::exit ) {
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
          } else {
               current_exit_direction++;
               current_exit_direction %= 4;
          }
     } break;
     case Mode::lever:
          break;
     case Mode::pushable_block:
          break;
     case Mode::light_detector:
          current_light_detector_bryte--;
          current_light_detector_bryte %= 2;
          break;
     }
}

Void State::mouse_scrolled ( Int32 scroll )
{
     switch ( mode )
     {
     default:
          break;
     case Mode::enemy:
          if ( mouse_on_map ( ) ) {
               Auto* enemy_spawn = map.check_coordinates_for_enemy_spawn ( mouse_tile_x, mouse_tile_y );

               if ( enemy_spawn ) {
                    Int32 value = static_cast<Int32>( enemy_spawn->drop );
                    value += scroll;
                    enemy_spawn->drop = static_cast<Pickup::Type>( value % Pickup::Type::count );
               } else {
                    current_enemy_drop += scroll;
               }
          } else {
               current_enemy_drop += scroll;
          }

          current_enemy_drop %= Pickup::Type::count;
          break;
     case Mode::pushable_block:
          break;
     }
}

extern "C" Bool game_init ( GameMemory& game_memory, Void* settings )
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

     if ( !load_bitmap_with_game_memory ( state->map_display.tilesheet,  game_memory,
                                          state->settings->map_tilesheet_filename ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( state->map_display.decorsheet, game_memory,
                                          state->settings->map_decorsheet_filename ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( state->map_display.lampsheet, game_memory,
                                          state->settings->map_lampsheet_filename ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( state->character_display.enemy_sheets [ Enemy::Type::rat ],
                                          game_memory,
                                          "test_rat.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( state->character_display.enemy_sheets [ Enemy::Type::bat ],
                                          game_memory,
                                          "test_bat.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( state->interactives_display.interactive_sheet,
                                          game_memory,
                                          "castle_interactivesheet.bmp" ) ) {
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

     state->camera.zero ( );

     state->mode = Mode::tile;

     state->mouse_x = 0;
     state->mouse_y = 0;
     state->mouse_screen_x = 0;
     state->mouse_screen_y = 0;
     state->mouse_tile_x = 0;
     state->mouse_tile_y = 0;

     for ( Int32 i = 0; i < 4; ++i ) {
          state->camera_direction_keys [ i ] = false;
     }

     state->left_button_down = false;
     state->right_button_down = false;

     state->current_tile                 = 0;
     state->current_solid                = false;
     state->current_decor                = 0;
     state->current_lamp                 = 0;
     state->current_enemy                = 0;
     state->current_enemy_direction      = 0;
     state->current_enemy_drop           = 0;
     state->current_exit_direction       = 0;
     state->current_exit_state           = 0;
     state->current_torch                = 0;
     state->current_pushable_torch       = 0;
     state->current_light_detector_bryte = 0;
     state->current_field                = 0;

     state->track_current_interactive    = false;
     state->current_interactive_x        = 0;
     state->current_interactive_y        = 0;

     state->draw_solids                  = false;
     state->draw_light                   = false;

     state->message_buffer [ 0 ] = '\0';

     return true;
}

extern "C" Void game_destroy ( GameMemory& game_memory )
{
     State* state = get_state ( game_memory );

     SDL_FreeSurface ( state->mode_icons_surface );

     SDL_FreeSurface ( state->map_display.tilesheet );
     SDL_FreeSurface ( state->map_display.decorsheet );
     SDL_FreeSurface ( state->map_display.lampsheet );

     for ( int i = 0; i < Enemy::Type::count; ++i ) {
          SDL_FreeSurface ( state->character_display.enemy_sheets [ i ] );
     }

     SDL_FreeSurface ( state->interactives_display.interactive_sheet );
}

extern "C" Void game_user_input ( GameMemory& game_memory, const GameInput& game_input )
{
     State* state = get_state ( game_memory );

     for ( Uint32 i = 0; i < game_input.mouse_button_change_count; ++i ) {
          Auto change = game_input.mouse_button_changes [ i ];

          switch ( change.button ) {
          default:
               break;
          case SDL_BUTTON_LEFT:
               state->left_button_down = change.down;

               if ( state->left_button_down ) {
                    state->mouse_button_left_clicked ( );
               }
               break;
          case SDL_BUTTON_RIGHT:
               state->right_button_down = change.down;

               if ( state->right_button_down ) {
                    state->mouse_button_right_clicked ( );
               }
               break;
          }
     }

     state->mouse_x = game_input.mouse_position_x;
     state->mouse_y = game_input.mouse_position_y;

     if ( game_input.mouse_scroll ) {
          state->mouse_scrolled ( game_input.mouse_scroll );
     }

     for ( Uint32 i = 0; i < game_input.key_change_count; ++i ) {
          const GameInput::KeyChange& key_change = game_input.key_changes [ i ];

          switch ( key_change.scan_code ) {
          default:
               break;
          case SDL_SCANCODE_Q:
               if ( key_change.down ) {
                    state->option_button_up_pressed ( );
               }
               break;
          case SDL_SCANCODE_E:
               if ( key_change.down ) {
                    state->option_button_down_pressed ( );
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
          break;
     case Mode::tile:
          if ( !state->mouse_on_map ( ) ) {
               break;
          }

          if ( state->left_button_down ) {
               state->map.set_coordinate_value ( state->mouse_tile_x, state->mouse_tile_y,
                                                 state->current_tile );
          } else if ( state->right_button_down ) {
               state->map.set_coordinate_solid ( state->mouse_tile_x, state->mouse_tile_y,
                                                 state->current_solid );
          }
          break;
     case Mode::decor:
          break;
     case Mode::light:
          sprintf ( state->message_buffer, "BASE %d", state->map.base_light_value ( ) );
          break;
     case Mode::enemy:
     {
          if ( !state->mouse_on_map ( ) ) {
               break;
          }

          Auto* enemy_spawn = state->map.check_coordinates_for_enemy_spawn ( state->mouse_tile_x, state->mouse_tile_y );

          if ( enemy_spawn ) {
               sprintf ( state->message_buffer, "PICKUP %s", Pickup::c_names [ enemy_spawn->drop ] );
          }
     } break;
     case Mode::exit:
     {
          if ( !state->mouse_on_map ( ) ) {
               break;
          }

          Auto& interactive = state->interactives.get_from_tile ( state->mouse_tile_x, state->mouse_tile_y );

          if ( interactive.type == Interactive::Type::exit ) {
               Auto& exit = interactive.interactive_exit;
               sprintf ( state->message_buffer, "MAP %d EXIT %d %d", exit.map_index,
                         exit.exit_index_x, exit.exit_index_y );
          }
     } break;
     case Mode::lever:
     {
          if ( state->track_current_interactive ) {
               sprintf ( state->message_buffer, "C ACT %d %d",
                         state->current_interactive_x, state->current_interactive_y );
               break;
          }

          if ( !state->mouse_on_map ( ) ) {
               break;
          }

          Auto& interactive = state->interactives.get_from_tile ( state->mouse_tile_x, state->mouse_tile_y );

          if ( interactive.type == Interactive::Type::lever ) {
               Auto& lever = interactive.interactive_lever;
               sprintf ( state->message_buffer, "ACT %d %d",
                         lever.activate_coordinate_x, lever.activate_coordinate_y );
          }
     } break;
     case Mode::pushable_block:
     {
          if ( state->track_current_interactive ) {
               sprintf ( state->message_buffer, "C ACT %d %d",
                         state->current_interactive_x, state->current_interactive_y );
               break;
          }

          if ( !state->mouse_on_map ( ) ) {
               break;
          }

          Auto& interactive = state->interactives.get_from_tile ( state->mouse_tile_x, state->mouse_tile_y );

          if ( interactive.type == Interactive::Type::pushable_block ) {
               Auto& pushable_block = interactive.interactive_pushable_block;
               sprintf ( state->message_buffer, "ACT %d %d",
                         pushable_block.activate_coordinate_y,
                         pushable_block.activate_coordinate_x );
          }
     } break;
     case Mode::light_detector:
     {
          if ( state->track_current_interactive ) {
               sprintf ( state->message_buffer, "C ACT %d %d",
                         state->current_interactive_x, state->current_interactive_y );
               break;
          }

          if ( !state->mouse_on_map ( ) ) {
               break;
          }

          Auto& interactive = state->interactives.get_from_tile ( state->mouse_tile_x, state->mouse_tile_y );

          if ( interactive.type == Interactive::Type::light_detector ) {
               Auto& light_detector = interactive.interactive_light_detector;
               sprintf ( state->message_buffer, "ACT %d %d",
                         light_detector.activate_coordinate_x, light_detector.activate_coordinate_y );
          }
     } break;
     case Mode::pressure_plate:
     {
          if ( state->track_current_interactive ) {
               sprintf ( state->message_buffer, "C ACT %d %d",
                         state->current_interactive_x, state->current_interactive_y );
               break;
          }

          if ( !state->mouse_on_map ( ) ) {
               break;
          }

          Auto& interactive = state->interactives.get_from_tile ( state->mouse_tile_x, state->mouse_tile_y );

          if ( interactive.underneath.type == UnderneathInteractive::Type::pressure_plate ) {
               Auto& pressure_plate = interactive.underneath.underneath_pressure_plate;
               sprintf ( state->message_buffer, "ACT %d %d",
                         pressure_plate.activate_coordinate_x, pressure_plate.activate_coordinate_y );
          }
     } break;


     }

     state->map.reset_light ( );

     state->interactives.contribute_light ( state->map );

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

static Void render_enemy_spawns ( SDL_Surface* back_buffer, SDL_Surface** enemy_sheet,
                                  Map& map, Real32 camera_x, Real32 camera_y )
{
     for ( Uint8 i = 0; i < map.enemy_spawn_count ( ); ++i ) {
          Auto& enemy_spawn = map.enemy_spawn ( i );

          SDL_Rect enemy_spawn_rect { enemy_spawn.location.x * Map::c_tile_dimension_in_pixels,
                                      enemy_spawn.location.y * Map::c_tile_dimension_in_pixels,
                                      Map::c_tile_dimension_in_pixels,
                                      Map::c_tile_dimension_in_pixels };
          SDL_Rect clip_rect { 0, static_cast<Int32>( enemy_spawn.facing ) * Map::c_tile_dimension_in_pixels,
                               Map::c_tile_dimension_in_pixels,
                               Map::c_tile_dimension_in_pixels };

          enemy_spawn_rect.x = enemy_spawn.location.x * Map::c_tile_dimension_in_pixels;
          enemy_spawn_rect.y = enemy_spawn.location.y * Map::c_tile_dimension_in_pixels;

          world_to_sdl ( enemy_spawn_rect, back_buffer, camera_x, camera_y );

          SDL_BlitSurface ( enemy_sheet [ enemy_spawn.id ], &clip_rect, back_buffer, &enemy_spawn_rect );
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

     // map
     state->map_display.render ( back_buffer, state->map, state->camera.x ( ), state->camera.y ( ) );

     // interactives
     state->interactives_display.render ( back_buffer, state->interactives,
                                          state->camera.x ( ), state->camera.y ( ) );

     // enemy spawns
     render_enemy_spawns ( back_buffer, state->character_display.enemy_sheets,
                           state->map, state->camera.x ( ), state->camera.y ( ) );

     // light
     if ( state->draw_light ) {
          render_light ( back_buffer, state->map, state->camera.x ( ), state->camera.y ( ) );
     }

     // solids
     if ( state->draw_solids ) {
          render_map_solids ( back_buffer, state->map, state->camera.x ( ), state->camera.y ( ) );
     }

     // ui
     SDL_Rect hud_rect { 0, 0, back_buffer->w, 34 };
     SDL_FillRect ( back_buffer, &hud_rect, SDL_MapRGB ( back_buffer->format, 0, 0, 0 ) );

     render_mode_icons ( back_buffer, state->mode_icons_surface, state->mode );

     switch ( state->mode ) {
     default:
          break;
     case Mode::tile:
     {
          Auto tile = state->current_tile;

          if ( tile ) {
               render_current_icon ( back_buffer, state->map_display.tilesheet, state->mouse_x, state->mouse_y,
                                     tile - 1, 0 );
          }
          break;
     }
     case Mode::decor:
          render_current_icon ( back_buffer, state->map_display.decorsheet, state->mouse_x, state->mouse_y,
                                state->current_decor, 0 );
          break;
     case Mode::light:
          render_current_icon ( back_buffer, state->map_display.lampsheet, state->mouse_x, state->mouse_y,
                                state->current_lamp, 0 );
          break;
     case Mode::enemy:
          render_current_icon ( back_buffer, state->character_display.enemy_sheets [ state->current_enemy ],
                                state->mouse_x, state->mouse_y, 0, state->current_enemy_direction );
          break;
     case Mode::lever:
          render_current_icon ( back_buffer,
                                state->interactives_display.interactive_sheet,
                                state->mouse_x, state->mouse_y,
                                0, Interactive::Type::lever - 1 );
          break;
     case Mode::pushable_block:
          render_current_icon ( back_buffer,
                                state->interactives_display.interactive_sheet,
                                state->mouse_x, state->mouse_y,
                                0, Interactive::Type::pushable_block - 1 );
          break;
     case Mode::torch:
          render_current_icon ( back_buffer,
                                state->interactives_display.interactive_sheet,
                                state->mouse_x, state->mouse_y,
                                0, Interactive::Type::torch - 1 );
          break;
     case Mode::pushable_torch:
          render_current_icon ( back_buffer,
                                state->interactives_display.interactive_sheet,
                                state->mouse_x, state->mouse_y,
                                0, Interactive::Type::pushable_torch - 1 );
          break;
     case Mode::light_detector:
          render_current_icon ( back_buffer,
                                state->interactives_display.interactive_sheet,
                                state->mouse_x, state->mouse_y,
                                2 * state->current_light_detector_bryte,
                                Interactive::Type::light_detector - 1 );
          break;
     case Mode::exit:
          render_current_icon ( back_buffer,
                                state->interactives_display.interactive_sheet,
                                state->mouse_x, state->mouse_y,
                                state->current_exit_direction,
                                Interactive::Type::exit + ( state->current_exit_state * 2 ) );
          break;
     case Mode::pressure_plate:
          render_current_icon ( back_buffer,
                                state->interactives_display.interactive_sheet,
                                state->mouse_x, state->mouse_y,
                                0,
                                Interactive::Type::exit + 5 );
          break;
     case Mode::popup_block:
          render_current_icon ( back_buffer,
                                state->interactives_display.interactive_sheet,
                                state->mouse_x, state->mouse_y,
                                0,
                                Interactive::Type::pushable_block - 1 );
          break;
     }

     // text ui
     Char8 buffer [ 64 ];
     sprintf ( buffer, "T %d %d", state->mouse_tile_x, state->mouse_tile_y );
     state->text.render ( back_buffer, buffer, 210, 4 );

     sprintf ( buffer, "FIELD %d", state->current_field );
     state->text.render ( back_buffer, buffer, 210, 20 );

     state->text.render ( back_buffer, state->message_buffer,
                          1 * ( state->text.character_width + state->text.character_spacing ), 20 );
}

#endif

