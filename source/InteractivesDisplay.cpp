#include "InteractivesDisplay.hpp"
#include "Map.hpp"
#include "GameMemory.hpp"
#include "Bitmap.hpp"

using namespace bryte;

Bool InteractivesDisplay::load_surfaces ( GameMemory& game_memory, const Char8* exitsheet_filepath )
{
     if ( !load_bitmap_with_game_memory ( lever_sheet, game_memory, "content/images/test_lever.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( pushable_block_sheet, game_memory,
                                          "content/images/test_pushable_block.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( torch_sheet, game_memory, "content/images/test_torch.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( pushable_torch_sheet, game_memory,
                                          "content/images/test_pushable_torch.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( bombable_block_sheet, game_memory,
                                          "content/images/test_bombable_block.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( turret_sheet, game_memory, "content/images/test_turret.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( pressure_plate_sheet, game_memory,
                                          "content/images/test_pressure_plate.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( moving_walkway_sheet, game_memory,
                                          "content/images/test_moving_walkway.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( popup_block_sheet, game_memory,
                                          "content/images/test_popup_block.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( ice_sheet, game_memory,
                                          "content/images/test_ice.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( light_detector_sheet, game_memory,
                                          "content/images/test_light_detector.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( exit_sheet, game_memory, exitsheet_filepath ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( hole_sheet, game_memory,
                                          "content/images/test_hole.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( torch_element_sheet, game_memory,
                                          "content/images/torch_fire.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( ice_detector_sheet, game_memory,
                                          "content/images/test_ice_detector.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( portal_sheet, game_memory, "content/images/test_portal.bmp" ) ) {
          return false;
     }

     if ( !load_bitmap_with_game_memory ( destructable_sheet, game_memory,
                                          "content/images/test_destructable.bmp" ) ) {
          return false;
     }

     return true;
}

Void InteractivesDisplay::unload_surfaces ( )
{
     FREE_SURFACE ( lever_sheet );
     FREE_SURFACE ( pushable_block_sheet );
     FREE_SURFACE ( torch_sheet );
     FREE_SURFACE ( pushable_torch_sheet );
     FREE_SURFACE ( bombable_block_sheet );
     FREE_SURFACE ( turret_sheet );
     FREE_SURFACE ( pressure_plate_sheet );
     FREE_SURFACE ( moving_walkway_sheet );
     FREE_SURFACE ( popup_block_sheet );
     FREE_SURFACE ( ice_sheet );
     FREE_SURFACE ( light_detector_sheet );
     FREE_SURFACE ( ice_detector_sheet );
     FREE_SURFACE ( exit_sheet );
     FREE_SURFACE ( hole_sheet );
     FREE_SURFACE ( torch_element_sheet );
     FREE_SURFACE ( portal_sheet );
     FREE_SURFACE ( destructable_sheet );
}

Void InteractivesDisplay::clear ( )
{
     animation.clear ( );
     ice_animation.clear ( );
     moving_walkway_animation.clear ( );

     ice_sleep_counter = 0;
}

Void InteractivesDisplay::tick ( )
{
     animation.update_increment ( c_frames_per_update );
     moving_walkway_animation.update_increment ( c_moving_walkway_frame_delay, c_moving_walkway_frame_count );

     ice_sleep_counter++;

     if ( ice_sleep_counter > c_ice_frame_sleep ) {
          Int32 current_frame = ice_animation.frame;

          ice_animation.update_increment ( c_ice_frame_delay, c_ice_frame_count );

          // check for when we wrap around
          if ( current_frame > ice_animation.frame ) {
               ice_sleep_counter = 0;
               ice_animation.frame = 0;
               ice_animation.delay_tracker = 0;
          }
     }
}

Void InteractivesDisplay::render ( SDL_Surface* back_buffer, Interactives& interactives,
                                   const Map& map, Real32 camera_x, Real32 camera_y, Bool invisible )
{
     if ( invisible ) {
          for ( Int32 y = 0; y < interactives.height ( ); ++y ) {
               for ( Int32 x = 0; x < interactives.width ( ); ++x ) {
                    Location tile ( x, y );
                    Location position ( tile );

                    Map::convert_tiles_to_pixels ( &position );

                    Auto& interactive = interactives.get_from_tile ( tile );

                    SDL_Rect dest_rect { position.x, position.y,
                                         Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

                    world_to_sdl ( dest_rect, back_buffer, camera_x, camera_y );

                    render_underneath ( back_buffer, interactive.underneath, &dest_rect );
                    render_interactive ( back_buffer, interactive, &dest_rect );
               }
          }
     } else {
          for ( Int32 y = 0; y < interactives.height ( ); ++y ) {
               for ( Int32 x = 0; x < interactives.width ( ); ++x ) {
                    Location tile ( x, y );

                    if ( map.get_tile_location_invisible ( tile ) ) {
                         continue;
                    }

                    Location position ( tile );

                    Map::convert_tiles_to_pixels ( &position );

                    Auto& interactive = interactives.get_from_tile ( tile );

                    SDL_Rect dest_rect { position.x, position.y,
                                         Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

                    world_to_sdl ( dest_rect, back_buffer, camera_x, camera_y );

                    render_underneath ( back_buffer, interactive.underneath, &dest_rect );
                    render_interactive ( back_buffer, interactive, &dest_rect );
               }
          }
     }
}

Void InteractivesDisplay::render_underneath ( SDL_Surface* back_buffer, UnderneathInteractive& underneath,
                                              SDL_Rect* dest_rect )
{
     SDL_Rect clip_rect { 0, 0,
                          Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

     SDL_Surface* underneath_sheet = nullptr;

     switch ( underneath.type ) {
     default:
          ASSERT ( 0 );
          break;
     case UnderneathInteractive::Type::none:
          return;
     case UnderneathInteractive::Type::pressure_plate:
          underneath_sheet = pressure_plate_sheet;
          if ( underneath.underneath_pressure_plate.entered ) {
               clip_rect.x += Map::c_tile_dimension_in_pixels;
          }
          break;
     case UnderneathInteractive::Type::popup_block:
          underneath_sheet = popup_block_sheet;

          if ( underneath.underneath_popup_block.up ) {
               clip_rect.x += Map::c_tile_dimension_in_pixels;
          }
          break;
     case UnderneathInteractive::Type::ice:
          underneath_sheet = ice_sheet;
          clip_rect.x = ice_animation.frame * Map::c_tile_dimension_in_pixels;
          break;
     case UnderneathInteractive::Type::moving_walkway:
          underneath_sheet = moving_walkway_sheet;
          clip_rect.x = moving_walkway_animation.frame * Map::c_tile_dimension_in_pixels;
          clip_rect.y = underneath.underneath_moving_walkway.facing * Map::c_tile_dimension_in_pixels;
          break;
     case UnderneathInteractive::Type::light_detector:
          underneath_sheet = light_detector_sheet;

          if ( underneath.underneath_light_detector.type == LightDetector::Type::bryte ) {
               if ( !underneath.underneath_light_detector.below_value ) {
                    Int32 detector_frame = 1 + ( animation.frame % c_light_detector_frame_count );
                    clip_rect.x += Map::c_tile_dimension_in_pixels * detector_frame;
               }
          } else {
               clip_rect.y += Map::c_tile_dimension_in_pixels;

               if ( underneath.underneath_light_detector.below_value ) {
                    Int32 detector_frame = 1 + ( animation.frame % c_light_detector_frame_count );
                    clip_rect.x += Map::c_tile_dimension_in_pixels * detector_frame;
               }
          }
          break;
     case UnderneathInteractive::Type::ice_detector:
          underneath_sheet = ice_detector_sheet;
          clip_rect.x = ice_animation.frame * Map::c_tile_dimension_in_pixels;
          clip_rect.y = underneath.underneath_ice_detector.detected * Map::c_tile_dimension_in_pixels;
          break;
     case UnderneathInteractive::Type::hole:
          underneath_sheet = hole_sheet;
          clip_rect.x = underneath.underneath_hole.filled * Map::c_tile_dimension_in_pixels;
          break;
     case UnderneathInteractive::Type::destructable:
          underneath_sheet = destructable_sheet;
          clip_rect.x = underneath.underneath_destructable.destroyed * Map::c_tile_dimension_in_pixels;
          break;
     }

     ASSERT ( underneath_sheet );

     SDL_BlitSurface ( underneath_sheet, &clip_rect, back_buffer, dest_rect );
}

Void InteractivesDisplay::render_interactive ( SDL_Surface* back_buffer, Interactive& interactive,
                                               SDL_Rect* dest_rect )
{
     SDL_Rect clip_rect { 0, 0,
                          Map::c_tile_dimension_in_pixels, Map::c_tile_dimension_in_pixels };

     SDL_Surface* sheet = nullptr;

     switch ( interactive.type ) {
     default:
          ASSERT ( 0 );
          break;
     case Interactive::Type::none:
          return;
     case Interactive::Type::lever:
          sheet = lever_sheet;
          if ( interactive.interactive_lever.state == Lever::State::on ) {
               clip_rect.x = Map::c_tile_dimension_in_pixels * 2;
          } else if ( interactive.interactive_lever.state == Lever::State::changing_on ||
                      interactive.interactive_lever.state == Lever::State::changing_off ) {
               clip_rect.x = Map::c_tile_dimension_in_pixels;
          }
          break;
     case Interactive::Type::pushable_block:
          sheet = pushable_block_sheet;
          break;
     case Interactive::Type::torch:
          sheet = torch_sheet;
          break;
     case Interactive::Type::pushable_torch:
          sheet = pushable_torch_sheet;
          break;
     case Interactive::Type::exit:
          sheet = exit_sheet;
          clip_rect.x = interactive.interactive_exit.direction * Map::c_tile_dimension_in_pixels;

          switch ( interactive.interactive_exit.state ) {
          default:
               break;
          case Exit::State::closed:
               break;
          case Exit::State::open:
               clip_rect.y += 2 * Map::c_tile_dimension_in_pixels;
               break;
          case Exit::State::locked:
               clip_rect.y += 4 * Map::c_tile_dimension_in_pixels;
               break;
          case Exit::State::changing_to_closed:
          case Exit::State::changing_to_open:
               clip_rect.y += Map::c_tile_dimension_in_pixels;
               break;
          case Exit::State::changing_to_locked:
          case Exit::State::changing_to_unlocked:
               clip_rect.y += 3 * Map::c_tile_dimension_in_pixels;
               break;
          }

          break;
     case Interactive::Type::bombable_block:
          sheet = bombable_block_sheet;
          break;
     case Interactive::Type::turret:
          sheet = turret_sheet;
          clip_rect.x = interactive.interactive_turret.facing * Map::c_tile_dimension_in_pixels;
          break;
     case Interactive::Type::portal:
          sheet = portal_sheet;
          break;
     }

     ASSERT ( sheet );

     SDL_BlitSurface ( sheet, &clip_rect, back_buffer, dest_rect );

     switch ( interactive.type ) {
     default:
          break;
     case Interactive::Type::torch:
          if ( interactive.interactive_torch.element ) {
               Int32 torch_frame = animation.frame % c_torch_frame_count;
               Int32 torch_row = interactive.interactive_torch.element;

               torch_row -= 1;
               torch_row *= Map::c_tile_dimension_in_pixels;

               clip_rect.x = Map::c_tile_dimension_in_pixels * torch_frame;
               clip_rect.y = torch_row;

               SDL_BlitSurface ( torch_element_sheet, &clip_rect, back_buffer, dest_rect );
          }
          break;
     case Interactive::Type::pushable_torch:
          if ( interactive.interactive_pushable_torch.torch.element ) {
               // TODO: clearly the torch and pushabled torch have the same element display code
               Int32 torch_frame = animation.frame % c_torch_frame_count;
               Int32 torch_row = interactive.interactive_torch.element;

               torch_row -= 1;
               torch_row *= Map::c_tile_dimension_in_pixels;

               clip_rect.x = Map::c_tile_dimension_in_pixels * torch_frame;
               clip_rect.y = torch_row;
               SDL_BlitSurface ( torch_element_sheet, &clip_rect, back_buffer, dest_rect );
          }
          break;
     }
}

