#ifndef BRYTE_RECT_HPP
#define BRYTE_RECT_HPP

#include "Vector.hpp"
#include "Utils.hpp"

namespace bryte
{
     struct Rect {

          inline Void set ( Int32 left, Int32 bottom, Int32 right, Int32 top );
          inline Void set ( Real32 left, Real32 bottom, Real32 right, Real32 top );
          inline Void set ( const Vector& min, const Vector& max );
          inline Void set_from_center ( const Vector& center, const Vector& dimensions );

          inline Real32 left ( ) const;
          inline Real32 top ( ) const;
          inline Real32 right ( ) const;
          inline Real32 bottom ( ) const;

          inline Real32 width ( ) const;
          inline Real32 height ( ) const;

          inline Real32 center_x ( ) const;
          inline Real32 center_y ( ) const;

          inline Vector center ( ) const;

          Vector min;
          Vector max;
     };

     inline Void Rect::set ( Int32 left, Int32 bottom, Int32 right, Int32 top )
     {
          ASSERT ( left <= right );
          ASSERT ( bottom <= top );

          min.set_x ( pixels_to_meters ( left ) );
          max.set_x ( pixels_to_meters ( right ) );
          min.set_y ( pixels_to_meters ( bottom ) );
          max.set_y ( pixels_to_meters ( top ) );
     }

     inline Void Rect::set ( Real32 left, Real32 bottom, Real32 right, Real32 top )
     {
          ASSERT ( left <= right );
          ASSERT ( bottom <= top );

          min.set_x ( left );
          max.set_x ( right );
          min.set_y ( bottom );
          max.set_y ( top );
     }

     inline Void Rect::set ( const Vector& min, const Vector& max )
     {
          ASSERT ( min.x ( ) <= max.x ( ) );
          ASSERT ( min.y ( ) <= max.y ( ) );

          this->min = min;
          this->max = max;
     }

     inline Void Rect::set_from_center ( const Vector& center, const Vector& dimensions )
     {
          ASSERT ( dimensions.x ( ) >= 0.0f );
          ASSERT ( dimensions.y ( ) >= 0.0f );

          Vector half_dimensions = dimensions * 0.5f;

          this->min =  center - half_dimensions;
          this->min =  center + half_dimensions;
     }

     inline Real32 Rect::width ( ) const
     {
          return max.x ( ) - min.x ( );
     }

     inline Real32 Rect::height ( ) const
     {
          return max.y ( ) - min.y ( );
     }

     inline Real32 Rect::left ( ) const { return min.x ( ); }
     inline Real32 Rect::top ( ) const { return max.x ( ); }
     inline Real32 Rect::right ( ) const { return max.x ( ); }
     inline Real32 Rect::bottom ( ) const { return min.y ( ); }

     inline Real32 Rect::center_x ( ) const { return min.x ( ) + width ( ) * 0.5f; }
     inline Real32 Rect::center_y ( ) const { return min.y ( ) + height ( ) * 0.5f; }

     inline Vector Rect::center ( ) const { return min + Vector ( width ( ), height ( ) ) * 0.5f; }
}

#endif

