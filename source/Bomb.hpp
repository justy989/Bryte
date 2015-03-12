#ifndef BRYTE_BOMB_HPP
#define BRYTE_BOMB_HPP

#include "Entity.hpp"
#include "StopWatch.hpp"

namespace bryte {

     struct Bomb : public Entity {
     public:

          Void update ( float dt );
          Void clear ( );

     public:

          static const Real32 c_explode_time;
          static const Real32 c_explode_radius;

     public:

          Stopwatch explode_watch;
     };
}

#endif

