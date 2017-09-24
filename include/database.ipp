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

#include "connection_pool.ipp"

#include <boost/thread/lock_guard.hpp>

namespace rodbc
{

template< typename Database_, typename Statements, typename ConnectionPool >
template< typename... Args >
inline Database< Database_, Statements, ConnectionPool >::Database(const char* const connStr , Args&&... args)
: pool_{ connStr, std::forward< Args >( args )... }
{
}

template< typename Database_, typename Statements, typename ConnectionPool >
inline Database< Database_, Statements, ConnectionPool >::BoundTransaction::BoundTransaction( Database_& database )
: database{ database }
, lease{ database.pool_ }
, transaction{ lease( []( Connection& conn ) { return Transaction{ conn }; } ) }
{
}

template< typename Database_, typename Statements, typename ConnectionPool >
inline void Database< Database_, Statements, ConnectionPool >::BoundTransaction::doCommit()
{
    lease( [ this ]() { transaction.commit(); } );
}

template< typename Database_, typename Statements, typename ConnectionPool >
inline Database< Database_, Statements, ConnectionPool >::BoundStatement::BoundStatement( BoundTransaction& transaction )
: database{ transaction.database }
, lease{ transaction.lease }
, stmts{ *lease( []( Statements& stmts ) { return &stmts; } ) }
{
}

template< typename Database_, typename Statements, typename ConnectionPool >
template< typename Statement >
inline void Database< Database_, Statements, ConnectionPool >::BoundStatement::doExec( Statement& stmt )
{
    lease( [ &stmt ]() { stmt.exec(); } );
}

template< typename Database_, typename Statements, typename ConnectionPool >
template< typename Statement >
inline bool Database< Database_, Statements, ConnectionPool >::BoundStatement::doFetch( Statement& stmt )
{
    return lease( [ &stmt ]() { return stmt.fetch(); } );
}

}
