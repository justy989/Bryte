#ifndef BRYTE_DAMAGE_NUMBER_HPP
#define BRYTE_DAMAGE_NUMBER_HPP

#include "Entity.hpp"
#include "StopWatch.hpp"

namespace bryte
{
     struct DamageNumber : public Entity {
     public:

          Void update ( float time_delta );
          Void clear ( );

     public:

          static const Real32 c_rise_height;
          static const Real32 c_rise_speed;

     public:

          Stopwatch life_watch;
          Int32 value;
          Real32 starting_y;
     };
}

#endif

