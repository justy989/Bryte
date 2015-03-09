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
          Interactive& interactive = place_or_clear_interactive ( Interactive::Type::exit,
                                                                  mouse_tile_x, mouse_tile_y );

          if ( interactive.type == Interactive::Type::exit ) {
               interactive.interactive_exit.direction    = static_cast<Direction>( current_exit_direction );
               interactive.interactive_exit.state        = static_cast<Exit::State>( current_exit_state * 2 );
          }
     } break;
     case Mode::lever:
     {
          Interactive& interactive = place_or_clear_interactive ( Interactive::Type::lever,
                                                                  mouse_tile_x, mouse_tile_y );

          if ( interactive.type == Interactive::Type::lever ) {
               interactive.type = Interactive::Type::lever;
               interactive.reset ( );
          }
     } break;
     case Mode::pushable_block:
     {
          Interactive& interactive = place_or_clear_interactive ( Interactive::Type::pushable_block,
                                                                  mouse_tile_x, mouse_tile_y );

          if ( interactive.type == Interactive::Type::pushable_block ) {
               interactive.interactive_pushable_block.one_time = true;
               interactive.interactive_pushable_block.state = current_pushable_solid ?
                    PushableBlock::State::solid : PushableBlock::State::idle;
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
          Interactive& interactive = place_or_clear_interactive ( Interactive::Type::torch,
                                                                  mouse_tile_x, mouse_tile_y );

          if ( interactive.type == Interactive::Type::torch ) {
               interactive.interactive_torch.element = static_cast<Element>(current_torch);
          }
     } break;
     case Mode::pushable_torch:
     {
          Interactive& interactive = place_or_clear_interactive ( Interactive::Type::pushable_torch,
                                                                  mouse_tile_x, mouse_tile_y );

          if ( interactive.type == Interactive::Type::pushable_torch ) {
               interactive.interactive_pushable_torch.torch.element =
                    static_cast<Element>(current_pushable_torch);
          }
     } break;
     case Mode::light_detector:
     {
          Interactive& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );

          if ( interactive.underneath.type == UnderneathInteractive::Type::none ) {
               interactive.underneath.type = UnderneathInteractive::Type::light_detector;
               Auto& detector = interactive.underneath.underneath_light_detector;
               detector.type = static_cast<LightDetector::Type>( current_light_detector_bryte );
               if ( detector.type == LightDetector::Type::bryte ) {
                    detector.below_value = true;
               } else {
                    detector.below_value = false;
               }
          } else {
               interactive.underneath.type = UnderneathInteractive::Type::none;
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
     case Mode::all_killed:
     {
          Map::Location location =  { static_cast<Uint8>( mouse_tile_x ), static_cast<Uint8>( mouse_tile_y ) };
          map.set_activate_location_on_all_enemies_killed ( location );
     } break;
     case Mode::bombable_block:
          place_or_clear_interactive ( Interactive::Type::bombable_block, mouse_tile_x, mouse_tile_y );
          break;
     case Mode::turret:
     {
          Interactive& interactive = place_or_clear_interactive ( Interactive::Type::turret,
                                                                  mouse_tile_x, mouse_tile_y );

          if ( interactive.type == Interactive::Type::turret ) {
               interactive.interactive_turret.facing = static_cast<Direction>(current_turret_direction);
               interactive.interactive_turret.automatic = static_cast<Bool>(current_turret_automatic);
          }
     } break;
     case Mode::ice:
     {
          Interactive& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );
          UnderneathInteractive& underneath = interactive.underneath;

          if ( underneath.type == UnderneathInteractive::Type::ice ) {
               underneath.type = UnderneathInteractive::Type::none;
               interactive.reset ( );
          } else {
               underneath.type = UnderneathInteractive::Type::ice;
               interactive.reset ( );
          }
     } break;
     case Mode::moving_walkway:
     {
          Interactive& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );
          UnderneathInteractive& underneath = interactive.underneath;

          if ( underneath.type == UnderneathInteractive::Type::moving_walkway ) {
               underneath.type = UnderneathInteractive::Type::none;
               interactive.reset ( );
          } else {
               underneath.type = UnderneathInteractive::Type::moving_walkway;
               interactive.reset ( );
               underneath.underneath_moving_walkway.facing = static_cast<Direction>( current_moving_walkway );
          }
     } break;
     case Mode::ice_detector:
     {
          Interactive& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );

          if ( interactive.underneath.type == UnderneathInteractive::Type::none ) {
               interactive.underneath.type = UnderneathInteractive::Type::ice_detector;
          } else {
               interactive.underneath.type = UnderneathInteractive::Type::none;
          }
     } break;
     case Mode::secret:
          map.set_secret_location ( Map::Location { static_cast<Uint8>( mouse_tile_x ),
                                                    static_cast<Uint8>( mouse_tile_y ) } );
          break;
     case Mode::hole:
     {
          Interactive& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );

          if ( interactive.underneath.type == UnderneathInteractive::Type::none ) {
               interactive.underneath.type = UnderneathInteractive::Type::hole;
          } else {
               interactive.underneath.type = UnderneathInteractive::Type::none;
          }
     } break;
     case Mode::portal:
     {
          Interactive& interactive = place_or_clear_interactive ( Interactive::Type::portal,
                                                                  mouse_tile_x, mouse_tile_y );

          if ( interactive.type == Interactive::Type::portal ) {
               interactive.type = Interactive::Type::portal;
               interactive.reset ( );
               interactive.interactive_portal.on = static_cast<Bool>( current_portal );
          }
     } break;
     case Mode::border:
     {
          Auto& border_exit = map.get_border_exit ( static_cast<Direction>( current_border ) );

          border_exit.bottom_left = Map::Location { static_cast<Uint8>( mouse_tile_x ),
                                                    static_cast<Uint8>( mouse_tile_y ) };
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
          if ( current_tile_flag & Map::TileFlags::solid ) {
               current_solid = !map.get_coordinate_solid ( mouse_tile_x, mouse_tile_y );
               map.set_coordinate_solid ( mouse_tile_x, mouse_tile_y, current_solid );
          } else if ( current_tile_flag & Map::TileFlags::invisible ) {
               current_invisible = !map.get_coordinate_invisible ( mouse_tile_x, mouse_tile_y );
               map.set_coordinate_invisible ( mouse_tile_x, mouse_tile_y, current_invisible );
          }
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
               } else {
                    current_pushable_solid = !current_pushable_solid;
               }
          }
     } break;
     case Mode::enemy:
          current_enemy_direction++;
          current_enemy_direction %= Direction::count;
          break;
     case Mode::torch:
          current_torch++;
          current_torch %= 3;
          break;
     case Mode::pushable_torch:
          current_pushable_torch++;
          current_pushable_torch %= 3;
          break;
     case Mode::light_detector:
     {
          Interactive& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );

          if ( interactive.underneath.type == UnderneathInteractive::Type::light_detector ) {
               current_interactive_x = mouse_tile_x;
               current_interactive_y = mouse_tile_y;
               track_current_interactive = true;
          } else {
               if ( track_current_interactive ) {
                    Auto& interactive = interactives.get_from_tile ( current_interactive_x,
                                                                        current_interactive_y );
                    ASSERT ( interactive.underneath.type == UnderneathInteractive::Type::light_detector );
                    Auto& light_detector = interactive.underneath.underneath_light_detector;
                    light_detector.activate_coordinate_x = mouse_tile_x;
                    light_detector.activate_coordinate_y = mouse_tile_y;
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
     case Mode::all_killed:
     {
          map.set_activate_location_on_all_enemies_killed ( Map::Location { 0, 0 } );
     } break;
     case Mode::bombable_block:
     {
          Interactive& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );

          if ( interactive.type == Interactive::Type::bombable_block ) {
               interactive.type = Interactive::Type::none;
               interactive.reset ( );
          }
     } break;
     case Mode::turret:
          current_turret_automatic = !current_turret_automatic;
          break;
     case Mode::ice_detector:
     {
          Interactive& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );

          if ( interactive.underneath.type == UnderneathInteractive::Type::ice_detector ) {
               current_interactive_x = mouse_tile_x;
               current_interactive_y = mouse_tile_y;
               track_current_interactive = true;
          } else {
               if ( track_current_interactive ) {
                    Auto& interactive = interactives.get_from_tile ( current_interactive_x,
                                                                     current_interactive_y );
                    ASSERT ( interactive.underneath.type == UnderneathInteractive::Type::ice_detector );
                    Auto& detector = interactive.underneath.underneath_ice_detector;
                    detector.activate_coordinate_x = mouse_tile_x;
                    detector.activate_coordinate_y = mouse_tile_y;
                    track_current_interactive = false;
               }
          }
     } break;
     case Mode::secret:
          map.set_secret_clear_tile ( Map::Location { static_cast<Uint8>( mouse_tile_x ),
                                                      static_cast<Uint8>( mouse_tile_y ) } );
          break;
     case Mode::portal:
     {
          Interactive& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );

          if ( !track_current_interactive &&
               interactive.type == Interactive::Type::portal ) {
               current_interactive_x = mouse_tile_x;
               current_interactive_y = mouse_tile_y;
               track_current_interactive = true;
          } else {
               Auto& portal = interactives.get_from_tile ( current_interactive_x,
                                                           current_interactive_y );
               ASSERT ( portal.type == Interactive::Type::portal );

               portal.interactive_portal.destination_x = mouse_tile_x;
               portal.interactive_portal.destination_y = mouse_tile_y;

               track_current_interactive = false;
          }
     } break;
     case Mode::border:
     {
          Auto& border_exit = map.get_border_exit ( static_cast<Direction>( current_border ) );

          border_exit.map_bottom_left = Map::Location { static_cast<Uint8>( mouse_tile_x ),
                                                        static_cast<Uint8>( mouse_tile_y ) };
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
     case Mode::turret:
          current_turret_direction++;
          current_turret_direction %= 4;
          break;
     case Mode::moving_walkway:
          current_moving_walkway++;
          current_moving_walkway %= 4;
          break;
     case Mode::portal:
          current_portal = !current_portal;
          break;
     case Mode::border:
     {
          Auto& border_exit = map.get_border_exit ( static_cast<Direction>( current_border ) );

          border_exit.map_index++;
     } break;
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
     case Mode::turret:
          current_turret_direction--;
          current_turret_direction %= 4;
          break;
     case Mode::moving_walkway:
          current_moving_walkway--;
          current_moving_walkway %= 4;
          break;
     case Mode::portal:
          current_portal = !current_portal;
          break;
     case Mode::border:
     {
          Auto& border_exit = map.get_border_exit ( static_cast<Direction>( current_border ) );

          border_exit.map_index--;
     } break;
     }
}

Void State::mouse_scrolled ( Int32 scroll )
{
     switch ( mode )
     {
     default:
          break;
     case Mode::tile:
          current_tile_flag <<= 1;
          if ( current_tile_flag > Map::TileFlags::invisible ) {
               current_tile_flag = Map::TileFlags::solid;
          }
          break;
     case Mode::enemy:
          if ( mouse_on_map ( ) ) {
               Auto* enemy_spawn = map.check_coordinates_for_enemy_spawn ( mouse_tile_x, mouse_tile_y );

               if ( enemy_spawn ) {
                    Int32 value = static_cast<Int32>( enemy_spawn->drop );
                    value += scroll;
                    if ( value >= 0 ) {
                         enemy_spawn->drop = static_cast<Pickup::Type>( value % Pickup::Type::count );
                    }
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
     case Mode::border:
          break;
     }
}

Interactive& State::place_or_clear_interactive ( Interactive::Type type,
                                                 Int32 mouse_tile_x, Int32 mouse_tile_y )
{
     Interactive& interactive = interactives.get_from_tile ( mouse_tile_x, mouse_tile_y );

     if ( interactive.type == type ) {
          interactive.type = Interactive::Type::none;
          interactive.reset ( );
     } else {
          interactive.type = type;
          interactive.reset ( );
     }

     return interactive;
}

extern "C" Bool game_init ( GameMemory& game_memory, Void* settings )
{
     MemoryLocations* memory_locations = GAME_PUSH_MEMORY ( game_memory, MemoryLocations );
     State* state = GAME_PUSH_MEMORY ( game_memory, State);

     state->settings = reinterpret_cast<Settings*>( settings );

     memory_locations->state = state;

     if ( !state->text.load_surfaces ( game_memory ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( state->mode_icons_surface, game_memory,
                                          "editor_mode_icons.bmp" ) ) {
          return false;
     }

     if ( !state->map_display.load_surfaces ( game_memory,
                                              state->settings->map_tilesheet_filename,
                                              state->settings->map_decorsheet_filename,
                                              state->settings->map_lampsheet_filename ) ) {
          return false;
     }

     if ( !state->character_display.load_surfaces ( game_memory ) ) {
          return false;
     }

     if ( !state->interactives_display.load_surfaces ( game_memory ) ) {
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
     state->current_tile_flag            = Map::TileFlags::solid;
     state->current_solid                = false;
     state->current_invisible            = false;
     state->current_decor                = 0;
     state->current_lamp                 = 0;
     state->current_enemy                = 0;
     state->current_enemy_direction      = 0;
     state->current_enemy_drop           = 0;
     state->current_exit_direction       = 0;
     state->current_exit_state           = 0;
     state->current_pushable_solid       = 0;
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
                    state->mode = static_cast<Mode>( static_cast<Int32>( state->mode ) + 1 );
                    state->mode = static_cast<Mode>( static_cast<Int32>( state->mode ) % Mode::count );
                    state->current_field = 0;
                    state->track_current_interactive = false;
               }
               break;
          case SDL_SCANCODE_N:
               if ( key_change.down ) {
                    Int32 new_mode = static_cast<Int32>( state->mode ) - 1;
                    // wrap around if necessary
                    if ( new_mode <= 0 ) {
                         new_mode = static_cast<Mode>( static_cast<Int32>(Mode::count) - 1 );
                    }
                    state->mode = static_cast<Mode>( new_mode );
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
          if ( state->current_tile_flag & Map::TileFlags::solid ) {
               sprintf ( state->message_buffer, "SOLIDS\n" );
          } else if ( state->current_tile_flag & Map::TileFlags::invisible ) {
               sprintf ( state->message_buffer, "INVIS\n" );
          }

          if ( !state->mouse_on_map ( ) ) {
               break;
          }

          if ( state->left_button_down ) {
               state->map.set_coordinate_value ( state->mouse_tile_x, state->mouse_tile_y,
                                                 state->current_tile );
          } else if ( state->right_button_down ) {
               if ( state->current_tile_flag & Map::TileFlags::solid ) {
                    state->map.set_coordinate_solid ( state->mouse_tile_x, state->mouse_tile_y,
                                                      state->current_solid );
               } else if ( state->current_tile_flag & Map::TileFlags::invisible ) {
                    state->map.set_coordinate_invisible ( state->mouse_tile_x, state->mouse_tile_y,
                                                          state->current_invisible );
               }
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
               sprintf ( state->message_buffer, "ACT %d %d SOLID %d",
                         pushable_block.activate_coordinate_y,
                         pushable_block.activate_coordinate_x,
                         pushable_block.state == PushableBlock::State::solid );
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

          if ( interactive.underneath.type == UnderneathInteractive::Type::light_detector ) {
               Auto& detector = interactive.underneath.underneath_light_detector;
               sprintf ( state->message_buffer, "ACT %d %d",
                         detector.activate_coordinate_x, detector.activate_coordinate_y );
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
                         pressure_plate.activate_coordinate_x,
                         pressure_plate.activate_coordinate_y );
          }
     } break;
     case Mode::all_killed:
     {
          Map::Location loc = state->map.activate_on_all_enemies_killed ( );
          sprintf ( state->message_buffer, "ACT %d %d", loc.x, loc.y );
     } break;
     case Mode::turret:
     {
          if ( !state->mouse_on_map ( ) ) {
               break;
          }

          Auto& interactive = state->interactives.get_from_tile ( state->mouse_tile_x, state->mouse_tile_y );

          if ( interactive.type == Interactive::Type::turret ) {
               Auto& turret = interactive.interactive_turret;
               sprintf ( state->message_buffer, "DIR %d AUTO %d", turret.facing, turret.automatic );
          }
     } break;
     case Mode::ice_detector:
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

          if ( interactive.underneath.type == UnderneathInteractive::Type::ice_detector ) {
               Auto& ice_detector = interactive.underneath.underneath_ice_detector;
               sprintf ( state->message_buffer, "ACT %d %d",
                         ice_detector.activate_coordinate_x, ice_detector.activate_coordinate_y );
          }
     } break;
     case Mode::portal:
     {
          if ( state->track_current_interactive ) {
               sprintf ( state->message_buffer, "S DST %d %d",
                         state->current_interactive_x, state->current_interactive_y );
               break;
          }

          if ( !state->mouse_on_map ( ) ) {
               break;
          }

          Auto& interactive = state->interactives.get_from_tile ( state->mouse_tile_x, state->mouse_tile_y );

          if ( interactive.type == Interactive::Type::portal ) {
               Auto& portal = interactive.interactive_portal;
               sprintf ( state->message_buffer, "DST %d %d",
                         portal.destination_x, portal.destination_y );
          }
     } break;
     case Mode::border:
     {
          const Char8* c_direction_names [ Direction::count ] =
          {
               "LEFT",
               "UP",
               "RIGHT",
               "DOWN"
          };

          Auto& border_exit = state->map.get_border_exit ( static_cast<Direction>( state->current_border ) );
          sprintf ( state->message_buffer,
                    "SIDE %s LOC %d %d MAP %d MLOC %d %d",
                    c_direction_names [ state->current_border ],
                    border_exit.bottom_left.x, border_exit.bottom_left.y,
                    border_exit.map_index, border_exit.map_bottom_left.x,
                    border_exit.map_bottom_left.y );
     } break;
     }

     state->map.reset_light ( );

     state->interactives.contribute_light ( state->map );
}

static Void render_map_solids ( SDL_Surface* back_buffer, Map& map, Real32 camera_x, Real32 camera_y )
{
     Uint32 red_color = SDL_MapRGB ( back_buffer->format, 255, 0, 0 );
     Uint32 green_color = SDL_MapRGB ( back_buffer->format, 0, 255, 0 );

     for ( Int32 y = 0; y < static_cast<Int32>( map.height ( ) ); ++y ) {
          for ( Int32 x = 0; x < static_cast<Int32>( map.width ( ) ); ++x ) {

               bool is_solid = map.get_coordinate_solid ( x, y );

               if ( is_solid ) {
                    SDL_Rect outline_rect { ( x * Map::c_tile_dimension_in_pixels ),
                                            ( y * Map::c_tile_dimension_in_pixels ),
                                            Map::c_tile_dimension_in_pixels,
                                            Map::c_tile_dimension_in_pixels};

                    world_to_sdl ( outline_rect, back_buffer, camera_x, camera_y );

                    render_rect_outline ( back_buffer, outline_rect, red_color );
               }

               Bool is_invisible = map.get_coordinate_invisible ( x, y );

               if ( is_invisible ) {
                    const Int32 c_half_tile = Map::c_tile_dimension_in_pixels / 2;
                    SDL_Rect outline_rect { ( x * Map::c_tile_dimension_in_pixels ) + c_half_tile,
                                            ( y * Map::c_tile_dimension_in_pixels ) + c_half_tile,
                                            1,
                                            1 };

                    world_to_sdl ( outline_rect, back_buffer, camera_x, camera_y );

                    render_rect_outline ( back_buffer, outline_rect, green_color );
               }
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

static Int32 calc_icon_x ( Int32 index, Int32 icons_per_row, Int32 icon_starting_x, Int32 icon_dimension )
{
     return ( ( index % icons_per_row ) * icon_dimension ) + icon_starting_x + 1;
}

static Void render_mode_icons ( SDL_Surface* back_buffer, SDL_Surface* icon_surface, Mode mode )
{
     static const Int32 c_icon_dimension  = 11;
     static const Int32 c_icon_starting_x = 2;
     static const Int32 c_icons_per_row   = 17;

     for ( int i = 0; i < Mode::count; ++i ) {
          SDL_Rect dest_rect { calc_icon_x ( i, c_icons_per_row, c_icon_starting_x, c_icon_dimension ),
                               3 + ( ( i / c_icons_per_row ) * ( c_icon_dimension + 2 ) ),
                               c_icon_dimension, c_icon_dimension };
          SDL_Rect clip_rect { i * c_icon_dimension, 0, c_icon_dimension, c_icon_dimension };

          SDL_BlitSurface ( icon_surface, &clip_rect, back_buffer, &dest_rect );
     }

     static const Int32 c_border_dimension = c_icon_dimension + 2;

     Int32 mode_value = static_cast<Int32>( mode );
     Int32 x = calc_icon_x ( mode_value, c_icons_per_row, c_icon_starting_x, c_icon_dimension ) - 1;
     Int32 y = 2 + ( ( mode_value / c_icons_per_row ) * ( c_icon_dimension + 2 ) );
     Uint32 red_color = SDL_MapRGB ( back_buffer->format, 255, 0, 0 );

     SDL_Rect outline_rect { x, y, c_border_dimension, c_border_dimension };

     render_rect_outline ( back_buffer, outline_rect, red_color );
}

static Void render_secret ( SDL_Surface* back_buffer, const Map::Secret& secret,
                            Real32 camera_x, Real32 camera_y )
{
     Uint32 red_color = SDL_MapRGB ( back_buffer->format, 255, 0, 0 );
     Uint32 green_color = SDL_MapRGB ( back_buffer->format, 0, 255, 0 );

     SDL_Rect location_rect { ( secret.location.x * Map::c_tile_dimension_in_pixels ),
                              ( secret.location.y * Map::c_tile_dimension_in_pixels ),
                              Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };
     SDL_Rect secret_rect { ( secret.clear_tile.x * Map::c_tile_dimension_in_pixels ),
                            ( secret.clear_tile.y * Map::c_tile_dimension_in_pixels ),
                            Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

     world_to_sdl ( location_rect, back_buffer, camera_x, camera_y );
     world_to_sdl ( secret_rect, back_buffer, camera_x, camera_y );

     render_rect_outline ( back_buffer, location_rect, red_color );
     render_rect_outline ( back_buffer, secret_rect, green_color );
}

extern "C" Void game_render ( GameMemory& game_memory, SDL_Surface* back_buffer )
{
     State* state = get_state ( game_memory );

     // map
     state->map_display.render ( back_buffer, state->map, state->camera.x ( ), state->camera.y ( ), true );

     // interactives
     state->interactives_display.render ( back_buffer, state->interactives, state->map,
                                          state->camera.x ( ), state->camera.y ( ), true );

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

     // render the secret in a specific mode
     if ( state->mode == Mode::secret ) {
          render_secret ( back_buffer, state->map.secret ( ), state->camera.x ( ), state->camera.y ( ) );
     }

     // ui
     SDL_Rect hud_rect { 0, 0, back_buffer->w, 44 };
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
                                state->interactives_display.light_detector_sheet,
                                state->mouse_x, state->mouse_y,
                                2 * state->current_light_detector_bryte,
                                0 );
          break;
     case Mode::exit:
          render_current_icon ( back_buffer,
                                state->interactives_display.exit_sheet,
                                state->mouse_x, state->mouse_y,
                                state->current_exit_direction,
                                state->current_exit_state * 2 );
          break;
     case Mode::pressure_plate:
          render_current_icon ( back_buffer,
                                state->interactives_display.interactive_sheet,
                                state->mouse_x, state->mouse_y,
                                0,
                                ( Interactive::Type::count - 2 ) + UnderneathInteractive::Type::pressure_plate );
          break;
     case Mode::popup_block:
          render_current_icon ( back_buffer,
                                state->interactives_display.interactive_sheet,
                                state->mouse_x, state->mouse_y,
                                0,
                                Interactive::Type::pushable_block - 1 );
          break;
     case Mode::bombable_block:
          render_current_icon ( back_buffer,
                                state->interactives_display.interactive_sheet,
                                state->mouse_x, state->mouse_y,
                                0,
                                Interactive::Type::bombable_block - 1);
          break;
     case Mode::turret:
          render_current_icon ( back_buffer,
                                state->interactives_display.interactive_sheet,
                                state->mouse_x, state->mouse_y,
                                state->current_turret_direction,
                                Interactive::Type::turret - 1 );
          break;
     case Mode::ice:
          render_current_icon ( back_buffer,
                                state->interactives_display.interactive_sheet,
                                state->mouse_x, state->mouse_y,
                                0,
                                ( Interactive::Type::count - 2 ) + UnderneathInteractive::Type::ice );
          break;
     case Mode::moving_walkway:
          render_current_icon ( back_buffer,
                                state->interactives_display.moving_walkway_sheet,
                                state->mouse_x, state->mouse_y,
                                0,
                                state->current_moving_walkway );
          break;
     case Mode::ice_detector:
          render_current_icon ( back_buffer,
                                state->interactives_display.interactive_sheet,
                                state->mouse_x, state->mouse_y,
                                0,
                                ( Interactive::Type::count - 2 ) + UnderneathInteractive::Type::ice_detector );
          break;
     }

     // text ui
     Char8 buffer [ 64 ];
     sprintf ( buffer, "T %d %d", state->mouse_tile_x, state->mouse_tile_y );
     state->text.render ( back_buffer, buffer, 210, 4 );

     sprintf ( buffer, "FIELD %d", state->current_field );
     state->text.render ( back_buffer, buffer, 210, 20 );

     state->text.render ( back_buffer, state->message_buffer,
                          1 * ( state->text.character_width + state->text.character_spacing ), 30 );
}

#endif

