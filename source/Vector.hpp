#ifndef BRYTE_VECTOR_HPP
#define BRYTE_VECTOR_HPP

#include "Types.hpp"

class Vector
{
public:

     inline Vector ( Real32 x = 0.0f, Real32 y = 0.0f );

     inline Vector operator+ ( const Vector& v );
     inline Vector operator- ( const Vector& v );
     inline Vector operator* ( Real32 scale );

     inline Vector& operator+= ( const Vector& v );
     inline Vector& operator-= ( const Vector& v );
     inline Vector& operator*= ( Real32 scale );

     inline Real32 inner_product ( const Vector& v );

     inline void zero ( );

     inline void set ( Real32 x, Real32 y );
     inline void set_x ( Real32 x );
     inline void set_y ( Real32 y );

     inline Real32 x ( ) const;
     inline Real32 y ( ) const;

private:

     Real32 m_x;
     Real32 m_y;
};

inline Vector::Vector ( Real32 x, Real32 y ) :
     m_x { x },
     m_y { y }
{

}

inline Vector Vector::operator+ ( const Vector& v )
{
     return Vector ( m_x + v.m_x, m_y + v.m_y );
}

inline Vector Vector::operator- ( const Vector& v )
{
     return Vector ( m_x - v.m_x, m_y - v.m_y );
}

inline Vector Vector::operator* ( Real32 scale )
{
     return Vector ( m_x * scale, m_y * scale );
}

inline Vector& Vector::operator+= ( const Vector& v )
{
     m_x += v.m_x;
     m_y += v.m_y;

     return *this;
}

inline Vector& Vector::operator-= ( const Vector& v )
{
     m_x -= v.m_x;
     m_y -= v.m_y;

     return *this;
}

inline Vector& Vector::operator*= ( Real32 scale )
{
     m_x *= scale;
     m_y *= scale;

     return *this;
}

inline Real32 Vector::inner_product ( const Vector& v )
{
     return m_x * v.m_x + m_y * v.m_y;
}

inline void Vector::zero ( )
{
     m_x = 0.0f;
     m_y = 0.0f;
}

inline void Vector::set ( Real32 x, Real32 y )
{
     m_x = x;
     m_y = y;
}

inline void Vector::set_x ( Real32 x )
{
     m_x = x;
}

inline void Vector::set_y ( Real32 y )
{
     m_y = y;
}

Real32 Vector::x ( ) const
{
     return m_x;
}

Real32 Vector::y ( ) const
{
     return m_y;
}

#endif

