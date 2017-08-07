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

namespace rodbc
{
namespace detail
{

void dropTableIfExists( Connection& conn, const char* const name )
{
    std::string stmt{ "DROP TABLE IF EXISTS " };

    stmt += name;

    Statement{ conn, stmt.c_str() }.exec();
}

void createTable(
    Connection& conn, const char* const name,
    const char* const* const columnNamesBegin, const char* const* const columnNamesEnd,
    const char* const* const columnTypesBegin, const char* const* const columnTypesEnd,
    const std::size_t* const primaryKeyBegin, const std::size_t* const primaryKeyEnd,
    const bool temporary
)
{
    std::string stmt{ "CREATE " };

    if ( temporary )
    {
        stmt += "TEMPORARY ";
    }

    stmt += "TABLE ";
    stmt += name;
    stmt += " (";

    for ( auto columnName = columnNamesBegin, columnType = columnTypesBegin; columnName != columnNamesEnd && columnType != columnTypesEnd; ++columnName, ++columnType )
    {
        if ( columnName != columnNamesBegin )
        {
            stmt += ", ";
        }

        stmt += *columnName;
        stmt += ' ';
        stmt += *columnType;
    }

    if ( primaryKeyBegin != primaryKeyEnd )
    {
        stmt += ", PRIMARY KEY (";

        for ( auto primaryKey = primaryKeyBegin; primaryKey != primaryKeyEnd; ++primaryKey )
        {
            if ( primaryKey != primaryKeyBegin )
            {
                stmt += ", ";
            }

            stmt += columnNamesBegin[ *primaryKey ];
        }

        stmt += ')';
    }

    stmt += ')';

    Statement{ conn, stmt.c_str() }.exec();
}

}
}
