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

#include <types.hpp>

#include <boost/fusion/include/flatten_view.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/size.hpp>

namespace rodbc
{

class Connection;

constexpr unsigned DROP_TABLE_IF_EXISTS = 1 << 0;
constexpr unsigned TEMPORARY_TABLE = 1 << 1;

/**
 * @brief The CreateTable struct template
 */
template< typename Columns >
struct CreateTable
{
    using ColumnNames = std::initializer_list< const char* >;
    using PrimaryKey = std::initializer_list< std::size_t >;

    CreateTable( Connection& conn, const char* const tableName, const ColumnNames& columnNames, const PrimaryKey& primaryKey = {}, const unsigned flags = 0 );
};

namespace detail
{

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

void dropTableIfExists( Connection& conn, const char* const name );

void createTable(
    Connection& conn, const char* const name,
    const char* const* const columnNamesBegin, const char* const* const columnNamesEnd,
    const char* const* const columnTypesBegin, const char* const* const columnTypesEnd,
    const std::size_t* const primaryKeyBegin, const std::size_t* const primaryKeyEnd,
    const bool temporary
);

}

template< typename Columns >
inline CreateTable< Columns >::CreateTable( Connection& conn, const char* const tableName, const ColumnNames& columnNames, const PrimaryKey& primaryKey, const unsigned flags )
{
    if ( flags & DROP_TABLE_IF_EXISTS )
    {
        detail::dropTableIfExists( conn, tableName );
    }

    const char* columnTypes[ boost::mpl::size< boost::fusion::flatten_view< Columns > >::value ];
    boost::mpl::for_each< boost::fusion::flatten_view< Columns > >( detail::ColumnTypeInserter{ columnTypes } );

    detail::createTable(
        conn, tableName,
        std::begin( columnNames ), std::end( columnNames ),
        std::begin( columnTypes ), std::end( columnTypes ),
        std::begin( primaryKey ), std::end( primaryKey ),
        flags & TEMPORARY_TABLE
    );
}

}
