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
#include "connection.hpp"

#include "types.hpp"

#include <sql.h>
#include <sqlext.h>

#include <boost/algorithm/string/predicate.hpp>

namespace rodbc
{
namespace
{

inline void check( const SQLRETURN rc, const SQLSMALLINT type, const SQLHANDLE handle )
{
    if ( !SQL_SUCCEEDED( rc ) && rc != SQL_NO_DATA )
    {
        throw Exception{ type, handle };
    }
}

}

Environment::Environment()
{
    check( ::SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env_ ), SQL_HANDLE_ENV, nullptr );
    check( ::SQLSetEnvAttr( env_, SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC3, 0), SQL_HANDLE_ENV, env_ );
    check( ::SQLSetEnvAttr( env_, SQL_ATTR_CONNECTION_POOLING, (SQLPOINTER) SQL_CP_OFF, 0), SQL_HANDLE_ENV, env_ );
}

Environment::~Environment()
{
    if ( env_ )
    {
        ::SQLFreeHandle( SQL_HANDLE_ENV, env_ );

        env_ = nullptr;
    }
}

Environment::Environment( Environment&& that ) noexcept
{
    env_ = that.env_;
    that.env_ = nullptr;
}

Environment& Environment::operator= ( Environment&& that ) noexcept
{
    std::swap( env_, that.env_ );

    return *this;
}

Connection::Connection( Environment& env, const char* const connStr )
{
    check( ::SQLAllocHandle( SQL_HANDLE_DBC, env.env_, &dbc_ ), SQL_HANDLE_ENV, env.env_ );
    check( ::SQLDriverConnect( dbc_, nullptr, (SQLCHAR*) connStr, SQL_NTS, nullptr, 0, 0, SQL_DRIVER_COMPLETE_REQUIRED ), SQL_HANDLE_DBC, dbc_ );
    check( ::SQLSetConnectAttr( dbc_, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER) SQL_AUTOCOMMIT_OFF, 0 ), SQL_HANDLE_DBC, dbc_ );
}

Connection::~Connection()
{
    if ( dbc_ )
    {
        ::SQLFreeHandle( SQL_HANDLE_DBC, dbc_ );

        dbc_ = nullptr;
    }
}

Connection::Connection( Connection&& that ) noexcept
{
    dbc_ = that.dbc_;
    that.dbc_ = nullptr;
}

Connection& Connection::operator= ( Connection&& that ) noexcept
{
    std::swap( dbc_, that.dbc_ );

    return* this;
}

DBMS Connection::dbms() const
{
    if ( dbms_ )
    {
        return *dbms_;
    }

    SQLSMALLINT nameLength;
    check( ::SQLGetInfo( dbc_, SQL_DBMS_NAME, nullptr, 0, &nameLength ), SQL_HANDLE_DBC, dbc_ );

    std::string name;
    name.resize( nameLength + 1 );
    check( ::SQLGetInfo( dbc_, SQL_DBMS_NAME, &name.front(), name.size(), nullptr ), SQL_HANDLE_DBC, dbc_ );

    if ( boost::icontains( name, "SQLite" ) )
    {
        dbms_= DBMS::SQLite;
    }
    else if ( boost::icontains( name, "PostgreSQL" ) )
    {
        dbms_ = DBMS::PostgreSQL;
    }
    else if ( boost::icontains( name, "MySQL" ) )
    {
        dbms_ = DBMS::MySQL;
    }
    else
    {
        dbms_ = DBMS::Other;
    }

    return *dbms_;
}

IsolationLevel Connection::isolationLevel() const
{
    SQLUINTEGER txnIsolation;

    check( ::SQLGetConnectAttr( dbc_, SQL_ATTR_TXN_ISOLATION, &txnIsolation, 0, nullptr ), SQL_HANDLE_DBC, dbc_ );

    switch ( txnIsolation )
    {
    case SQL_TXN_READ_UNCOMMITTED:
        return IsolationLevel::ReadUncommitted;
    case SQL_TXN_READ_COMMITTED:
        return IsolationLevel::ReadCommitted;
    case SQL_TXN_REPEATABLE_READ:
        return IsolationLevel::RepeatableRead;
    case SQL_TXN_SERIALIZABLE:
        return IsolationLevel::Serializable;
    default:
        return IsolationLevel::Other;
    }
}

void Connection::setIsolationLevel( const IsolationLevel isolationLevel )
{
    SQLULEN txnIsolation;

    switch( isolationLevel )
    {
    case IsolationLevel::Other:
        check( ::SQLGetInfo( dbc_, SQL_DEFAULT_TXN_ISOLATION, &txnIsolation, 0, nullptr ), SQL_HANDLE_DBC, dbc_ );
        break;
    case IsolationLevel::ReadUncommitted:
        txnIsolation = SQL_TXN_READ_UNCOMMITTED;
        break;
    case IsolationLevel::ReadCommitted:
        txnIsolation = SQL_TXN_READ_COMMITTED;
        break;
    case IsolationLevel::RepeatableRead:
        txnIsolation = SQL_TXN_REPEATABLE_READ;
        break;
    case IsolationLevel::Serializable:
        txnIsolation = SQL_TXN_SERIALIZABLE;
        break;
    }

    check( ::SQLSetConnectAttr( dbc_, SQL_TXN_ISOLATION, (SQLPOINTER) txnIsolation, 0 ), SQL_HANDLE_DBC, dbc_ );
}

bool Connection::readOnly() const
{
    SQLUINTEGER accessMode;

    check( ::SQLGetConnectAttr( dbc_, SQL_ATTR_ACCESS_MODE, &accessMode, 0, nullptr ), SQL_HANDLE_DBC, dbc_ );

    return accessMode == SQL_MODE_READ_ONLY;
}

void Connection::setReadOnly( const bool readOnly )
{
    const SQLULEN accessMode = readOnly ? SQL_MODE_READ_ONLY : SQL_MODE_READ_WRITE;

    check( ::SQLSetConnectAttr( dbc_, SQL_ATTR_ACCESS_MODE, (SQLPOINTER) accessMode, 0 ), SQL_HANDLE_DBC, dbc_ );
}

bool Connection::isDead() const
{
    SQLUINTEGER dead = SQL_CD_TRUE;

    ::SQLGetConnectAttr( dbc_, SQL_ATTR_CONNECTION_DEAD, &dead, sizeof ( dead ), nullptr );

    return dead != SQL_CD_FALSE;
}

Transaction::Transaction( Connection& conn )
: dbc_{ conn.dbc_ }
{
}

Transaction::~Transaction()
{
    if ( dbc_ )
    {
        ::SQLEndTran( SQL_HANDLE_DBC, dbc_, SQL_ROLLBACK );

        dbc_ = nullptr;
    }
}

Transaction::Transaction( Transaction&& that ) noexcept
{
    dbc_ = that.dbc_;
    that.dbc_ = nullptr;
}

Transaction& Transaction::operator= ( Transaction&& that ) noexcept
{
    std::swap( dbc_, that.dbc_ );

    return *this;
}

void Transaction::commit()
{
    check( ::SQLEndTran( SQL_HANDLE_DBC, dbc_, SQL_COMMIT ), SQL_HANDLE_DBC, dbc_ );

    dbc_ = nullptr;
}

}
