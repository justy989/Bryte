#ifndef BRYTE_DIRECTION_HPP
#define BRYTE_DIRECTION_HPP

namespace bryte {

     enum Direction {
          left = 0,
          up,
          right,
          down,
          count
     };

     inline Direction opposite_direction ( Direction dir )
     {
          switch ( dir ) {
          default:
               break;
          case Direction::left:
               return Direction::right;
          case Direction::right:
               return Direction::left;
          case Direction::up:
               return Direction::down;
          case Direction::down:
               return Direction::up;
          }

          return Direction::count;
     }
}

#endif

