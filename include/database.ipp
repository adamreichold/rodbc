#include "database.hpp"

#include "types.hpp"

namespace rodbc
{

template< typename Statements >
inline Database< Statements >::Database( const char* const connStr )
: connStr_{ connStr }
{
}

template< typename Statements >
inline Database< Statements >::BoundTransaction::BoundTransaction( Database& database )
: database_{ database }
, transaction_{ database_.openSession().conn }
{
}

template< typename Statements >
inline Database< Statements >::BoundTransaction::BoundTransaction( BoundTransaction&& that )
: database_{ that.database_ }
, transaction_{ std::move( that.transaction_ ) }
{
}

template< typename Statements >
inline void Database< Statements >::BoundTransaction::doCommit()
{
    database_.closeDeadSession( [&]() { transaction_.commit(); } );
}

template< typename Statements >
inline Database< Statements >::BoundStatement::BoundStatement( Database& database )
: database_{ database }
, stmts_{ database_.openSession().stmts }
{
}

template< typename Statements >
template< typename Statement >
inline void Database< Statements >::BoundStatement::doExec( Statement& stmt )
{
    database_.closeDeadSession( [&]() { stmt.exec(); } );
}

template< typename Statements >
template< typename Statement >
inline bool Database< Statements >::BoundStatement::doFetch( Statement& stmt )
{
    return database_.closeDeadSession( [&]() { return stmt.fetch(); } );
}

template< typename Statements >
inline Database< Statements >::Session::Session( const char* const connStr )
: conn{ connStr }
, stmts{ conn }
{
}

template< typename Statements >
inline typename Database< Statements >::Session& Database< Statements >::openSession()
{
    auto* session = session_.get();

    if ( !session )
    {
        session = new Session{ connStr_ };

        session_.reset( session );
    }

    return *session;
}

template< typename Statements >
template< typename Action >
inline auto Database< Statements >::closeDeadSession( Action action ) -> decltype( action() )
{
    try
    {
        return action();
    }
    catch ( Exception& )
    {
        if ( const auto* const session = session_.get() )
        {
            if ( session->conn.isDead() )
            {
                session_.reset();
            }
        }

        throw;
    }
}

}
