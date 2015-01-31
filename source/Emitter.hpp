#ifndef BRYTE_EMITTER_HPP
#define BRYTE_EMITTER_HPP

#include "Entity.hpp"
#include "StopWatch.hpp"
#include "Random.hpp"

namespace bryte {

     struct Particle {
          Vector position;
          Vector velocity;
     };

     struct Emitter : public Entity {
     public:

          enum LifeType {
               immortal,
               stopwatch,
               entity,
          };

          Void setup_immortal ( const Vector& postion, Uint32 color,
                                Real32 min_particle_angle, Real32 max_particle_angle,
                                Real32 min_particle_lifetime, Real32 max_particle_lifetime,
                                Real32 min_particle_speed, Real32 max_particle_speed,
                                Uint8 particles_per_frame, Uint8 frames_per_particle_batch );

          Void setup_limited_time ( const Vector& postion, Real32 lifetime, Uint32 color,
                                    Real32 min_particle_angle, Real32 max_particle_angle,
                                    Real32 min_particle_lifetime, Real32 max_particle_lifetime,
                                    Real32 min_particle_speed, Real32 max_particle_speed,
                                    Uint8 particles_per_frame, Uint8 frames_per_particle_batch );

          Void setup_to_track_entity ( Entity* entity, const Vector& entity_offset, Uint32 color,
                                       Real32 min_particle_angle, Real32 max_particle_angle,
                                       Real32 min_particle_lifetime, Real32 max_particle_lifetime,
                                       Real32 min_particle_speed, Real32 max_particle_speed,
                                       Uint8 particles_per_frame, Uint8 frames_per_particle_batch  );

          Void clear ( );

          Void update ( float time_delta, Random& random );

     private:

          Bool spawn_particle ( Random& random );

     public:

          static const Uint8 c_max_particles = 32;

     public:

          Particle particles [ c_max_particles ];
          Stopwatch particle_lifetime_watches [ c_max_particles ];

          Uint32 color;

          Real32 min_particle_angle;
          Real32 max_particle_angle;
          Real32 min_particle_lifetime;
          Real32 max_particle_lifetime;
          Real32 min_particle_speed;
          Real32 max_particle_speed;

          Uint8 particles_per_frame;
          Uint8 frames_per_particle_batch;
          Uint8 frames_since_last_batch;
          Uint8 next_free_particle;

          LifeType life_type;

          // either tracking an enemy or have a limited lifetime
          union {
               TrackEntity track_entity;
               Stopwatch lifetime_watch;
          };
     };

};

#endif

