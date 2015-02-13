#ifndef BRYTE_PROJECTILE_HPP
#define BRYTE_PROJECTILE_HPP

#include "Character.hpp"
#include "StopWatch.hpp"

namespace bryte
{
     class Map;
     class Interactives;

     struct Projectile : public Entity {
     public:

          enum Type {
               arrow,
               goo
          };

     public:

          Void hit_character ( Character& character );
          Void update ( float dt, const Map& map, Interactives& interactives );
          Bool check_for_solids ( const Map& map, Interactives& interactives );
          Void clear ( );

     private:

          Void update_arrow ( float dt, const Map& map, Interactives& interactives );
          Void update_goo ( float dt, const Map& map, Interactives& interactives );

     public:

          static Vector collision_points [ Direction::count ];

          static const Real32 c_arrow_speed;
          static const Real32 c_goo_speed;
          static const Real32 c_stuck_time;

     public:

          Type type;
          Direction facing;
          Stopwatch stuck_watch;
          TrackEntity track_entity;
          Bool on_fire;
     };
}

#endif

