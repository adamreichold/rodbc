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

#include <boost/fusion/include/flatten_view.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/size.hpp>

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

template< typename Type > struct ColumnType;
template<> struct ColumnType< std::int8_t > { static constexpr const char* value = "TINYINT"; };
template<> struct ColumnType< std::int16_t > { static constexpr const char* value = "SMALLINT"; };
template<> struct ColumnType< std::int32_t > { static constexpr const char* value = "INT"; };
template<> struct ColumnType< std::int64_t > { static constexpr const char* value = "BIGINT"; };
template<> struct ColumnType< std::uint8_t > { static constexpr const char* value = "TINYINT UNSIGNED"; };
template<> struct ColumnType< std::uint16_t > { static constexpr const char* value = "SMALLINT UNSIGNED"; };
template<> struct ColumnType< std::uint32_t > { static constexpr const char* value = "INT UNSIGNED"; };
template<> struct ColumnType< std::uint64_t > { static constexpr const char* value = "BIGINT UNSIGNED"; };
template<> struct ColumnType< float > { static constexpr const char* value = "REAL"; };
template<> struct ColumnType< double > { static constexpr const char* value = "DOUBLE PRECISION"; };
template< std::size_t Size > struct ColumnType< String< Size > > { static constexpr const char* value = "TEXT"; };
template<> struct ColumnType< Timestamp > { static constexpr const char* value = "TIMESTAMP"; };
template< typename Type > struct ColumnType< Nullable< Type > > { static constexpr const char* value = ColumnType< Type >::value; };

struct ColumnTypeInserter
{
    const char** values;

    template< typename Type >
    void operator() ( const Type& )
    {
        *values++ = ColumnType< Type >::value;
    }
};

template< typename Columns >
inline constexpr bool areValidIndices()
{
    return true;
}

template< typename Columns, std::size_t Index, std::size_t... Indices >
inline constexpr bool areValidIndices()
{
    return sizeOfColumns< Columns >() > Index && areValidIndices< Columns, Indices... >();
}

void dropTableIfExists( Connection& conn, const char* const name );

void createTable(
    Connection& conn, const char* const name,
    const char* const* const columnNamesBegin, const char* const* const columnNamesEnd,
    const char* const* const columnTypesBegin, const char* const* const columnTypesEnd,
    const std::size_t* const primaryKeyBegin, const std::size_t* const primaryKeyEnd,
    const bool temporary
);

}

template< typename Columns, std::size_t... PrimaryKey >
inline CreateTable< Columns, PrimaryKey... >::CreateTable( Connection& conn, const char* const tableName, const ColumnNames& columnNames, const unsigned flags )
{
    if ( flags & DROP_TABLE_IF_EXISTS )
    {
        detail::dropTableIfExists( conn, tableName );
    }

    const char* columnTypes[ detail::sizeOfColumns< Columns >() ];
    detail::forEachColumn< Columns >( detail::ColumnTypeInserter{ columnTypes } );

    const std::size_t primaryKey[]{ PrimaryKey... };
    static_assert( detail::areValidIndices< Columns, PrimaryKey... >(), "Primary key column indices must be valid." );

    detail::createTable(
        conn, tableName,
        std::begin( columnNames ), std::end( columnNames ),
        std::begin( columnTypes ), std::end( columnTypes ),
        &primaryKey[ 0 ], &primaryKey[ sizeof... ( PrimaryKey ) ],
        flags & TEMPORARY_TABLE
    );
}

}
