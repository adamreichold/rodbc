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
#include "create_table.hpp"

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

void createTable( Connection& conn, const char* const name, const char* const definition, const bool temporary )
{
    std::string stmt{ "CREATE " };

    if ( temporary )
    {
        stmt += "TEMPORARY ";
    }

    stmt += "TABLE ";
    stmt += name;
    stmt += " (";
    stmt += definition;
    stmt += ')';

    Statement{ conn, stmt.c_str() }.exec();
}

void defineColumn( std::string& definition, const char* const name, const char* const type )
{
    if ( !definition.empty() )
    {
        definition += ", ";
    }

    definition += name;
    definition += ' ';
    definition += type;
}

void definePrimaryKey( std::string& definition, const std::vector< const char* >& columnNames, const std::vector< std::size_t >& columns )
{
    if ( columns.empty() )
    {
        return;
    }

    definition += ", PRIMARY KEY (";

    for ( std::size_t index = 0; index < columns.size(); ++index )
    {
        if ( index > 0 )
        {
            definition += ", ";
        }

        definition += columnNames[ columns[ index ] ];
    }

    definition += ')';
}

}
}
