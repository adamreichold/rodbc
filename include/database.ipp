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
#include "database.hpp"

#include "types.hpp"

#include <boost/thread/lock_guard.hpp>

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
template< typename Action >
inline void Database< Statements >::withStatements( Action action )
{
    closeDeadSession( [&]() { action( openSession().stmts ); } );
}


template< typename Statements >
inline Connection Database< Statements >::makeConnection()
{
    boost::lock_guard< boost::mutex > lock{ mutex_ };

    return { env_, connStr_ };
}


template< typename Statements >
inline Database< Statements >::Session::Session( Connection&& conn )
: conn{ std::move( conn ) }
, stmts{ this->conn }
{
}

template< typename Statements >
inline typename Database< Statements >::Session& Database< Statements >::openSession()
{
    auto* session = session_.get();

    if ( !session )
    {
        session = new Session{ makeConnection() };
        session_.reset( session );
    }

    return *session;
}

template< typename Statements >
template< typename Action >
inline auto Database< Statements >::closeDeadSession( Action action ) -> decltype ( action() )
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
