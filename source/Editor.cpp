#include "Editor.hpp"
#include "Log.hpp"
#include "Utils.hpp"
#include "Bitmap.hpp"
#include "MapDisplay.hpp"

using namespace editor;

const Real32 State::c_camera_speed = 20.0f;

static State* get_state ( GameMemory& game_memory )
{
     return reinterpret_cast<MemoryLocations*>( game_memory.location ( ) )->state;
}

void State::mouse_button_changed_down ( bool left )
{
     Int32 sx = mouse_x - meters_to_pixels ( camera.x ( ) );
     Int32 sy = mouse_y - meters_to_pixels ( camera.y ( ) );

     Int32 tx = sx / bryte::Map::c_tile_dimension_in_pixels;
     Int32 ty = sy / bryte::Map::c_tile_dimension_in_pixels;

     switch ( mode ) {
     default:
          ASSERT ( 0 );
          break;
     case Mode::tile:
          // on right click, set solids
          if ( !left ) {
               if ( tx >= 0 && tx < map.width ( ) &&
                    ty >= 0 && ty < map.height ( ) ) {
                    auto solid = map.get_coordinate_solid ( tx, ty );
                    map.set_coordinate_solid ( tx, ty, !solid );
               }
          }
          break;
     case Mode::decor:
          if ( left ) {
               if ( tx >= 0 && tx < map.width ( ) &&
                    ty >= 0 && ty < map.height ( ) ) {
                    map.set_coordinate_decor ( tx, ty, current_decor );
               }
          }
          break;
     case Mode::light:
     {
          bryte::Map::Lamp* lamp = map.check_position_lamp ( tx, ty );

          if ( left ) {
               if ( lamp ) {
                    map.remove_lamp ( lamp );
               } else {
                    if ( tx >= 0 && tx < map.width ( ) &&
                         ty >= 0 && ty < map.height ( ) ) {
                         map.add_lamp ( tx, ty, current_lamp );
                    }
               }

               map.reset_light ( );
          } else {
               current_lamp++;
               current_lamp %= bryte::Map::c_unique_lamp_count;
          }
     } break;
     case Mode::exit:
     {
          bryte::Map::Exit* exit = map.check_position_exit ( tx, ty );

          if ( left ) {
               if ( exit ) {
                    map.remove_exit ( exit );
               } else {
                    if ( tx >= 0 && tx < map.width ( ) &&
                         ty >= 0 && ty < map.height ( ) ) {
                         map.add_exit ( tx, ty );
                    }
               }
          } else {
               exit->exit_index++;
               exit->exit_index %= bryte::Map::c_max_exits;
          }
     } break;
     }
}

void State::option_button_changed_down ( bool up )
{
     Int32 sx = mouse_x - meters_to_pixels ( camera.x ( ) );
     Int32 sy = mouse_y - meters_to_pixels ( camera.y ( ) );

     Int32 tx = sx / bryte::Map::c_tile_dimension_in_pixels;
     Int32 ty = sy / bryte::Map::c_tile_dimension_in_pixels;

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
               if ( current_tile < tilesheet->w / bryte::Map::c_tile_dimension_in_pixels ) {
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
               if ( current_decor < decorsheet->w / bryte::Map::c_tile_dimension_in_pixels ) {
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
          bryte::Map::Exit* exit = map.check_position_exit ( tx, ty );

          if ( exit ) {
               if ( up ) {
                    exit->map_index--;
               } else {
                    exit->map_index++;
               }
          }
     } break;
     }

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

     if ( !load_bitmap_with_game_memory ( state->tilesheet,  game_memory,
                                          state->settings->map_tilesheet_filename ) ) {
          LOG_ERROR ( "Failed to load: '%s'\n", state->settings->map_tilesheet_filename );
          return false;
     }

     if ( !load_bitmap_with_game_memory ( state->decorsheet, game_memory,
                                          state->settings->map_decorsheet_filename ) ) {
          LOG_ERROR ( "Failed to load: '%s'\n", state->settings->map_decorsheet_filename );
          return false;
     }

     if ( !load_bitmap_with_game_memory ( state->lampsheet, game_memory,
                                          state->settings->map_lampsheet_filename ) ) {
          LOG_ERROR ( "Failed to load: '%s'\n", state->settings->map_lampsheet_filename );
          return false;
     }

     if ( state->settings->map_load_filename ) {
          if ( !state->settings->map_save_filename ) {
               state->settings->map_save_filename = state->settings->map_load_filename;
          }

          state->map.load ( state->settings->map_load_filename );
     } else {
          state->map.initialize ( state->settings->map_width, state->settings->map_height );
     }

     state->current_tile = 1;

     state->camera.set ( 0.0f, 0.0f );

     state->left_button_down = false;
     state->right_button_down = false;

     state->mode = Mode::tile;

     return true;
}

extern "C" Void game_destroy ( GameMemory& game_memory )
{
     State* state = get_state ( game_memory );

     SDL_FreeSurface ( state->tilesheet );
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
          case SDL_SCANCODE_O:
               if ( key_change.down ) {
                    state->map.save ( state->settings->map_save_filename );
               }
               break;
          case SDL_SCANCODE_I:
               if ( key_change.down ) {
                    state->map.load ( state->settings->map_save_filename );
               }
               break;
          case SDL_SCANCODE_B:
               if ( key_change.down ) {
                    state->draw_solids = !state->draw_solids;
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

     Int32 sx = state->mouse_x - meters_to_pixels ( state->camera.x ( ) );
     Int32 sy = state->mouse_y - meters_to_pixels ( state->camera.y ( ) );

     Int32 tx = sx / bryte::Map::c_tile_dimension_in_pixels;
     Int32 ty = sy / bryte::Map::c_tile_dimension_in_pixels;

     switch ( state->mode ) {
     default:
          ASSERT ( 0 );
          break;
     case Mode::tile:
          if ( tx >= 0 && tx < state->map.width ( ) &&
               ty >= 0 && ty < state->map.height ( ) ) {
               if ( state->left_button_down ) {
                    state->map.set_coordinate_value ( tx, ty, state->current_tile );
               }
          }
          break;
     case Mode::decor:
          break;
     case Mode::light:
          sprintf ( state->message_buffer, "BASE %d", state->map.base_light_value ( ) );
          break;
     case Mode::exit:
     {
          bryte::Map::Exit* exit = state->map.check_position_exit ( tx, ty );

          if ( exit ) {
               sprintf ( state->message_buffer, "MAP %d EXIT %d", exit->map_index, exit->exit_index );
          }
     } break;
     }
}

static Void render_map_solids ( SDL_Surface* back_buffer, bryte::Map& map, Real32 camera_x, Real32 camera_y )
{
     Uint32 red_color = SDL_MapRGB ( back_buffer->format, 255, 0, 0 );

     for ( Int32 y = 0; y < static_cast<Int32>( map.height ( ) ); ++y ) {
          for ( Int32 x = 0; x < static_cast<Int32>( map.width ( ) ); ++x ) {

               bool is_solid = map.get_coordinate_solid ( x, y );

               if ( !is_solid ) {
                    continue;
               }

               SDL_Rect solid_rect_b { 0, 0, bryte::Map::c_tile_dimension_in_pixels, 1 };
               SDL_Rect solid_rect_l { 0, 0, 1, bryte::Map::c_tile_dimension_in_pixels };
               SDL_Rect solid_rect_t { 0, 0, bryte::Map::c_tile_dimension_in_pixels, 1 };
               SDL_Rect solid_rect_r { 0, 0, 1, bryte::Map::c_tile_dimension_in_pixels };

               solid_rect_b.x = x * bryte::Map::c_tile_dimension_in_pixels;
               solid_rect_b.y = y * bryte::Map::c_tile_dimension_in_pixels;
               solid_rect_l.x = x * bryte::Map::c_tile_dimension_in_pixels;
               solid_rect_l.y = y * bryte::Map::c_tile_dimension_in_pixels;
               solid_rect_t.x = x * bryte::Map::c_tile_dimension_in_pixels;
               solid_rect_t.y = y * bryte::Map::c_tile_dimension_in_pixels +
                                bryte::Map::c_tile_dimension_in_pixels - 1;
               solid_rect_r.x = x * bryte::Map::c_tile_dimension_in_pixels +
                                bryte::Map::c_tile_dimension_in_pixels - 1;
               solid_rect_r.y = y * bryte::Map::c_tile_dimension_in_pixels;

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

static Void render_current_tile ( SDL_Surface* back_buffer, SDL_Surface* tilesheet,
                                  Int32 mouse_x, Int32 mouse_y, int current_tile )
{
     SDL_Rect tile_rect { mouse_x, back_buffer->h - mouse_y,
                          bryte::Map::c_tile_dimension_in_pixels, bryte::Map::c_tile_dimension_in_pixels };
     SDL_Rect clip_rect { current_tile * bryte::Map::c_tile_dimension_in_pixels, 0,
                          bryte::Map::c_tile_dimension_in_pixels, bryte::Map::c_tile_dimension_in_pixels };

     SDL_BlitSurface ( tilesheet, &clip_rect, back_buffer, &tile_rect );
}

static Void render_current_decor ( SDL_Surface* back_buffer, SDL_Surface* decorsheet,
                                   Int32 mouse_x, Int32 mouse_y, int current_decor )
{
     if ( current_decor <= 0 ) {
          return;
     }

     current_decor--;

     SDL_Rect tile_rect { mouse_x, back_buffer->h - mouse_y,
                          bryte::Map::c_tile_dimension_in_pixels, bryte::Map::c_tile_dimension_in_pixels };
     SDL_Rect clip_rect { current_decor * bryte::Map::c_tile_dimension_in_pixels, 0,
                          bryte::Map::c_tile_dimension_in_pixels, bryte::Map::c_tile_dimension_in_pixels };

     SDL_BlitSurface ( decorsheet, &clip_rect, back_buffer, &tile_rect );
}

static Void render_current_lamp ( SDL_Surface* back_buffer, SDL_Surface* lampsheet,
                                  Int32 mouse_x, Int32 mouse_y, int current_lamp )
{
     SDL_Rect tile_rect { mouse_x, back_buffer->h - mouse_y,
                          bryte::Map::c_tile_dimension_in_pixels, bryte::Map::c_tile_dimension_in_pixels };
     SDL_Rect clip_rect { current_lamp * bryte::Map::c_tile_dimension_in_pixels, 0,
                          bryte::Map::c_tile_dimension_in_pixels, bryte::Map::c_tile_dimension_in_pixels };

     SDL_BlitSurface ( lampsheet, &clip_rect, back_buffer, &tile_rect );
}

extern "C" Void game_render ( GameMemory& game_memory, SDL_Surface* back_buffer )
{
     State* state = get_state ( game_memory );

     render_map ( back_buffer, state->tilesheet, state->map, state->camera.x ( ), state->camera.y ( ) );
     render_map_decor ( back_buffer, state->decorsheet, state->map, state->camera.x ( ), state->camera.y ( ) );
     render_map_lamps ( back_buffer, state->lampsheet, state->map, state->camera.x ( ), state->camera.y ( ) );
     render_map_exits ( back_buffer, state->map, state->camera.x ( ), state->camera.y ( ) );

     render_light ( back_buffer, state->map, state->camera.x ( ), state->camera.y ( ) );

     if ( state->draw_solids ) {
          render_map_solids ( back_buffer, state->map, state->camera.x ( ), state->camera.y ( ) );
     }

     SDL_Rect hud_rect { 0, 0, back_buffer->w, 20 };
     SDL_FillRect ( back_buffer, &hud_rect, SDL_MapRGB ( back_buffer->format, 255, 255, 255 ) );

     switch ( state->mode ) {
     default:
          ASSERT ( 0 );
          break;
     case Mode::tile:
          render_current_tile ( back_buffer, state->tilesheet, state->mouse_x, state->mouse_y,
                                state->current_tile );

          state->text.render ( back_buffer, "TILE MODE", 10, 10 );
          break;
     case Mode::decor:
          render_current_decor ( back_buffer, state->decorsheet, state->mouse_x, state->mouse_y,
                                 state->current_decor );

          state->text.render ( back_buffer, "DECOR MODE", 10, 10 );
          break;
     case Mode::light:
          render_current_lamp ( back_buffer, state->lampsheet, state->mouse_x, state->mouse_y,
                                 state->current_lamp );

          state->text.render ( back_buffer, "LIGHT MODE", 10, 10 );
          break;
     case Mode::exit:
          state->text.render ( back_buffer, "EXIT MODE", 10, 10 );
          break;
     }

     state->text.render ( back_buffer, state->message_buffer, 80, 10 );
}

