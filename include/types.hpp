/*

Copyright 2017 Adam Reichold

This file is part of rodbc.

rodbc is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

rodbc is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with rodbc.  If not, see <http://www.gnu.org/licenses/>.

*/
#pragma once

#include <cstring>
#include <stdexcept>
#include <string>

namespace rodbc
{
namespace detail
{

void assign( char* const dst, long& dst_ind, const char* const src, const long src_ind );
void assign( char* const dst, long& dst_ind, const std::string& src );

bool compare( const char* const lhs, const long lhs_ind, const char* const rhs, const long rhs_ind );

std::string str( const char* const val, const long ind );
const char* c_str( char* const val, const long ind );

}

class Exception : public std::exception
{
public:
    Exception( const short type, void* const handle );

    const char* what() const noexcept override;

private:
    std::string what_;
};

template< std::size_t Size >
class String
{
public:
    String();

    String( const String& );
    String& operator= ( const String& );

    String( const std::string& );
    String& operator= ( const std::string& );

    bool isNull() const;

public:
    bool operator== ( const String& );
    bool operator!= ( const String& );

public:
    std::string str() const;
    const char* c_str() const;

private:
    char val_[ Size + 1 ];
    long ind_;

    friend class Statement;
};

struct Timestamp
{
   short year;
   unsigned short month;
   unsigned short day;
   unsigned short hour;
   unsigned short minute;
   unsigned short second;
   unsigned fraction;

   std::string str() const;
};

template< typename Type >
class Nullable
{
public:
    Nullable();
    Nullable( const Type& val );

    Nullable( const Nullable& ) = default;
    Nullable& operator= ( const Nullable& ) = default;

    bool isNull() const;

public:
    Type* value();
    const Type* value() const;

private:
    Type val_;
    long ind_;

    friend class Statement;
};

template< std::size_t Size >
inline String< Size >::String()
: ind_{ -1 }
{
}

template< std::size_t Size >
inline String< Size >::String( const String& that )
{
    detail::assign( val_, ind_, that.val_, that.ind_ );
}

template< std::size_t Size >
inline String< Size >& String< Size >::operator= ( const String& that )
{
    if ( this == &that )
    {
        return *this;
    }

    detail::assign( val_, ind_, that.val_, that.ind_ );

    return *this;
}

template< std::size_t Size >
inline String< Size >::String( const std::string& str )
{
    detail::assign( val_, ind_, str );
}

template< std::size_t Size >
inline String< Size >& String< Size >::operator= ( const std::string& str )
{
    detail::assign( val_, ind_, str );
    return *this;
}

template< std::size_t Size >
inline bool String< Size >::isNull() const
{
    return ind_ < 0;
}

template< std::size_t Size >
bool String< Size >::operator== ( const String& that )
{
    return detail::compare( val_, ind_, that.val_, that.ind_ );
}

template< std::size_t Size >
bool String< Size >::operator!= ( const String& that )
{
    return !( *this == that );
}

template< std::size_t Size >
inline std::string String< Size >::str() const
{
    return detail::str( val_, ind_ );
}

template< std::size_t Size >
inline const char* String< Size >::c_str() const
{
    return detail::c_str( const_cast< char* >( val_ ), ind_ );
}

template< typename Type >
inline Nullable< Type >::Nullable()
: val_{}
, ind_{ -1 }
{
}

template< typename Type >
inline Nullable< Type >::Nullable( const Type& val )
: val_{ val }
, ind_{ sizeof( Type ) }
{
}

template< typename Type >
inline bool Nullable< Type >::isNull() const
{
    return ind_ < 0;
}

template< typename Type >
inline Type* Nullable< Type >::value()
{
    return ind_ < 0 ? nullptr : &val_;
}

template< typename Type >
inline const Type* Nullable< Type >::value() const
{
    return ind_ < 0 ? nullptr : &val_;
}

}
