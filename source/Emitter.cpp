#include "Emitter.hpp"
#include "Utils.hpp"
#include "Log.hpp"

using namespace bryte;

Void Emitter::setup_immortal ( const Vector& position, Uint32 color,
                               Real32 min_particle_angle, Real32 max_particle_angle,
                               Real32 min_particle_lifetime, Real32 max_particle_lifetime,
                               Real32 min_particle_speed, Real32 max_particle_speed,
                               Uint8 particles_per_frame, Uint8 frames_per_particle_batch )
{
     ASSERT ( min_particle_angle <= max_particle_angle );
     ASSERT ( min_particle_lifetime <= max_particle_lifetime );
     ASSERT ( min_particle_speed <= max_particle_speed );
     ASSERT ( particles_per_frame );

     this->life_state = Entity::LifeState::alive;

     life_type = LifeType::immortal;

     this->position = position;
     this->color = color;

     this->min_particle_angle = min_particle_angle;
     this->max_particle_angle = max_particle_angle;
     this->min_particle_lifetime = min_particle_lifetime;
     this->max_particle_lifetime = max_particle_lifetime;
     this->min_particle_speed = min_particle_speed;
     this->max_particle_speed = max_particle_speed;

     this->particles_per_frame = particles_per_frame;
     this->frames_per_particle_batch = frames_per_particle_batch;

     this->track_entity = track_entity;
}

Void Emitter::setup_limited_time ( const Vector& position, Real32 lifetime, Uint32 color,
                                   Real32 min_particle_angle, Real32 max_particle_angle,
                                   Real32 min_particle_lifetime, Real32 max_particle_lifetime,
                                   Real32 min_particle_speed, Real32 max_particle_speed,
                                   Uint8 particles_per_frame, Uint8 frames_per_particle_batch )
{
     ASSERT ( min_particle_angle <= max_particle_angle );
     ASSERT ( min_particle_lifetime <= max_particle_lifetime );
     ASSERT ( min_particle_speed <= max_particle_speed );
     ASSERT ( particles_per_frame );

     this->life_state = Entity::LifeState::alive;

     life_type = LifeType::stopwatch;

     this->position = position;
     this->color = color;

     this->min_particle_angle = min_particle_angle;
     this->max_particle_angle = max_particle_angle;
     this->min_particle_lifetime = min_particle_lifetime;
     this->max_particle_lifetime = max_particle_lifetime;
     this->min_particle_speed = min_particle_speed;
     this->max_particle_speed = max_particle_speed;

     this->particles_per_frame = particles_per_frame;
     this->frames_per_particle_batch = frames_per_particle_batch;

     this->track_entity = track_entity;

     lifetime_watch.reset ( lifetime );
}

Void Emitter::setup_to_track_entity ( Entity* entity, const Vector& entity_offset, Uint32 color,
                                      Real32 min_particle_angle, Real32 max_particle_angle,
                                      Real32 min_particle_lifetime, Real32 max_particle_lifetime,
                                      Real32 min_particle_speed, Real32 max_particle_speed,
                                      Uint8 particles_per_frame, Uint8 frames_per_particle_batch  )
{
     ASSERT ( entity );
     ASSERT ( min_particle_angle <= max_particle_angle );
     ASSERT ( min_particle_lifetime <= max_particle_lifetime );
     ASSERT ( min_particle_speed <= max_particle_speed );
     ASSERT ( particles_per_frame );

     life_type = LifeType::entity;

     this->color = color;

     this->min_particle_angle = min_particle_angle;
     this->max_particle_angle = max_particle_angle;
     this->min_particle_lifetime = min_particle_lifetime;
     this->max_particle_lifetime = max_particle_lifetime;
     this->min_particle_speed = min_particle_speed;
     this->max_particle_speed = max_particle_speed;

     this->particles_per_frame = particles_per_frame;
     this->frames_per_particle_batch = frames_per_particle_batch;

     this->track_entity.entity = entity;
     this->track_entity.offset = entity_offset;

     position = track_entity.entity->position + entity_offset;
}

Void Emitter::clear ( )
{
     for ( Int32 i = 0; i < c_max_particles; ++i ) {
          particle_lifetime_watches [ i ].reset ( 0.0f );
          particles [ i ].position.zero ( );
          particles [ i ].velocity.zero ( );
     }

     color = 0;

     min_particle_angle = 0.0f;
     max_particle_angle = 0.0f;
     min_particle_lifetime = 0.0f;
     max_particle_lifetime = 0.0f;
     min_particle_speed = 0.0f;
     max_particle_speed = 0.0f;

     particles_per_frame = 0;
     frames_per_particle_batch = 0;
     frames_since_last_batch = 255; // so we always start off spawning particles

     track_entity.entity = nullptr;
     track_entity.offset.zero ( );
}

Void Emitter::update ( float time_delta, Random& random )
{
     switch ( life_type ) {
     default:
          ASSERT ( 0 );
          break;
     case LifeType::immortal:
          break;
     case LifeType::entity:
          position = track_entity.entity->position + track_entity.offset;

          if ( track_entity.entity->is_dead ( ) ) {
               track_entity.entity = nullptr;
               track_entity.offset.zero ( );
               life_state = Entity::LifeState::dead;
               return;
          }
          break;
     case LifeType::stopwatch:
          lifetime_watch.tick ( time_delta );

          if ( lifetime_watch.expired ( ) ) {
               life_state = Entity::LifeState::dead;
          }
          break;
     }

     // update particles
     for ( Uint8 i = 0; i < c_max_particles; ++i ) {
          auto& lifetime_watch = particle_lifetime_watches [ i ];

          if ( lifetime_watch.expired ( ) ) {
               continue;
          }

          lifetime_watch.tick ( time_delta );

          auto& particle = particles [ i ];

          particle.position += ( particle.velocity * time_delta );
     }

     if ( frames_since_last_batch < frames_per_particle_batch ) {
          frames_since_last_batch++;
          return;
     }

     frames_since_last_batch = 0;

     for ( Uint8 i = 0; i < particles_per_frame; ++i ) {
          if ( !spawn_particle ( random ) ) {
               break;
          }
     }
}

static Real32 gen_real32_from_range ( Random& random, Real32 min, Real32 max )
{
     static const Int32 c_max_value = 10000;

     Int32 val = random.generate ( 0, c_max_value );

     Real32 real_val = static_cast<float>( val ) / static_cast<float> ( c_max_value );

     return min + ( ( max - min ) * real_val );
}

Bool Emitter::spawn_particle ( Random& random )
{
     if ( !particle_lifetime_watches [ next_free_particle ].expired ( ) ) {
          return false;
     }

     Real32 particle_angle = gen_real32_from_range ( random, min_particle_angle, max_particle_angle );
     Real32 particle_lifetime = gen_real32_from_range ( random, min_particle_lifetime, max_particle_lifetime );
     Real32 particle_speed = gen_real32_from_range ( random, min_particle_speed, max_particle_speed );

     Particle& particle = particles [ next_free_particle ];
     Stopwatch& lifewatch = particle_lifetime_watches [ next_free_particle ];

#if 0
     LOG_DEBUG ( "Spawn Particle P: %f, %f V: %f, %f L: %f\n",
                 particle.position.x ( ), particle.position.y ( ),
                 particle.velocity.x ( ), particle.velocity.x ( ),
                 particle_lifetime );
#endif

     particle.position = position;

     particle.velocity.set ( cos ( particle_angle ), sin ( particle_angle ) );
     particle.velocity *= particle_speed;

     lifewatch.reset ( particle_lifetime );

     next_free_particle++;
     next_free_particle %= c_max_particles;

     return true;
}

