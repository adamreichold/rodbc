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

#include <boost/mpl/accumulate.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/multiprecision/cpp_int.hpp>

#include <ctime>
#include <functional>
#include <stdexcept>
#include <string>
#include <iosfwd>

namespace rodbc
{

/**
 * @brief The Exception class
 */
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

/**
 * @brief The String class template
 */
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
    void clear();

public:
    std::string str() const;
    const char* c_str() const;

    const char* begin() const;
    const char* end() const;

private:
    char val_[ Size + 1 ];
    long ind_;

    friend class Statement;
    template< std::size_t Size_ > friend class Number;
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

template< std::size_t Size > std::ostream& operator<< ( std::ostream& stream, const String< Size >& string );

/**
 * @brief The Number class template
 */
template< std::size_t Size >
class Number
{
public:
    Number() = default;

    Number( const Number& ) = default;
    Number& operator= ( const Number& ) = default;

    explicit Number( const boost::multiprecision::cpp_int& );
    Number& operator= ( const boost::multiprecision::cpp_int& );

    explicit Number( const std::int64_t );
    Number& operator= ( const std::int64_t );

    explicit Number( const std::uint64_t );
    Number& operator= ( const std::uint64_t );

    bool isNull() const;
    void clear();

public:
    boost::multiprecision::cpp_int value() const;

    std::int64_t toInt64() const;
    std::uint64_t toUInt64() const;

private:
    String< Size > val_;

    friend class Statement;
    template< std::size_t Size_ > friend std::ostream& operator<< ( std::ostream& stream, const Number< Size_ >& number );
    template< class Key > friend struct std::hash;
};

template< std::size_t Size > std::ostream& operator<< ( std::ostream& stream, const Number< Size >& number );

/**
 * @brief The Timestamp struct
 */
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

std::ostream& operator<< ( std::ostream& stream, const Timestamp& ts );

Timestamp fromTime_t( const std::time_t time );
std::time_t toTime_t( const Timestamp& ts );
std::string toString( const Timestamp& ts );

/**
 * @brief The Nullable class template
 */
template< typename Type >
class Nullable
{
public:
    Nullable();
    Nullable( const Type& val );

    Nullable( const Nullable& ) = default;
    Nullable& operator= ( const Nullable& ) = default;

    bool isNull() const;
    void clear();

public:
    Type* value();
    const Type* value() const;

    Type value( const Type& defVal ) const;

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

template< typename Type > std::ostream& operator<< ( std::ostream& stream, const Nullable< Type >& nullable );

template< std::size_t... Indices >
struct IndexSequence
{
};

namespace detail
{

struct PushBackIndex
{
    template< typename, typename >
    struct apply;

    template< std::size_t... Indices, typename Index >
    struct apply< IndexSequence< Indices... >, Index >
    {
        using type = IndexSequence< Indices..., Index::value >;
    };
};

}

template< std::size_t Size >
using MakeIndexSequence = typename boost::mpl::accumulate< boost::mpl::range_c< std::size_t, 0, Size >, IndexSequence<>, detail::PushBackIndex >::type;

}
