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

#include "staged_statement.hpp"

#include "table.ipp"
#include "typed_statement.ipp"

namespace rodbc
{

template< typename StagedParams, typename Params, typename Cols, typename StagingIndex >
inline StagedStatement< StagedParams, Params, Cols, StagingIndex >::StagedStatement(
    Connection& conn,
    const std::string& stagingTableName, const StagingTableColumnNames& stagingTableColumnNames,
    const char* const stmt
)
: createStagingTable_{ conn, stagingTableName, stagingTableColumnNames, DROP_TABLE_IF_EXISTS | TEMPORARY_TABLE }
, deleteFromStagingTable_{ conn, detail::deleteAll( stagingTableName ).c_str() }
, insertIntoStagingTable_{ conn, detail::insert( stagingTableName, stagingTableColumnNames.data(), stagingTableColumnNames.size() ).c_str() }
, stmt_{ conn, stmt }
{
}

template< typename StagedParams, typename Params, typename Cols, typename StagingIndex >
inline void StagedStatement< StagedParams, Params, Cols, StagingIndex >::resizeStagedParams( const StagingIndex size )
{
    auto& params = insertIntoStagingTable_.params();

    StagingIndex index = params.size();

    params.resize( size );

    for ( ; index < size; ++index )
    {
        std::get< 0 >( params[ index ] ) = index;
    }
}

template< typename StagedParams, typename Params, typename Cols, typename StagingIndex >
inline StagedParams& StagedStatement< StagedParams, Params, Cols, StagingIndex >::stagedParams( const StagingIndex index )
{
    return std::get< 1 >( insertIntoStagingTable_.params()[ index ] );
}

template< typename StagedParams, typename Params, typename Cols, typename StagingIndex >
inline Params& StagedStatement< StagedParams, Params, Cols, StagingIndex >::params()
{
    return stmt_.params();
}

template< typename StagedParams, typename Params, typename Cols, typename StagingIndex >
inline const Cols& StagedStatement< StagedParams, Params, Cols, StagingIndex >::cols() const
{
    return stmt_.cols();
}

template< typename StagedParams, typename Params, typename Cols, typename StagingIndex >
inline void StagedStatement< StagedParams, Params, Cols, StagingIndex >::exec()
{
    deleteFromStagingTable_.exec();
    insertIntoStagingTable_.exec();

    stmt_.exec();
}

template< typename StagedParams, typename Params, typename Cols, typename StagingIndex >
inline bool StagedStatement< StagedParams, Params, Cols, StagingIndex >::fetch()
{
    return stmt_.fetch();
}

}
