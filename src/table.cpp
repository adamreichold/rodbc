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
#include "table.ipp"

#include "statement.hpp"

#include <sstream>

namespace rodbc
{
namespace detail
{

void dropTableIfExists(
    Connection& conn,
    const char* const tableName
)
{
    std::ostringstream stmt;

    stmt << "DROP TABLE IF EXISTS " << tableName;

    Statement{ conn, stmt.str().c_str() }.exec();
}

void createTable(
    Connection& conn,
    const char* const tableName,
    const char* const* const columnNames, const std::size_t columnNamesSize,
    const char* const* const columnTypes, const std::size_t* const columnSizes, const char* const* const columnConstraints,
    const std::initializer_list< std::size_t >& primaryKey,
    const bool temporary
)
{
    std::ostringstream stmt;

    stmt << "CREATE ";

    if ( temporary )
    {
        stmt << "TEMPORARY ";
    }

    stmt << "TABLE " << tableName << " (";

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

    if ( primaryKey.size() != 0 )
    {
        stmt << ", PRIMARY KEY (";

        for ( auto column = primaryKey.begin(); column != primaryKey.end(); ++column )
        {
            if ( column != primaryKey.begin() )
            {
                stmt << ", ";
            }

            stmt << columnNames[ *column ];
        }

        stmt << ')';
    }

    stmt << ')';

    Statement{ conn, stmt.str().c_str() }.exec();
}

std::string selectFrom(
    const char* const tableName,
    const char* const* const columnNames, const std::size_t columnNamesSize,
    const std::initializer_list< std::size_t >& primaryKey
)
{
    std::ostringstream stmt;

    stmt << "SELECT ";

    for ( std::size_t column = 0; column != columnNamesSize; ++column )
    {
        if ( column != 0 )
        {
            stmt << ", ";
        }

        stmt << columnNames[ column ];
    }

    stmt << " FROM " << tableName << " WHERE ";

    for ( auto column = primaryKey.begin(); column != primaryKey.end(); ++column )
    {
        if ( column != primaryKey.begin() )
        {
            stmt << " AND ";
        }

        stmt << columnNames[ *column ] << " = ?";
    }

    return stmt.str();
}

std::string selectFrom(
    const char* const tableName,
    const char* const* const columnNames, const std::size_t columnNamesSize
)
{
    std::ostringstream stmt;

    stmt << "SELECT ";

    for ( std::size_t column = 0; column != columnNamesSize; ++column )
    {
        if ( column != 0 )
        {
            stmt << ", ";
        }

        stmt << columnNames[ column ];
    }

    stmt << " FROM " << tableName;

    return stmt.str();
}

std::string insertInto( const char* const tableName, const char* const* const columnNames, const std::size_t columnNamesSize )
{
    std::ostringstream stmt;

    stmt << "INSERT INTO " << tableName << " (";

    for ( std::size_t column = 0; column != columnNamesSize; ++column )
    {
        if ( column != 0 )
        {
            stmt << ", ";
        }

        stmt << columnNames[ column ];
    }

    stmt << ") VALUES (";

    for ( std::size_t column = 0; column != columnNamesSize; ++column )
    {
        if ( column != 0 )
        {
            stmt << ", ";
        }

        stmt << '?';
    }

    stmt << ')';

    return stmt.str();
}

std::string updateSet(
    const char* const tableName,
    const char* const* const columnNames, const std::size_t columnNamesSize,
    const std::initializer_list< std::size_t >& primaryKey
)
{
    std::ostringstream stmt;

    stmt << "UPDATE " << tableName << " SET ";

    for ( std::size_t column = 0; column != columnNamesSize; ++column )
    {
        if ( column != 0 )
        {
            stmt << ", ";
        }

        stmt << columnNames[ column ] << " = ?";
    }

    stmt << " WHERE ";

    for ( auto column = primaryKey.begin(); column != primaryKey.end(); ++column )
    {
        if ( column != primaryKey.begin() )
        {
            stmt << " AND ";
        }

        stmt << columnNames[ *column ] << " = ?";
    }

    return stmt.str();
}

std::string deleteFrom( const char* const tableName )
{
    std::ostringstream stmt;

    stmt << "DELETE FROM " << tableName;

    return stmt.str();
}

std::string deleteFrom(
    const char* const tableName,
    const char* const* const columnNames,
    const std::initializer_list< std::size_t >& primaryKey
)
{
    std::ostringstream stmt;

    stmt << "DELETE FROM " << tableName << " WHERE ";

    for ( auto column = primaryKey.begin(); column != primaryKey.end(); ++column )
    {
        if ( column != primaryKey.begin() )
        {
            stmt << " AND ";
        }

        stmt << columnNames[ *column ] << " = ?";
    }

    return stmt.str();
}

}
}
