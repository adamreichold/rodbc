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

#include <ctime>
#include <functional>
#include <stdexcept>
#include <string>

namespace rodbc
{
namespace detail
{

void assign( char* const dst, long& dst_ind, const char* const src, const long src_ind );
void assign( char* const dst, long& dst_ind, const std::string& src );
void assign( char* const dst, long& dst_ind, const char* const src );

int compare( const char* const lhs, const long lhs_ind, const char* const rhs, const long rhs_ind );

std::string str( const char* const val, const long ind );
const char* c_str( char* const val, const long ind );

std::size_t hash( const char* const val, const long ind );

}

class Exception : public std::exception
{
public:
    Exception( const short type, void* const handle );

    const char* state() const noexcept;
    int nativeError() const noexcept;

    bool isTimeout() const noexcept;
    bool isConstraintViolation() const noexcept;

    const char* what() const noexcept override;

private:
    char state_[ 5 + 1 ];
    int nativeError_;
    std::string message_;
};

template< std::size_t Size >
class String
{
public:
    String();

    String( const String& );
    String& operator= ( const String& );

    explicit String( const std::string& );
    String& operator= ( const std::string& );

    explicit String( const char* const );
    String& operator= ( const char* const );

    bool isNull() const;

public:
    std::string str() const;
    const char* c_str() const;

private:
    char val_[ Size + 1 ];
    long ind_;

    friend class Statement;
    template< std::size_t Size_ > friend bool operator== ( const String< Size_ >& lhs, const String< Size_ >& rhs );
    template< std::size_t Size_ > friend bool operator!= ( const String< Size_ >& lhs, const String< Size_ >& rhs );
    template< std::size_t Size_ > friend bool operator< ( const String< Size_ >& lhs, const String< Size_ >& rhs );
    template< std::size_t Size_ > friend bool operator<= ( const String< Size_ >& lhs, const String< Size_ >& rhs );
    template< std::size_t Size_ > friend bool operator> ( const String< Size_ >& lhs, const String< Size_ >& rhs );
    template< std::size_t Size_ > friend bool operator>= ( const String< Size_ >& lhs, const String< Size_ >& rhs );
    template< class Key > friend struct std::hash;
};

template< std::size_t Size > bool operator== ( const String< Size >& lhs, const String< Size >& rhs );
template< std::size_t Size > bool operator!= ( const String< Size >& lhs, const String< Size >& rhs );
template< std::size_t Size > bool operator< ( const String< Size >& lhs, const String< Size >& rhs );
template< std::size_t Size > bool operator<= ( const String< Size >& lhs, const String< Size >& rhs );
template< std::size_t Size > bool operator> ( const String< Size >& lhs, const String< Size >& rhs );
template< std::size_t Size > bool operator>= ( const String< Size >& lhs, const String< Size >& rhs );

struct Timestamp
{
   short year;
   unsigned short month;
   unsigned short day;
   unsigned short hour;
   unsigned short minute;
   unsigned short second;
   unsigned fraction;
};

bool operator== ( const Timestamp& lhs, const Timestamp& rhs );
bool operator!= ( const Timestamp& lhs, const Timestamp& rhs );

Timestamp from_time_t( const std::time_t time );
std::time_t to_time_t( const Timestamp& ts );
std::string to_string( const Timestamp& ts );

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
    template< typename Type_ > friend bool operator== ( const Nullable< Type_ >& lhs, const Nullable< Type_ >& rhs );
    template< typename Type_ > friend bool operator!= ( const Nullable< Type_ >& lhs, const Nullable< Type_ >& rhs );
    template< class Key > friend struct std::hash;
};

template< typename Type > bool operator== ( const Nullable< Type >& lhs, const Nullable< Type >& rhs );
template< typename Type > bool operator!= ( const Nullable< Type >& lhs, const Nullable< Type >& rhs );

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
inline String< Size >::String( const char* const str )
{
    detail::assign( val_, ind_, str );
}

template< std::size_t Size >
inline String< Size >& String< Size >::operator= ( const char* const str )
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
inline bool operator== ( const String< Size >& lhs, const String< Size >& rhs )
{
    return detail::compare( lhs.val_, lhs.ind_, rhs.val_, rhs.ind_ ) == 0;
}

template< std::size_t Size >
inline bool operator!= ( const String< Size >& lhs, const String< Size >& rhs )
{
    return !( lhs == rhs );
}

template< std::size_t Size >
inline bool operator< ( const String< Size >& lhs, const String< Size >& rhs )
{
    return detail::compare( lhs.val_, lhs.ind_, rhs.val_, rhs.ind_ ) < 0;
}

template< std::size_t Size >
inline bool operator<= ( const String< Size >& lhs, const String< Size >& rhs )
{
    return detail::compare( lhs.val_, lhs.ind_, rhs.val_, rhs.ind_ ) <= 0;
}

template< std::size_t Size >
inline bool operator> ( const String< Size >& lhs, const String< Size >& rhs )
{
    return rhs < lhs;
}

template< std::size_t Size >
inline bool operator>= ( const String< Size >& lhs, const String< Size >& rhs )
{
    return rhs <= lhs;
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

inline bool operator!= ( const Timestamp& lhs, const Timestamp& rhs )
{
    return !( lhs == rhs );
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

template< typename Type >
inline bool operator== ( const Nullable< Type >& lhs, const Nullable< Type >& rhs )
{
    if ( lhs.ind_ < 0 || rhs.ind_ < 0 )
    {
        return false;
    }

    return lhs.val_ == rhs.val_;
}

template< typename Type >
inline bool operator!= ( const Nullable< Type >& lhs, const Nullable< Type >& rhs )
{
    return !( lhs == rhs );
}

}

namespace std
{

template< std::size_t Size >
struct hash< rodbc::String< Size > >
{
    using argument_type = rodbc::String< Size >;
    using result_type = std::size_t;

    result_type operator() ( const argument_type& val ) const
    {
        return rodbc::detail::hash( val.val_, val.ind_ );
    }
};

template< typename Type >
struct hash< rodbc::Nullable< Type > >
{
    using argument_type = rodbc::Nullable< Type >;
    using result_type = std::size_t;

    result_type operator() ( const argument_type& val ) const
    {
        return val.ind_ < 0 ? 0 : std::hash< Type >{}( val.val_ );
    }
};

}
