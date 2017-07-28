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
    std::memcpy( dst, src.data(), src.size() );

    dst_ind = src.size();
}

bool compare( const char* const lhs, const long lhs_ind, const char* const rhs, const long rhs_ind )
{
    if ( lhs_ind < 0 || rhs_ind < 0 )
    {
        return false;
    }

    return std::memcmp( lhs, rhs, std::min( lhs_ind, rhs_ind ) ) == 0;
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
{
    std::array< char, 5 + 1 > state;
    std::array< char, 255 + 1 > message;

    const auto rc = ::SQLGetDiagRec( type, handle, 1, (SQLCHAR*) state.data(), nullptr, (SQLCHAR*) message.data(), message.max_size(), nullptr );

    what_.assign( SQL_SUCCEEDED( rc ) ? message.data() : "unknown ODBC error" );
}

const char* Exception::what() const noexcept
{
    return what_.c_str();
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


