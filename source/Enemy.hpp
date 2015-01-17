#ifndef BRYTE_ENEMY_HPP
#define BRYTE_ENEMY_HPP

#include "Character.hpp"
#include "Random.hpp"

namespace bryte
{
     struct Enemy : public Character {
     public:

          enum Type {
               none,
               rat,
          };

          struct RatState {
               bool      moving; // either moving or waiting
               Stopwatch timer;
          };

     public:

          Void think ( const Vector& player, Random& random, float time_delta );

     private:

          Void rat_think ( const Vector& player, Random& random, float time_delta );

     public:

          Type type;

          union {
               RatState rat_state;
          };
     };
}

#endif

