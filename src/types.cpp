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
#include "types.hpp"

#include <sql.h>
#include <sqlext.h>

#include <boost/format.hpp>

namespace rodbc
{
namespace detail
{

void assign( char* const dst, long& dst_ind, const char* const src, const long src_ind )
{
    if ( src_ind > 0 )
    {
        std::memcpy( dst, src, src_ind );
    }

    dst_ind = src_ind;
}

void assign( char* const dst, long& dst_ind, const std::string& src )
{
    dst_ind = src.size();

    std::memcpy( dst, src.data(), dst_ind );
}


void assign( char* const dst, long& dst_ind, const char* const src )
{
    dst_ind = std::strlen( src );

    std::memcpy( dst, src, dst_ind );
}

int compare( const char* const lhs, const long lhs_ind, const char* const rhs, const long rhs_ind )
{
    if ( lhs_ind < 0 )
    {
        return -1;
    }

    if ( rhs_ind < 0 )
    {
        return +1;
    }

    return std::memcmp( lhs, rhs, std::min( lhs_ind, rhs_ind ) );
}

std::string str( const char* const val, const long ind )
{
    if( ind < 0 )
    {
        return {};
    }

    return { val, static_cast< std::size_t >( ind ) };
}

const char* c_str( char* const val, const long ind )
{
    if ( ind < 0 )
    {
        return nullptr;
    }

    val[ ind ] = '\0';
    return val;
}

}

Exception::Exception( const short type, void* const handle )
: state_{ "00000" }
, nativeError_{ 0 }
{
    SQLSMALLINT messageLength;

    if ( SQL_SUCCEEDED( ::SQLGetDiagRec( type, handle, 1, (SQLCHAR*) state_, &nativeError_, nullptr, 0, &messageLength ) ) )
    {
        message_.resize( messageLength + 1 );

        if ( SQL_SUCCEEDED( ::SQLGetDiagRec( type, handle, 1, nullptr, nullptr, (SQLCHAR*) &message_.front(), message_.size(), nullptr ) ) )
        {
            return;
        }
    }

    message_ = "ODBC diagnostic record could not be retrieved.";
}

const char* Exception::state() const noexcept
{
    return state_;
}

int Exception::nativeError() const noexcept
{
    return nativeError_;
}

bool Exception::isTimeout() const noexcept
{
    return std::strcmp( state_, "HYT00" ) == 0;
}

bool Exception::isConstraintViolation() const noexcept
{
    const auto odbcIntegrityConstraintViolation = std::strncmp( state_, "23", 2 ) == 0;
    const auto sqliteConstraint = std::strcmp( state_, "HY000" ) == 0 && nativeError_ == 19;

    return odbcIntegrityConstraintViolation || sqliteConstraint;
}

const char* Exception::what() const noexcept
{
    return message_.c_str();
}

std::string Timestamp::str() const
{
    static thread_local auto format = boost::format(
            "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ"
    );

    return boost::str(
        format % year % month % day % hour % minute % second % fraction
    );
}

}


