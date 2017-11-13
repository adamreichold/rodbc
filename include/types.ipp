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

#include "types.hpp"

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

void from_int(const boost::multiprecision::cpp_int& int_val, char* const str_val, long& str_ind , const std::size_t str_len );

}

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
inline void String< Size >::clear()
{
    ind_ = -1;
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

template< std::size_t Size >
inline const char* String< Size >::begin() const
{
    return val_;
}

template< std::size_t Size >
inline const char* String< Size >::end() const
{
    return val_ + std::max( ind_, 0l );
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
inline std::ostream& operator<< ( std::ostream& stream, const String< Size >& string )
{
    if ( auto* const c_str = string.c_str() )
    {
        stream << c_str;
    }

    return stream;
}

template< std::size_t Size >
inline Number< Size >::Number( const boost::multiprecision::cpp_int& val )
{
    detail::from_int( val, val_.val_, val_.ind_, Size );
}

template< std::size_t Size >
inline Number< Size >& Number< Size >::operator= ( const boost::multiprecision::cpp_int& val )
{
    detail::from_int( val, val_.val_, val_.ind_, Size );

    return *this;
}

template< std::size_t Size >
inline bool Number< Size >::isNull() const
{
    return val_.isNull();
}

template< std::size_t Size >
inline void Number< Size >::clear()
{
    val_.clear();
}

template< std::size_t Size >
inline boost::multiprecision::cpp_int Number< Size >::value() const
{
    return boost::multiprecision::cpp_int{ val_.c_str() };
}

template< std::size_t Size >
inline std::ostream& operator<< ( std::ostream& stream, const Number< Size >& number )
{
    return stream << number.val_;
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
void Nullable< Type >::clear()
{
    ind_ = -1;
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
Type Nullable< Type >::value( const Type& defVal ) const
{
    return ind_ < 0 ? defVal : val_;
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

template< typename Type >
inline std::ostream& operator<< ( std::ostream& stream, const Nullable< Type >& nullable )
{
    if ( const auto* const value = nullable.value() )
    {
        stream << *value;
    }

    return stream;
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
