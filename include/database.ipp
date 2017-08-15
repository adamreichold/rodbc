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

template< typename Database_ >
inline Database< Database_ >::Database( std::string connStr )
: connStr_{ std::move( connStr ) }
{
}

template< typename Database_ >
inline Database< Database_ >::BoundTransaction::BoundTransaction( Database_& database )
: database{ database }
, transaction{ database.openSession().conn }
{
}

template< typename Database_ >
inline void Database< Database_ >::BoundTransaction::doCommit()
{
    database.closeDeadSession( [&]() { transaction.commit(); } );
}

template< typename Database_ >
inline Database< Database_ >::BoundStatement::BoundStatement( Database_& database )
: database{ database }
, stmts{ database.openSession().stmts }
{
}

template< typename Database_ >
template< typename Statement >
inline void Database< Database_ >::BoundStatement::doExec( Statement& stmt )
{
    database.closeDeadSession( [&]() { stmt.exec(); } );
}

template< typename Database_ >
template< typename Statement >
inline bool Database< Database_ >::BoundStatement::doFetch( Statement& stmt )
{
    return database.closeDeadSession( [&]() { return stmt.fetch(); } );
}

template< typename Database_ >
template< typename Action >
inline void Database< Database_ >::withStatements( Action action )
{
    closeDeadSession( [&]() { action( openSession().stmts ); } );
}


template< typename Database_ >
inline Connection Database< Database_ >::makeConnection()
{
    boost::lock_guard< boost::mutex > lock{ mutex_ };

    return { env_, connStr_.c_str() };
}


template< typename Database_ >
inline Database< Database_ >::Session::Session( Database_& db, Connection&& conn )
: conn{ std::move( conn ) }
, stmts{ db, this->conn }
{
}

template< typename Database_ >
inline typename Database< Database_ >::Session& Database< Database_ >::openSession()
{
    auto* session = session_.get();

    if ( !session )
    {
        session = new Session{ static_cast< Database_& >( *this ), makeConnection() };
        session_.reset( session );
    }

    return *session;
}

template< typename Database_ >
template< typename Action >
inline auto Database< Database_ >::closeDeadSession( Action action ) -> decltype ( action() )
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
