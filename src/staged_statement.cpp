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
#include "staged_statement.hpp"

namespace rodbc
{
namespace detail
{

std::string deleteFrom( const char* const tableName )
{
    std::string stmt{ "DELETE FROM " };

    stmt += tableName;

    return stmt;
}

std::string insertInto( const char* const tableName, const char* const* const columnNamesBegin, const char* const* const columnNamesEnd )
{
    std::string stmt{ "INSERT INTO " };

    stmt += tableName;
    stmt += " (";

    for ( auto columnName = columnNamesBegin; columnName != columnNamesEnd; ++columnName )
    {
        if ( columnName != columnNamesBegin )
        {
            stmt += ", ";
        }

        stmt += *columnName;
    }

    stmt += ") VALUES (";

    for ( auto columnName = columnNamesBegin; columnName != columnNamesEnd; ++columnName )
    {
        if ( columnName != columnNamesBegin )
        {
            stmt += ", ";
        }

        stmt += '?';
    }

    stmt += ')';

    return stmt;
}

}
}
