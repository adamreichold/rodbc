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

#include "typed_statement.hpp"

#include <boost/fusion/include/flatten_view.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/integral_c.hpp>
#include <boost/mpl/size.hpp>
#include <boost/optional.hpp>

namespace rodbc
{
namespace detail
{

template< typename Columns, std::size_t Index >
using ColumnAt = typename boost::mpl::at< boost::fusion::flatten_view< Columns >, boost::mpl::integral_c< std::size_t, Index > >::type;

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

template< typename Type >
struct IsNullable : std::false_type
{
};

template< typename Type >
struct IsNullable< Nullable< Type > > : std::true_type
{
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

}

class Connection;

constexpr unsigned DROP_TABLE_IF_EXISTS = 1 << 0;
constexpr unsigned TEMPORARY_TABLE = 1 << 1;

/**
 * @brief The Table class template
 */
template< typename Columns_, std::size_t... PrimaryKey >
class Table
{
    static_assert( detail::isValidPrimaryKey< Columns_, PrimaryKey... >(), "Primary key column indices must be valid." );

public:
    using Columns = Columns_;
    using PrimaryKeyColumns = std::tuple< detail::ColumnAt< Columns_, PrimaryKey >... >;

    struct ColumnNames;

public:
    struct Create
    {
        Create( Connection& conn, const char* const tableName, const ColumnNames& columnNames, const unsigned flags = 0 );
    };

public:
    Table( Connection& conn, const char* const tableName, const ColumnNames& columnNames );

    boost::optional< Columns > select( const PrimaryKeyColumns& primaryKey ) const;
    std::vector< Columns > selectAll() const;

    void insert( const Columns& row );
    void update( const PrimaryKeyColumns& primaryKey, const Columns& row );
    void erase( const PrimaryKeyColumns& primaryKey );

private:
    mutable TypedStatement< PrimaryKeyColumns, Columns > select_;
    mutable TypedStatement< std::tuple<>, Columns > selectAll_;

    TypedStatement< Columns, std::tuple<> > insert_;
    TypedStatement< std::tuple< Columns, PrimaryKeyColumns >, std::tuple<> > update_;
    TypedStatement< PrimaryKeyColumns, std::tuple<> > erase_;
};

}
