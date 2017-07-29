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
#include "db.hpp"

#include "database.ipp"
#include "typed_statement.hpp"

#include <boost/fusion/include/tuple.hpp>

namespace
{

template< typename Value >
void resize( std::vector< Value >& values, const std::size_t size )
{
    values.resize( size );
    values.shrink_to_fit();

    assert( values.size() == values.capacity() );
}

}

namespace foobar
{

struct CreateTables
{
    CreateTables( rodbc::Connection& conn )
    {
        rodbc::Statement{ conn, "CREATE TABLE IF NOT EXISTS foo (x INT, y INT, z INT);" }.exec();
        rodbc::Statement{ conn, "CREATE TABLE IF NOT EXISTS bar (a FLOAT, b FLOAT, c FLOAT);" }.exec();
    }
};

struct Statements
{
    Foo foo;
    std::vector< Bar > bar;
    boost::fusion::tuple< float > a;

    CreateTables tables;

    rodbc::TypedStatement< Foo, rodbc::None > insertFoo;
    rodbc::TypedStatement< rodbc::None, Foo > selectAllFoo;

    rodbc::TypedStatement< std::vector< Bar >, rodbc::None > insertBar;
    rodbc::TypedStatement< decltype( a ), std::vector< Bar > > selectBarByA;

    Statements( rodbc::Connection& conn )
    : tables{ conn }
    , insertFoo{ conn, "INSERT INTO foo (x, y, z) VALUES (?, ?, ?);", foo }
    , selectAllFoo{ conn, "SELECT x, y, z FROM foo;", foo }
    , insertBar{ conn, "INSERT INTO bar (a, b, c) VALUES (?, ?, ?);", bar }
    , selectBarByA{ conn, "SELECT a, b, c FROM bar WHERE a < ?;", a, bar }
    {
    }
};

Database::Database( const char* const connStr )
: rodbc::Database< Statements >{ connStr }
{
}

Database::Transaction::Transaction( Database& database )
: BoundTransaction{ database }
{
}

void Database::Transaction::commit()
{
    doCommit();
}

Database::InsertFoo::InsertFoo( Database& database )
: BoundStatement{ database }
, foo{ stmts_.foo }
{
}

void Database::InsertFoo::exec()
{
    doExec( stmts_.insertFoo );
}

Database::SelectAllFoo::SelectAllFoo( Database& database )
: BoundStatement{ database }
, foo{ stmts_.foo }
{
}

void Database::SelectAllFoo::exec()
{
    doExec( stmts_.selectAllFoo );
}

bool Database::SelectAllFoo::fetch()
{
    return doFetch( stmts_.selectAllFoo );
}

Database::InsertBar::InsertBar( Database& database )
: BoundStatement{ database }
, bar{ stmts_.bar }
{
}

void Database::InsertBar::exec()
{
    doExec( stmts_.insertBar );
}

Database::SelectBarByA::SelectBarByA( Database& database, const std::size_t batchSize )
: BoundStatement{ database }
, a{ stmts_.a.get() }
, bar{ stmts_.bar }
{
    resize( stmts_.bar, batchSize );
}

void Database::SelectBarByA::exec()
{
    doExec( stmts_.selectBarByA );
}

bool Database::SelectBarByA::fetch()
{
    return doFetch( stmts_.selectBarByA );
}

}
