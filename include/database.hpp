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
#include <boost/thread/tss.hpp>

namespace rodbc
{

template< typename Statements >
class Database : private boost::noncopyable
{
public:
    Database( const char* const connStr );

protected:
    class BoundTransaction : private boost::noncopyable
    {
    protected:
        BoundTransaction( Database& database );

        void doCommit();

    private:
        Database& database_;
        Transaction transaction_;
    };

    class BoundStatement : private boost::noncopyable
    {
    protected:
        BoundStatement( Database& database );

        Database& database_;
        Statements& stmts_;

        template< typename Statement >
        void doExec( Statement& stmt );
        template< typename Statement >
        bool doFetch( Statement& stmt );
    };

    template< typename Action >
    void withStatements( Action action );

private:
    const char* const connStr_;

    struct Session
    {
        Session( const char* const connStr );

        Environment env;
        Connection conn;
        Statements stmts;
    };

    boost::thread_specific_ptr< Session > session_;

    Session& openSession();

    template< typename Action >
    auto closeDeadSession( Action action ) -> decltype ( action() );
};

}
