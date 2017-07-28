#include "connection.hpp"

#include "types.hpp"

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

}

Connection::Connection( const char* const connStr )
{
    check( ::SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env_ ), SQL_HANDLE_ENV, nullptr );
    check( ::SQLSetEnvAttr( env_, SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC3, 0), SQL_HANDLE_ENV, env_ );
    check( ::SQLSetEnvAttr( env_, SQL_ATTR_CONNECTION_POOLING, (SQLPOINTER) SQL_CP_OFF, 0), SQL_HANDLE_ENV, env_ );

    check( ::SQLAllocHandle( SQL_HANDLE_DBC, env_, &dbc_ ), SQL_HANDLE_ENV, env_ );
    check( ::SQLDriverConnect( dbc_, nullptr, (SQLCHAR*) connStr, SQL_NTS, nullptr, 0, 0, SQL_DRIVER_COMPLETE_REQUIRED ), SQL_HANDLE_DBC, dbc_ );
    check( ::SQLSetConnectAttr( dbc_, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER) SQL_FALSE, 0 ), SQL_HANDLE_DBC, dbc_ );
}

Connection::~Connection()
{
    ::SQLFreeHandle( SQL_HANDLE_DBC, dbc_ );
    ::SQLFreeHandle( SQL_HANDLE_ENV, env_ );
}

bool Connection::isDead() const
{
    SQLUINTEGER dead = SQL_CD_TRUE;

    ::SQLGetConnectAttr( dbc_, SQL_ATTR_CONNECTION_DEAD, &dead, sizeof( dead ), nullptr );

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
        check( ::SQLEndTran( SQL_HANDLE_DBC, dbc_, SQL_ROLLBACK ), SQL_HANDLE_DBC, dbc_ );
    }
}

Transaction::Transaction( Transaction&& that )
{
    dbc_ = that.dbc_;
    that.dbc_ = nullptr;
}

Transaction& Transaction::operator=( Transaction&& that )
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
