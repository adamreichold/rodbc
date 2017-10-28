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

#include "create_table.hpp"

#include "column_definition.hpp"

#include <boost/fusion/include/flatten_view.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/integral_c.hpp>
#include <boost/mpl/size.hpp>

#include <array>

namespace rodbc
{
namespace detail
{

template< typename Columns >
inline constexpr std::size_t sizeOfColumns()
{
    return boost::mpl::size< boost::fusion::flatten_view< Columns > >::value;
}

template< typename Columns, typename Action >
inline void forEachColumn( Action action )
{
    boost::mpl::for_each< boost::fusion::flatten_view< Columns > >( action );
}

template< typename Columns, std::size_t Index >
using ColumnAt = typename boost::mpl::at< boost::fusion::flatten_view< Columns >, boost::mpl::integral_c< std::size_t, Index > >::type;

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

template< typename Columns >
inline constexpr bool isValidPrimaryKey()
{
    return true;
}

template< typename Columns, std::size_t Index, std::size_t... Indices >
inline constexpr bool isValidPrimaryKey()
{
    return sizeOfColumns< Columns >() > Index && !IsNullable< ColumnAt< Columns, Index > >::value && isValidPrimaryKey< Columns, Indices... >();
}

void dropTableIfExists( Connection& conn, const char* const name );

void createTable(
    Connection& conn, const char* const name,
    const char* const* const columnNames, const std::size_t columnNamesSize,
    const char* const* const columnTypes, const std::size_t* const columnSizes, const char* const* const columnConstraints,
    const std::size_t* const primaryKeyBegin, const std::size_t* const primaryKeyEnd,
    const bool temporary
);

template< typename Columns >
struct ColumnNamesBase : std::array< const char*, sizeOfColumns< Columns >() >
{
    template< typename... Values >
    ColumnNamesBase( Values&&... values )
    : std::array< const char*, sizeOfColumns< Columns >() >{ std::forward< Values >( values )... }
    {
        static_assert( sizeOfColumns< Columns >() == sizeof... ( Values ), "Number of columns and column names must be equal." );
    }
};

}

template< typename Columns, std::size_t... PrimaryKey >
struct CreateTable< Columns, PrimaryKey... >::ColumnNames : detail::ColumnNamesBase< Columns >
{
    using detail::ColumnNamesBase< Columns >::ColumnNamesBase;
};

template< typename Columns, std::size_t... PrimaryKey >
inline CreateTable< Columns, PrimaryKey... >::CreateTable( Connection& conn, const char* const tableName, const ColumnNames& columnNames, const unsigned flags )
{
    using namespace detail;

    const char* columnTypes[ columnNames.size() ];
    forEachColumn< Columns >( ColumnTypeInserter{ columnTypes } );

    std::size_t columnSizes[ columnNames.size() ];
    forEachColumn< Columns >( ColumnSizeInserter{ columnSizes } );

    const char* columnConstraints[ columnNames.size() ];
    forEachColumn< Columns >( ColumnConstraintInserter{ columnConstraints } );

    const std::size_t primaryKey[]{ PrimaryKey... };
    static_assert( isValidPrimaryKey< Columns, PrimaryKey... >(), "Primary key column indices must be valid." );

    if ( flags & DROP_TABLE_IF_EXISTS )
    {
        dropTableIfExists( conn, tableName );
    }

    createTable(
        conn, tableName,
        columnNames.data(), columnNames.size(),
        columnTypes, columnSizes, columnConstraints,
        &primaryKey[ 0 ], &primaryKey[ sizeof... ( PrimaryKey ) ],
        flags & TEMPORARY_TABLE
    );
}

}
