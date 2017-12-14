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

#include <bitset>
#include <unordered_map>

namespace rodbc
{
namespace detail
{

template< typename Columns, std::size_t Index >
using ColumnAt = typename boost::mpl::at<
    boost::fusion::flatten_view< Columns >,
    boost::mpl::integral_c< std::size_t, Index >
>::type;

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

struct StatementCacheEntryBase
{
    virtual ~StatementCacheEntryBase();
};

template< typename Columns, std::size_t... Key >
struct StatementCacheEntry : StatementCacheEntryBase
{
    template< typename Factory >
    StatementCacheEntry( Connection& conn, Factory factory );

    TypedStatement< std::tuple< ColumnAt< Columns, Key >... >, Columns > stmt;
};

template< typename Columns >
class StatementCache
{
public:
    template< std::size_t... Key, typename Factory >
    TypedStatement< std::tuple< ColumnAt< Columns, Key >... >, Columns >& lookUp( Connection& conn, Factory factory );

private:
    std::unordered_map< std::bitset< sizeOfColumns< Columns >() >, std::unique_ptr< StatementCacheEntryBase > > stmts_;
};

}

constexpr unsigned DROP_TABLE_IF_EXISTS = 1 << 0;
constexpr unsigned TEMPORARY_TABLE = 1 << 1;

/**
 * @brief The Table class template
 */
template< typename Columns_, std::size_t... PrimaryKey >
class Table
{
public:
    using Columns = Columns_;

    struct ColumnNames : std::array< std::string, detail::sizeOfColumns< Columns >() >
    {
        template< typename... Values >
        ColumnNames( Values&&... values );
    };

    template< std::size_t Index >
    using ColumnAt = detail::ColumnAt< Columns, Index >;

public:
    Table( Connection& conn, std::string name, ColumnNames columnNames );

    void create( const unsigned flags = 0 );
    void drop();

    boost::optional< Columns > select( const ColumnAt< PrimaryKey >&... primaryKey ) const;
    std::vector< Columns > selectAll() const;

    template< std::size_t... Key >
    std::vector< Columns > selectBy( const ColumnAt< Key >&... key ) const;

    void insert( const Columns& row );

    void update( const Columns& row );
    void update_( const Columns& row, const ColumnAt< PrimaryKey >&... primaryKey );

    void delete_( const ColumnAt< PrimaryKey >&... primaryKey );
    void deleteAll();

protected:
    Connection& conn_;

    const std::string name_;
    const ColumnNames columnNames_;

private:
    mutable boost::optional< TypedStatement< std::tuple< ColumnAt< PrimaryKey >... >, Columns > > select_;
    mutable boost::optional< TypedStatement< std::tuple<>, Columns > > selectAll_;

    mutable detail::StatementCache< Columns > selectBy_;

    boost::optional< TypedStatement< Columns, std::tuple<> > > insert_;

    boost::optional< TypedStatement< std::tuple< Columns, ColumnAt< PrimaryKey >... >, std::tuple<> > > update__;

    boost::optional< TypedStatement< std::tuple< ColumnAt< PrimaryKey >... >, std::tuple<> > > delete__;
    boost::optional< TypedStatement< std::tuple<>, std::tuple<> > > deleteAll_;
};

/**
 * @brief The CreateTable struct template
 */
template< typename Columns, std::size_t... PrimaryKey >
struct CreateTable
{
    using Table = rodbc::Table< Columns, PrimaryKey... >;

    CreateTable( Connection& conn, const std::string& name, const typename Table::ColumnNames& columnNames, const unsigned flags = 0 );
};

}
