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
#pragma once

#include "connection_pool.hpp"

namespace rodbc
{

/**
 * @brief The Database class template
 */
template< typename Database_, typename Statements, typename ConnectionPool >
class Database : private boost::noncopyable
{
public:
    template< typename... Args >
    explicit Database( const char* const connStr, Args&&... args );

protected:
    using Lease = typename ConnectionPool::Lease;

    class BoundStatement;

    class BoundTransaction : private boost::noncopyable
    {
    protected:
        BoundTransaction( Database_& database );

        void doCommit();

        Database_& database;
        Lease lease;
        Transaction transaction;

        friend class BoundStatement;
    };

    class BoundStatement : private boost::noncopyable
    {
    protected:
        BoundStatement( BoundTransaction& transaction );

        template< typename Statement >
        void doExec( Statement& stmt );
        template< typename Statement >
        bool doFetch( Statement& stmt );

        Database_& database;
        Lease& lease;
        Statements& stmts;
    };

    ConnectionPool pool_;
};

}
