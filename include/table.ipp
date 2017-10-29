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
    const std::initializer_list< std::size_t >& primaryKey
);

std::string selectAll(
    const std::string& tableName,
    const std::string* const columnNames, const std::size_t numberOfColumns
);

std::string insert(
    const std::string& tableName,
    const std::string* const columnNames, const std::size_t numberOfColumns
);

std::string update(
    const std::string& tableName,
    const std::string* const columnNames, const std::size_t numberOfColumns,
    const std::initializer_list< std::size_t >& primaryKey
);

std::string delete_(
    const std::string& tableName,
    const std::string* const columnNames,
    const std::initializer_list< std::size_t >& primaryKey
);

std::string deleteAll(
    const std::string& tableName
);

}

template< typename Columns, std::size_t... PrimaryKey >
template< typename... Values >
inline Table< Columns, PrimaryKey... >::ColumnNames::ColumnNames( Values&&... values )
: std::array< std::string, detail::sizeOfColumns< Columns >() >{ std::forward< Values >( values )... }
{
    static_assert( detail::sizeOfColumns< Columns >() == sizeof... ( Values ), "Number of columns and column names must be equal." );
}

template< typename Columns, std::size_t... PrimaryKey >
inline Table< Columns, PrimaryKey... >::Table( Connection& conn, std::string name, ColumnNames columnNames )
: conn_{ conn }
, name_{ std::move( name ) }
, columnNames_{ std::move( columnNames ) }
{
}

template< typename Columns, std::size_t... PrimaryKey >
inline void Table< Columns, PrimaryKey... >::create( const unsigned flags )
{
    const auto columns = detail::sizeOfColumns< Columns >();

    const char* columnTypes[ columns ];
    detail::forEachColumn< Columns >( detail::ColumnTypeInserter{ columnTypes } );

    std::size_t columnSizes[ columns ];
    detail::forEachColumn< Columns >( detail::ColumnSizeInserter{ columnSizes } );

    const char* columnConstraints[ columns ];
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
    if ( !select_ )
    {
        select_.emplace( conn_, detail::select( name_, columnNames_.data(), columnNames_.size(), { PrimaryKey... } ).c_str() );
    }

    select_->params() = std::forward_as_tuple( primaryKey... );

    select_->exec();

    if ( !select_->fetch() )
    {
        return boost::none;
    }

    return select_->cols();
}

template< typename Columns, std::size_t... PrimaryKey >
inline std::vector< Columns > Table< Columns, PrimaryKey... >::selectAll() const
{
    std::vector< Columns > rows;

    if ( !selectAll_ )
    {
        selectAll_.emplace( conn_, detail::selectAll( name_, columnNames_.data(), columnNames_.size() ).c_str() );
    }

    selectAll_->exec();

    while ( selectAll_->fetch() )
    {
        rows.push_back( selectAll_->cols() );
    }

    return rows;
}

template< typename Columns, std::size_t... PrimaryKey >
inline void Table< Columns, PrimaryKey... >::insert( const Columns& row )
{
    if ( !insert_ )
    {
        insert_.emplace( conn_, detail::insert( name_, columnNames_.data(), columnNames_.size() ).c_str() );
    }

    insert_->params() = row;

    insert_->exec();
}

template< typename Columns, std::size_t... PrimaryKey >
inline void Table< Columns, PrimaryKey... >::update( const Columns& row )
{
    update_( row, std::get< PrimaryKey >( row )... );
}

template< typename Columns, std::size_t... PrimaryKey >
inline void Table< Columns, PrimaryKey... >::update_( const Columns& row, const ColumnAt< PrimaryKey >&... primaryKey )
{
    if ( !update__ )
    {
        update__.emplace( conn_, detail::update( name_, columnNames_.data(), columnNames_.size(), { PrimaryKey... } ).c_str() );
    }

    update__->params() = std::forward_as_tuple( row, primaryKey... );

    update__->exec();
}

template< typename Columns, std::size_t... PrimaryKey >
inline void Table< Columns, PrimaryKey... >::delete_( const ColumnAt< PrimaryKey >&... primaryKey )
{
    if ( !delete__ )
    {
        delete__.emplace( conn_, detail::delete_( name_, columnNames_.data(), { PrimaryKey... } ).c_str() );
    }

    delete__->params() = std::forward_as_tuple( primaryKey... );

    delete__->exec();
}

template< typename Columns, std::size_t... PrimaryKey >
inline void Table< Columns, PrimaryKey... >::deleteAll()
{
    if ( !deleteAll_ )
    {
        deleteAll_.emplace( conn_, detail::deleteAll( name_, columnNames_.data(), { PrimaryKey... } ).c_str() );
    }

    deleteAll_->exec();
}

template< typename Columns, std::size_t... PrimaryKey >
inline CreateTable< Columns, PrimaryKey... >::CreateTable( Connection& conn, const std::string& name, const typename Table::ColumnNames& columnNames, const unsigned flags )
{
    Table{ conn, name, columnNames }.create( flags );
}

}
