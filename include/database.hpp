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

#include "connection.hpp"

#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/tss.hpp>

namespace rodbc
{

/**
 * @brief The Database class template
 */
template< typename Database_ >
class Database : private boost::noncopyable
{
public:
    Database( std::string connStr );

protected:
    class BoundTransaction : private boost::noncopyable
    {
    protected:
        BoundTransaction( Database_& database );

        void doCommit();

        Database_& database;
        Transaction transaction;
    };

    class BoundStatement : private boost::noncopyable
    {
    protected:
        BoundStatement( Database_& database );

        template< typename Statement >
        void doExec( Statement& stmt );
        template< typename Statement >
        bool doFetch( Statement& stmt );

        Database_& database;
        typename Database_::Statements& stmts;
    };

    template< typename Action >
    void withStatements( Action action );

private:
    boost::mutex mutex_;
    Environment env_;
    const std::string connStr_;

    Connection makeConnection();

    struct Session
    {
        Session( Database_& db, Connection&& conn );

        Connection conn;
        typename Database_::Statements stmts;
    };

    boost::thread_specific_ptr< Session > session_;

    Session& openSession();

    template< typename Action >
    auto closeDeadSession( Action action ) -> decltype ( action() );
};

}
