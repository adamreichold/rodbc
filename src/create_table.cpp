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
#include "create_table.ipp"

#include "statement.hpp"

#include <sstream>

namespace rodbc
{
namespace detail
{

void dropTableIfExists( Connection& conn, const char* const name )
{
    std::ostringstream stmt;

    stmt << "DROP TABLE IF EXISTS " << name;

    Statement{ conn, stmt.str().c_str() }.exec();
}

void createTable(
    Connection& conn, const char* const name,
    const char* const* const columnNames, const std::size_t columnNamesSize,
    const char* const* const columnTypes, const std::size_t* const columnSizes, const char* const* const columnConstraints,
    const std::size_t* const primaryKeyBegin, const std::size_t* const primaryKeyEnd,
    const bool temporary
)
{
    std::ostringstream stmt;

    stmt << "CREATE ";

    if ( temporary )
    {
        stmt << "TEMPORARY ";
    }

    stmt << "TABLE " << name << " (";

    for ( std::size_t column = 0; column != columnNamesSize; ++column )
    {
        if ( column != 0 )
        {
            stmt << ", ";
        }

        stmt << columnNames[ column ] << ' ' << columnTypes[ column ];

        if ( const std::size_t size = columnSizes[ column ] )
        {
            stmt << '(' << size << ')';
        }

        if ( const char* const constraint = columnConstraints[ column ] )
        {
            stmt << ' ' << constraint;
        }
    }

    if ( primaryKeyBegin != primaryKeyEnd )
    {
        stmt << ", PRIMARY KEY (";

        for ( const auto* primaryKey = primaryKeyBegin; primaryKey != primaryKeyEnd; ++primaryKey )
        {
            if ( primaryKey != primaryKeyBegin )
            {
                stmt << ", ";
            }

            stmt << columnNames[ *primaryKey ];
        }

        stmt << ')';
    }

    stmt << ')';

    Statement{ conn, stmt.str().c_str() }.exec();
}

}
}
