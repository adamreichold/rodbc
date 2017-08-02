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
#include "statement.hpp"

#include "connection.hpp"

#include <sql.h>
#include <sqlext.h>

namespace rodbc
{
namespace
{

inline void check( const SQLRETURN rc, const SQLSMALLINT type, const SQLHANDLE handle )
{
    if ( !SQL_SUCCEEDED( rc ) )
    {
        throw Exception{ type, handle };
    }
}

template< typename Type >
struct OdbcTraits;

#define DEF_ODBC_TRAITS( type, odbcType, cType, sqlType ) \
template<> \
struct OdbcTraits< type > \
{ \
    using OdbcType = odbcType; \
    static constexpr auto CType = cType; \
    static constexpr auto SqlType = sqlType; \
\
    static_assert( sizeof ( type ) == sizeof ( OdbcType ), "Size of mapped type and ODBC typedef must match." ); \
};

DEF_ODBC_TRAITS( std::int8_t, SCHAR, SQL_C_STINYINT, SQL_TINYINT )
DEF_ODBC_TRAITS( std::int16_t, SQLSMALLINT, SQL_C_SSHORT, SQL_SMALLINT )
DEF_ODBC_TRAITS( std::int32_t, SQLINTEGER, SQL_C_SLONG, SQL_INTEGER )
DEF_ODBC_TRAITS( std::int64_t, SQLBIGINT, SQL_C_SBIGINT, SQL_BIGINT )

DEF_ODBC_TRAITS( std::uint8_t, UCHAR, SQL_C_UTINYINT, SQL_TINYINT )
DEF_ODBC_TRAITS( std::uint16_t, SQLUSMALLINT, SQL_C_USHORT, SQL_SMALLINT )
DEF_ODBC_TRAITS( std::uint32_t, SQLUINTEGER, SQL_C_ULONG, SQL_INTEGER )
DEF_ODBC_TRAITS( std::uint64_t, SQLUBIGINT, SQL_C_UBIGINT, SQL_BIGINT )

DEF_ODBC_TRAITS( float, SQLREAL, SQL_C_FLOAT, SQL_FLOAT )
DEF_ODBC_TRAITS( double, SQLFLOAT, SQL_C_DOUBLE, SQL_DOUBLE )

DEF_ODBC_TRAITS( bool, SQLCHAR, SQL_C_BIT, SQL_TINYINT )

DEF_ODBC_TRAITS( Timestamp, SQL_TIMESTAMP_STRUCT, SQL_C_TIMESTAMP, SQL_TIMESTAMP );

#undef DEF_ODBC_TRAITS

}

Statement::Statement( Connection& conn, const char* const stmt )
{
   check( ::SQLAllocHandle( SQL_HANDLE_STMT, conn.dbc_, &stmt_ ), SQL_HANDLE_DBC, conn.dbc_ );
   check( ::SQLPrepare( stmt_, (SQLCHAR*) stmt, SQL_NTS ), SQL_HANDLE_STMT, stmt_ );
}

Statement::~Statement()
{
    ::SQLFreeHandle( SQL_HANDLE_STMT, stmt_ );
}

#define DEF_BIND_PARAM( type ) \
Statement& Statement::bindParam( const type& param ) \
{ \
    return doBindParam( &param ); \
} \
\
Statement& Statement::bindParam( const Nullable< type >& param ) \
{ \
    return doBindParam( &param.val_, &param.ind_ ); \
}

DEF_BIND_PARAM( std::int8_t )
DEF_BIND_PARAM( std::int16_t )
DEF_BIND_PARAM( std::int32_t )
DEF_BIND_PARAM( std::int64_t )

DEF_BIND_PARAM( std::uint8_t )
DEF_BIND_PARAM( std::uint16_t )
DEF_BIND_PARAM( std::uint32_t )
DEF_BIND_PARAM( std::uint64_t )

DEF_BIND_PARAM( float )
DEF_BIND_PARAM( double )

DEF_BIND_PARAM( bool )

DEF_BIND_PARAM( Timestamp )

#undef DEF_BIND_PARAM

Statement& Statement::rebindParams()
{
    param_ = 0;

    return *this;
}

#define DEF_BIND_COL( type ) \
Statement& Statement::bindCol( type& col ) \
{ \
    return doBindCol( &col ); \
} \
\
Statement& Statement::bindCol( Nullable< type >& col ) \
{ \
    return doBindCol( &col.val_, &col.ind_ ); \
}

DEF_BIND_COL( std::int8_t )
DEF_BIND_COL( std::int16_t )
DEF_BIND_COL( std::int32_t )
DEF_BIND_COL( std::int64_t )

DEF_BIND_COL( std::uint8_t )
DEF_BIND_COL( std::uint16_t )
DEF_BIND_COL( std::uint32_t )
DEF_BIND_COL( std::uint64_t )

DEF_BIND_COL( float )
DEF_BIND_COL( double )

DEF_BIND_COL( bool )

DEF_BIND_COL( Timestamp )

#undef DEF_BIND_COL

Statement& Statement::rebindCols()
{
    col_ = 0;

    return *this;
}

void Statement::exec()
{
    ::SQLCloseCursor( stmt_ );
    check( ::SQLExecute( stmt_ ), SQL_HANDLE_STMT, stmt_ );
}

bool Statement::fetch()
{
    return SQL_SUCCEEDED( ::SQLFetch( stmt_ ) );
}

Statement& Statement::doBindStringParam( const char* const data, const std::size_t length , const long* const indicator )
{
    return doBindParam( data, SQL_C_CHAR, SQL_VARCHAR, sizeof ( char ), length, indicator );
}

Statement& Statement::doBindStringCol( char* const data, const std::size_t length, long* const indicator )
{
    return doBindCol( data, SQL_C_CHAR, sizeof ( char ) * length, indicator );
}

void Statement::doBindParamArray( const std::size_t size, const std::size_t count )
{
    check( ::SQLSetStmtAttr( stmt_, SQL_ATTR_PARAM_BIND_TYPE, (SQLPOINTER) size, 0 ), SQL_HANDLE_STMT, stmt_ );
    check( ::SQLSetStmtAttr( stmt_, SQL_ATTR_PARAMSET_SIZE, (SQLPOINTER) count, 0 ), SQL_HANDLE_STMT, stmt_ );
}

void Statement::doBindColArray( const std::size_t size, const std::size_t count, long* const rowsFetched )
{
    check( ::SQLSetStmtAttr( stmt_, SQL_ATTR_ROW_BIND_TYPE, (SQLPOINTER) size, 0 ), SQL_HANDLE_STMT, stmt_ );
    check( ::SQLSetStmtAttr( stmt_, SQL_ATTR_ROW_ARRAY_SIZE, (SQLPOINTER) count, 0 ), SQL_HANDLE_STMT, stmt_ );
    check( ::SQLSetStmtAttr( stmt_, SQL_ATTR_ROWS_FETCHED_PTR, rowsFetched, 0 ), SQL_HANDLE_STMT, stmt_ );
}

template< typename Param >
Statement& Statement::doBindParam( const Param* const data, const long* const indicator )
{
    using ParamTraits = OdbcTraits< Param >;

    return doBindParam( data, ParamTraits::CType, ParamTraits::SqlType, sizeof ( Param ), 0, indicator );
}

template< typename Col >
Statement& Statement::doBindCol( Col* const data, long* const indicator )
{
    using ColTraits = OdbcTraits< Col >;

    return doBindCol( data, ColTraits::CType, sizeof ( Col ), indicator );

}

Statement& Statement::doBindParam( const void* const data, const short cType, const short sqlType, const std::size_t size, const std::size_t length, const long* const indicator )
{
    check( ::SQLBindParameter(
        stmt_,
        ++param_,
        SQL_PARAM_INPUT,
        cType,
        sqlType,
        length,
        0,
        (SQLPOINTER) data,
        size,
        (SQLLEN*) indicator
    ), SQL_HANDLE_STMT, stmt_ );

    return *this;
}

Statement& Statement::doBindCol( void* const data, const short cType, const std::size_t size, long* const indicator )
{
    check( ::SQLBindCol(
        stmt_,
        ++col_,
        cType,
        data,
        size,
        indicator
    ), SQL_HANDLE_STMT, stmt_ );

    return *this;
}

}
