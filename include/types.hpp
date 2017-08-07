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

template< std::size_t Size > std::ostream& operator<< ( std::ostream& stream, const String< Size >& string );

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

Timestamp from_time_t( const std::time_t time );
std::time_t to_time_t( const Timestamp& ts );
std::string to_string( const Timestamp& ts );

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

}
