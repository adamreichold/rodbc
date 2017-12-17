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

#include <boost/functional/hash.hpp>
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
inline constexpr std::size_t numberOfColumns()
{
    return boost::mpl::size< boost::fusion::flatten_view< Columns > >::value;
}

template< typename Columns, typename Action >
inline void forEachColumn( Action action )
{
    boost::mpl::for_each< boost::fusion::flatten_view< Columns > >( action );
}

enum class StatementCacheEntryType : std::uint8_t
{
    Select,
    Insert,
    Update,
    Delete
};

struct StatementCacheEntryBase
{
    virtual ~StatementCacheEntryBase();
};

template< typename Params, typename Cols, std::size_t... Indices >
struct StatementCacheEntry : StatementCacheEntryBase
{
    template< typename Factory >
    StatementCacheEntry( Connection& conn, Factory factory );

    using Stmt = TypedStatement< std::tuple< ColumnAt< Params, Indices >... >, Cols >;
    Stmt stmt;
};

template< std::size_t Size >
class StatementCache
{
public:
    template< StatementCacheEntryType Type, typename Params, typename Cols, std::size_t... Indices, typename Factory >
    typename StatementCacheEntry< Params, Cols, Indices... >::Stmt& lookUp( Connection& conn, Factory factory );

private:
    using Key = std::pair< StatementCacheEntryType, std::bitset< Size > >;
    std::unordered_map< Key, std::unique_ptr< StatementCacheEntryBase >, boost::hash< Key > > stmts_;
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

    static constexpr auto numberOfColumns = detail::numberOfColumns< Columns >();

    template< std::size_t Index >
    using ColumnAt = detail::ColumnAt< Columns, Index >;

    struct ColumnNames : std::array< std::string, numberOfColumns >
    {
        template< typename... Values >
        ColumnNames( Values&&... values );
    };

public:
    Table( Connection& conn, std::string name, ColumnNames columnNames );

    void create( const unsigned flags = 0 );
    void drop();

    boost::optional< Columns > select( const ColumnAt< PrimaryKey >&... primaryKey ) const;
    std::vector< Columns > selectAll() const;
    template< std::size_t... Key >
    std::vector< Columns > selectBy( const ColumnAt< Key >&... key ) const;

    void insert( const Columns& row ); ///< insert all values
    template< std::size_t... Value >
    void insertAt( const Columns& row, const IndexSequence< Value... >& ); ///< insert the given values

    void update( const Columns& row ); ///< update all values based on the primary key
    template< std::size_t... Key >
    void updateBy( const Columns& row, const IndexSequence< Key... >& ); ///< update all values based on the given key
    template< std::size_t... Value >
    void updateAt( const Columns& row, const IndexSequence< Value... >& ); ///< update the given values based on the pimary key
    template< std::size_t... Value, std::size_t... Key >
    void updateAtBy( const Columns& row, const IndexSequence< Value... >&, const IndexSequence< Key... >& ); ///< update the given values based on the given key

    void delete_( const ColumnAt< PrimaryKey >&... primaryKey );
    void deleteAll();
    template< std::size_t... Key >
    void deleteBy( const ColumnAt< Key >&... key );

protected:
    Connection& conn_;

    const std::string name_;
    const ColumnNames columnNames_;

private:
    mutable detail::StatementCache< 2 * numberOfColumns > cache_;
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

namespace boost
{

template< std::size_t Size >
struct hash< std::bitset< Size > >
{
    using argument_type = std::bitset< Size >;
    using result_type = std::size_t;

    result_type operator() ( const argument_type& val ) const
    {
        return std::hash< argument_type >{}( val );
    }
};

}
