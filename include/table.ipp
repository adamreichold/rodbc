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
#include "result_set.ipp"

#include <array>

namespace rodbc
{
namespace detail
{

template< std::size_t Size >
inline std::bitset< Size > makeKey()
{
    return {};
}

template< std::size_t Size, std::size_t Index, std::size_t... Indices >
inline std::bitset< Size > makeKey()
{
    auto key = makeKey< Size, Indices... >();

    key.set( Index );

    return key;
}

template< typename Params, typename Cols, std::size_t... Indices >
template< typename Factory >
inline StatementCacheEntry< Params, Cols, Indices... >::StatementCacheEntry( Connection& conn, Factory factory )
: stmt{ conn, factory().c_str() }
{
}

template< std::size_t Size >
template< StatementCacheEntryType Type, typename Params, typename Cols, std::size_t... Indices, typename Factory >
inline typename StatementCacheEntry< Params, Cols, Indices... >::Stmt& StatementCache< Size >::lookUp( Connection& conn, Factory factory )
{
    const auto key = std::make_pair( Type, makeKey< Size, Indices... >() );

    auto entry = stmts_.find( key );

    if ( entry == stmts_.end() )
    {
        entry = stmts_.emplace( key, Value{ new StatementCacheEntry< Params, Cols, Indices... >{ conn, factory } } ).first;
    }

    return static_cast< StatementCacheEntry< Params, Cols, Indices... >& >( *entry->second ).stmt;
}

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

void create(
    Connection& conn,
    const std::string& tableName,
    const std::string* const columnNames, const std::size_t numberOfColumns,
    const char* const* const columnTypes, const std::size_t* const columnSizes, const char* const* const columnConstraints,
    const std::initializer_list< std::size_t >& primaryKey,
    const bool temporary
);

void drop(
    Connection& conn,
    const std::string& tableName,
    const bool ifExists
);

std::string select(
    const std::string& tableName,
    const std::string* const columnNames, const std::size_t numberOfColumns,
    const std::initializer_list< std::size_t >& key
);

std::string insert(
    const std::string& tableName,
    const std::string* const columnNames,
    const std::initializer_list< std::size_t >& value
);

template< std::size_t... Value >
inline std::string insert(
    const std::string& tableName,
    const std::string* const columnNames,
    const IndexSequence< Value... >&
)
{
    return insert( tableName, columnNames, { Value... } );
}

std::string update(
    const std::string& tableName,
    const std::string* const columnNames,
    const std::initializer_list< std::size_t >& value,
    const std::initializer_list< std::size_t >& key
);

std::string delete_(
    const std::string& tableName,
    const std::string* const columnNames,
    const std::initializer_list< std::size_t >& key
);

}

template< typename Columns, std::size_t... PrimaryKey >
template< typename... Values >
inline Table< Columns, PrimaryKey... >::ColumnNames::ColumnNames( Values&&... values )
: std::array< std::string, numberOfColumns >( { std::forward< Values >( values )... } )
{
    static_assert( numberOfColumns == sizeof... ( Values ), "Number of columns and column names must be equal." );
}

template< typename Columns, std::size_t... PrimaryKey >
inline Table< Columns, PrimaryKey... >::Table( Connection& conn, std::string name, ColumnNames columnNames )
: conn_( conn )
, name_{ std::move( name ) }
, columnNames_{ std::move( columnNames ) }
{
}

template< typename Columns, std::size_t... PrimaryKey >
inline void Table< Columns, PrimaryKey... >::create( const unsigned flags )
{
    const char* columnTypes[ numberOfColumns ];
    detail::forEachColumn< Columns >( detail::ColumnTypeInserter{ columnTypes } );

    std::size_t columnSizes[ numberOfColumns ];
    detail::forEachColumn< Columns >( detail::ColumnSizeInserter{ columnSizes } );

    const char* columnConstraints[ numberOfColumns ];
    detail::forEachColumn< Columns >( detail::ColumnConstraintInserter{ columnConstraints } );

    if ( flags & DROP_TABLE_IF_EXISTS )
    {
        detail::drop( conn_, name_, true );
    }

    detail::create(
        conn_,
        name_,
        columnNames_.data(), columnNames_.size(),
        columnTypes, columnSizes, columnConstraints,
        { PrimaryKey... },
        flags & TEMPORARY_TABLE
    );
}

template< typename Columns, std::size_t... PrimaryKey >
inline void Table< Columns, PrimaryKey... >::drop()
{
    detail::drop( conn_, name_, false );
}

template< typename Columns, std::size_t... PrimaryKey >
inline boost::optional< Columns > Table< Columns, PrimaryKey... >::select( const ColumnAt< PrimaryKey >&... primaryKey ) const
{
    auto& stmt = cache_.template lookUp< detail::StatementCacheEntryType::Select, Columns, Columns, PrimaryKey... >( conn_, [ this ]() { return detail::select( name_, columnNames_.data(), columnNames_.size(), { PrimaryKey... } ); } );

    stmt.params() = std::forward_as_tuple( primaryKey... );

    stmt.exec();

    if ( !stmt.fetch() )
    {
        return boost::none;
    }

    return stmt.cols();
}

template< typename Columns, std::size_t... PrimaryKey >
inline ResultSet< Columns > Table< Columns, PrimaryKey... >::selectAll() const
{
    return selectBy<>();
}

template< typename Columns, std::size_t... PrimaryKey >
template< std::size_t... Key >
inline ResultSet< Columns > Table< Columns, PrimaryKey... >::selectBy( const ColumnAt< Key >&... key ) const
{
    auto& stmt = cache_.template lookUp< detail::StatementCacheEntryType::Select, Columns, Columns, Key... >( conn_, [ this ]() { return detail::select( name_, columnNames_.data(), columnNames_.size(), { Key... } ); } );

    stmt.params() = std::forward_as_tuple( key... );

    return ResultSet< Columns >{ stmt };
}

template< typename Columns, std::size_t... PrimaryKey >
inline void Table< Columns, PrimaryKey... >::insert( const Columns& row )
{
    insertAt( row, MakeIndexSequence< numberOfColumns >{} );
}

template< typename Columns, std::size_t... PrimaryKey >
template< std::size_t... Value >
inline void Table< Columns, PrimaryKey... >::insertAt( const Columns& row, const IndexSequence< Value... >& )
{
    auto& stmt = cache_.template lookUp< detail::StatementCacheEntryType::Insert, Columns, std::tuple<>, Value... >( conn_, [ this ]() { return detail::insert( name_, columnNames_.data(), { Value... } ); } );

    stmt.params() = std::forward_as_tuple( std::get< Value >( row )... );

    stmt.exec();
}

template< typename Columns, std::size_t... PrimaryKey >
inline void Table< Columns, PrimaryKey... >::update( const Columns& row )
{
    updateBy( row, IndexSequence< PrimaryKey... >{} );
}

template< typename Columns, std::size_t... PrimaryKey >
template< std::size_t... Key >
inline void Table< Columns, PrimaryKey... >::updateBy( const Columns& row, const IndexSequence< Key... >& key )
{
    updateAtBy( row, MakeIndexSequence< numberOfColumns >{}, key );
}

template< typename Columns, std::size_t... PrimaryKey >
template< std::size_t... Value >
inline void Table< Columns, PrimaryKey... >::updateAt( const Columns& row, const IndexSequence< Value... >& value )
{
    updateAtBy( row, value, IndexSequence< PrimaryKey... >{} );
}

template< typename Columns, std::size_t... PrimaryKey >
template< std::size_t... Value, std::size_t... Key >
inline void Table< Columns, PrimaryKey... >::updateAtBy( const Columns& row, const IndexSequence< Value... >&, const IndexSequence< Key... >& )
{
    auto& stmt = cache_.template lookUp< detail::StatementCacheEntryType::Update, std::tuple< Columns, Columns >, std::tuple<>, Value..., (numberOfColumns + Key)... >( conn_, [ this ]() { return detail::update( name_, columnNames_.data(), { Value... }, { Key... } ); } );

    stmt.params() = std::forward_as_tuple( std::get< Value >( row )..., std::get< Key >( row )... );

    stmt.exec();
}

template< typename Columns, std::size_t... PrimaryKey >
inline void Table< Columns, PrimaryKey... >::delete_( const ColumnAt< PrimaryKey >&... primaryKey )
{
    deleteBy< PrimaryKey... >( primaryKey... );
}

template< typename Columns, std::size_t... PrimaryKey >
inline void Table< Columns, PrimaryKey... >::deleteAll()
{
    deleteBy<>();
}

template< typename Columns, std::size_t... PrimaryKey >
template< std::size_t... Key >
inline void Table< Columns, PrimaryKey... >::deleteBy( const ColumnAt< Key >&... key )
{
    auto& stmt = cache_.template lookUp< detail::StatementCacheEntryType::Delete, Columns, std::tuple<>, Key... >( conn_, [ this ]() { return detail::delete_( name_, columnNames_.data(), { Key... } ); } );

    stmt.params() = std::forward_as_tuple( key... );

    stmt.exec();
}

template< typename Columns, std::size_t... PrimaryKey >
inline CreateTable< Columns, PrimaryKey... >::CreateTable( Connection& conn, const std::string& name, const typename Table::ColumnNames& columnNames, const unsigned flags )
{
    Table{ conn, name, columnNames }.create( flags );
}

}
