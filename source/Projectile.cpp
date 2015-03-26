#include "Projectile.hpp"
#include "Map.hpp"
#include "Interactives.hpp"
#include "Utils.hpp"

using namespace bryte;

const Real32 Projectile::c_arrow_speed = 20.0f;
const Real32 Projectile::c_goo_speed = 5.0f;
const Real32 Projectile::c_stuck_time = 1.5f;

Vector Projectile::collision_points [ Direction::count ];

Int32 Projectile::hit_character ( Character& character )
{
     Int32 damage_amount = 1;

     character.damage ( damage_amount, facing );
     character.effect_with_element ( effected_by_element );

     switch ( type ) {
     default:
          break;
     case Type::arrow:
          track_entity.entity = &character;
          track_entity.offset = position - character.position;
          stuck_watch.reset ( Projectile::c_stuck_time );
          break;
     case Type::goo:
          life_state = LifeState::dead;
          clear ( );
          break;
     }

     return damage_amount;
}

Bool Projectile::check_for_solids ( const Map& map, Interactives& interactives )
{
     // Note: Use the center of the arrow to check for tile collision, this
     //       makes lighting arrows on fire against a torch more user friendly
     Vector arrow_center = position + Vector { Map::c_tile_dimension_in_meters * 0.5f,
                                               Map::c_tile_dimension_in_meters * 0.5f };
     Map::Coordinates tile = Map::vector_to_coordinates ( arrow_center );

     if ( !map.coordinates_valid ( tile ) ) {
          life_state = Entity::LifeState::dead;
          return false;
     }

     Int32 tile_index = map.coordinate_to_tile_index ( tile.x, tile.y );
     Int32 prev_tile_x = map.tile_index_to_coordinate_x ( current_tile );
     Int32 prev_tile_y = map.tile_index_to_coordinate_y ( current_tile );

     if ( tile_index == current_tile ) {
          return false;
     } else {
          current_tile = tile_index;
     }

     Auto save_position = position;

     interactives.projectile_leave ( prev_tile_x, prev_tile_y, *this );
     interactives.projectile_enter ( tile.x, tile.y, *this );

     // recalculate tile index in case we have teleported
     if ( save_position != position ) {
          arrow_center = position + Vector { Map::c_tile_dimension_in_meters * 0.5f,
                                             Map::c_tile_dimension_in_meters * 0.5f };
          tile = Map::vector_to_coordinates ( arrow_center );
          tile_index = map.coordinate_to_tile_index ( tile.x, tile.y );

          current_tile = tile_index;
     }

     Auto& interactive = interactives.get_from_tile ( tile.x, tile.y );
     if ( interactive.type == Interactive::Type::exit ) {
          // otherwise arrows can escape when doors are open
          // TODO: is this ok?
          return true;
     }

     if ( !interactives.is_flyable ( tile.x, tile.y ) ) {
          return true;
     }

     if ( map.get_coordinate_solid ( tile.x, tile.y ) ) {
          return true;
     }

     return false;
}

Void Projectile::update ( float time_delta, const Map& map, Interactives& interactives  )
{
     switch ( type ) {
     default:
          ASSERT ( 0 );
          break;
     case Type::arrow:
          update_arrow ( time_delta, map, interactives );
          break;
     case Type::goo:
          update_goo ( time_delta, map, interactives );
          break;
     }
}

Void Projectile::update_arrow ( float time_delta, const Map& map, Interactives& interactives )
{
     switch ( life_state ) {
     default:
          ASSERT ( 0 );
          break;
     case LifeState::dead:
          break;
     case LifeState::spawning:
          // nop for now ** pre-mature planning wooo **
          life_state = LifeState::alive;
          break;
     case LifeState::alive:
          if ( stuck_watch.expired ( ) ) {
               position += vector_from_direction ( facing ) * c_arrow_speed * time_delta;

               if ( check_for_solids ( map, interactives ) ) {
                    stuck_watch.reset ( c_stuck_time );
               }
          } else {
               stuck_watch.tick ( time_delta );

               if ( track_entity.entity ) {
                    position = track_entity.entity->position + track_entity.offset;
               }

               if ( stuck_watch.expired ( ) ) {
                    life_state = dead;
                    clear ( );
               }
          }
          break;
     }
}

Void Projectile::update_goo ( float time_delta, const Map& map, Interactives& interactives )
{
     switch ( life_state ) {
     default:
          ASSERT ( 0 );
          break;
     case LifeState::dead:
          break;
     case LifeState::spawning:
          // nop for now ** pre-mature planning wooo **
          life_state = LifeState::alive;
          break;
     case LifeState::alive:
          position += vector_from_direction ( facing ) * c_goo_speed * time_delta;

          if ( check_for_solids ( map, interactives ) ) {
               stuck_watch.reset ( c_stuck_time );
               life_state = dead;
               clear ( );
          }
          break;
     }
}

Void Projectile::clear ( )
{
     type = Type::arrow;
     facing = Direction::left;
     stuck_watch.reset ( 0.0f );
     track_entity.entity = nullptr;
     track_entity.offset.zero ( );
     position.zero ( );
     current_tile = 0;
}

