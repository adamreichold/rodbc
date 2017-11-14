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
#include "types.ipp"

#include <sql.h>
#include <sqlext.h>

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/conversion.hpp>
#include <boost/format.hpp>
#include <boost/functional/hash.hpp>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/spirit/home/karma/generate.hpp>
#include <boost/spirit/home/karma/numeric/int.hpp>
#include <boost/spirit/home/karma/numeric/uint.hpp>
#include <boost/spirit/home/qi/numeric/int.hpp>
#include <boost/spirit/home/qi/numeric/uint.hpp>
#include <boost/spirit/home/qi/parse.hpp>

namespace rodbc
{
namespace
{

template< typename Integer, typename Generator >
void fromInteger( const Integer int_val, char* const str_val, long& str_ind, const std::size_t str_len )
{
    boost::iostreams::array_sink sink{ str_val, str_len };
    boost::iostreams::stream< boost::iostreams::array_sink > stream{ sink };
    boost::spirit::ostream_iterator iterator{ stream };

    if ( boost::spirit::karma::generate( iterator, Generator{}, int_val ) )
    {
        str_ind = stream.tellp();
    }
    else
    {
        throw std::range_error{ str( boost::format{ "Value %d is too large for a number with %d digits." } % int_val % str_len ) };
    }
}

template< typename Integer, typename Parser >
Integer toInteger( const char* const begin, const char* const end )
{
    Integer result;

    if ( !boost::spirit::qi::parse( begin, end, Parser{} , result ) )
    {
        throw std::runtime_error{ "Number value not valid as an integer." };
    }

    return result;
}

Timestamp fromPTime( const boost::posix_time::ptime& ptime )
{
    Timestamp ts;

    const auto date = ptime.date();
    ts.year = date.year();
    ts.month = date.month();
    ts.day = date.day();

    const auto time_of_day = ptime.time_of_day();
    ts.hour = time_of_day.hours();
    ts.minute = time_of_day.minutes();
    ts.second = time_of_day.seconds();
    ts.fraction = time_of_day.fractional_seconds();

    return ts;
}

boost::posix_time::ptime toPTime( const Timestamp& ts )
{
    return {
        { static_cast< unsigned short >( ts.year ), ts.month, ts.day },
        { ts.hour, ts.minute, ts.second, ts.fraction }
    };
};

const boost::format& format( const Timestamp& ts )
{
    static thread_local boost::format rfc3339{
        "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ"
    };

    return rfc3339 % ts.year % ts.month % ts.day % ts.hour % ts.minute % ts.second % ts.fraction;
}

}

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

std::size_t hash( const char* const val, const long ind )
{
    return ind < 0 ? 0 : boost::hash_range( val, val + ind );
}

void fromInt( const boost::multiprecision::cpp_int& int_val, char* const str_val, long& str_ind, const std::size_t str_len )
{
    boost::iostreams::array_sink sink{ str_val, str_len };
    boost::iostreams::stream< boost::iostreams::array_sink > stream{ sink };

    if ( stream << int_val )
    {
        str_ind = stream.tellp();
    }
    else
    {
        throw std::range_error{ str( boost::format{ "Value %s is too large for a number with %d digits." } % int_val.str() % str_len ) };
    }
}

void fromInt64( const std::int64_t int_val, char* const str_val, long& str_ind, const std::size_t str_len )
{
    fromInteger< std::int64_t, boost::spirit::karma::int_generator< std::int64_t > >( int_val, str_val, str_ind, str_len );
}

void fromUInt64( const std::uint64_t int_val, char* const str_val, long& str_ind, const std::size_t str_len )
{
    fromInteger< std::uint64_t, boost::spirit::karma::uint_generator< std::uint64_t > >( int_val, str_val, str_ind, str_len );
}

std::int64_t toInt64( const char* const begin, const char* const end )
{
    return toInteger< std::int64_t, boost::spirit::qi::int_parser< std::int64_t > >( begin, end );
}

std::uint64_t toUInt64( const char* const begin, const char* const end )
{
    return toInteger< std::uint64_t, boost::spirit::qi::uint_parser< std::uint64_t > >( begin, end );
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

bool operator== ( const Timestamp& lhs, const Timestamp& rhs )
{
    return toPTime( lhs ) == toPTime( rhs );
}

bool operator!= ( const Timestamp& lhs, const Timestamp& rhs )
{
    return !( lhs == rhs );
}

std::ostream& operator<< ( std::ostream& stream, const Timestamp& ts )
{
    return stream << format( ts );
}

Timestamp fromTime_t( const std::time_t time )
{
    return fromPTime( boost::posix_time::from_time_t( time ) );
}

std::time_t toTime_t( const Timestamp& ts )
{
    return to_time_t( toPTime( ts ) );
}

std::string toString( const Timestamp& ts )
{
    return str( format( ts ) );
}

}
