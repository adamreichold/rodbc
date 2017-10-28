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

#include "table.hpp"

#include "column_definition.hpp"
#include "typed_statement.ipp"

#include <array>

namespace rodbc
{
namespace detail
{

struct ColumnTypeInserter
{
    const char** values;

    template< typename Column >
    void operator() ( const Column& )
    {
        *values++ = ColumnDefinition< Column >::type;
    }
};

struct ColumnSizeInserter
{
    std::size_t* values;

    template< typename Column >
    void operator() ( const Column& )
    {
        *values++ = ColumnDefinition< Column >::size;
    }
};

struct ColumnConstraintInserter
{
    const char** values;

    template< typename Column >
    void operator() ( const Column& )
    {
        *values++ = ColumnDefinition< Column >::constraint;
    }
};

void dropTableIfExists(
    Connection& conn,
    const char* const tableName
);

void createTable(
    Connection& conn,
    const char* const tableName,
    const char* const* const columnNames, const std::size_t columnNamesSize,
    const char* const* const columnTypes, const std::size_t* const columnSizes, const char* const* const columnConstraints,
    const std::initializer_list< std::size_t >& primaryKey,
    const bool temporary
);

std::string selectFrom(
    const char* const tableName,
    const char* const* const columnNames, const std::size_t columnNamesSize,
    const std::initializer_list< std::size_t >& primaryKey
);

std::string selectFrom(
    const char* const tableName,
    const char* const* const columnNames, const std::size_t columnNamesSize
);

std::string insertInto(
    const char* const tableName,
    const char* const* const columnNames, const std::size_t columnNamesSize
);

std::string updateSet(
    const char* const tableName,
    const char* const* const columnName, const std::size_t columnNamesSize,
    const std::initializer_list< std::size_t >& primaryKey
);

std::string deleteFrom(
    const char* const tableName
);

std::string deleteFrom(
    const char* const tableName,
    const char* const* const columnNames,
    const std::initializer_list< std::size_t >& primaryKey
);

}

template< typename Columns, std::size_t... PrimaryKey >
struct Table< Columns, PrimaryKey... >::ColumnNames : std::array< const char*, detail::sizeOfColumns< Columns >() >
{
    template< typename... Values >
    ColumnNames( Values&&... values )
    : std::array< const char*, detail::sizeOfColumns< Columns >() >{ std::forward< Values >( values )... }
    {
        static_assert( detail::sizeOfColumns< Columns >() == sizeof... ( Values ), "Number of columns and column names must be equal." );
    }
};

template< typename Columns, std::size_t... PrimaryKey >
inline Table< Columns, PrimaryKey... >::Create::Create( Connection& conn, const char* const tableName, const ColumnNames& columnNames, const unsigned flags )
{
    using namespace detail;

    const auto columns = sizeOfColumns< Columns >();

    const char* columnTypes[ columns ];
    forEachColumn< Columns >( ColumnTypeInserter{ columnTypes } );

    std::size_t columnSizes[ columns ];
    forEachColumn< Columns >( ColumnSizeInserter{ columnSizes } );

    const char* columnConstraints[ columns ];
    forEachColumn< Columns >( ColumnConstraintInserter{ columnConstraints } );

    if ( flags & DROP_TABLE_IF_EXISTS )
    {
        dropTableIfExists( conn, tableName );
    }

    createTable(
        conn,
        tableName,
        columnNames.data(), columnNames.size(),
        columnTypes, columnSizes, columnConstraints,
        { PrimaryKey... },
        flags & TEMPORARY_TABLE
    );
}

template< typename Columns, std::size_t... PrimaryKey >
Table< Columns, PrimaryKey... >::Table( Connection& conn, const char* const tableName, const ColumnNames& columnNames )
: select_{ conn, detail::selectFrom( tableName, columnNames.data(), columnNames.size(), { PrimaryKey... } ).c_str() }
, selectAll_{ conn, detail::selectFrom( tableName, columnNames.data(), columnNames.size() ).c_str() }
, insert_{ conn, detail::insertInto( tableName, columnNames.data(), columnNames.size() ).c_str() }
, update_{ conn, detail::updateSet( tableName, columnNames.data(), columnNames.size(), { PrimaryKey... } ).c_str() }
, erase_{ conn, detail::deleteFrom( tableName, columnNames.data(), { PrimaryKey... } ).c_str() }
{
}

template< typename Columns, std::size_t... PrimaryKey >
boost::optional< Columns > Table< Columns, PrimaryKey... >::select( const PrimaryKeyColumns& primaryKey ) const
{
    select_.params() = primaryKey;

    select_.exec();

    if ( !select_.fetch() )
    {
        return boost::none;
    }

    return select_.cols();
}

template< typename Columns, std::size_t... PrimaryKey >
std::vector< Columns > Table< Columns, PrimaryKey... >::selectAll() const
{
    std::vector< Columns > rows;

    selectAll_.exec();

    while ( selectAll_.fetch() )
    {
        rows.push_back( selectAll_.cols() );
    }

    return rows;
}

template< typename Columns, std::size_t... PrimaryKey >
void Table< Columns, PrimaryKey... >::insert( const Columns& row )
{
    insert_.params() = row;

    insert_.exec();
}

template< typename Columns, std::size_t... PrimaryKey >
void Table< Columns, PrimaryKey... >::update( const PrimaryKeyColumns& primaryKey, const Columns& row )
{
    update_.params() = std::tie( row, primaryKey );

    update_.exec();
}

template< typename Columns, std::size_t... PrimaryKey >
void Table< Columns, PrimaryKey... >::erase( const PrimaryKeyColumns& primaryKey )
{
    erase_.params() = primaryKey;

    erase_.exec();
}

}
