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

namespace rodbc
{

/**
 * @brief The StagedStatement class template
 */
template< typename StagedParams, typename Params, typename Cols, typename StagingIndex = std::int32_t >
class StagedStatement
{
private:
    using StagingTable = Table< std::tuple< StagingIndex, StagedParams >, 0 >;
    using StagingColumns = typename StagingTable::Columns;
    using StagingColumnNames = typename StagingTable::ColumnNames;
    using CreateStagingTable = typename StagingTable::Create;

public:
    StagedStatement( Connection& conn, const char* const stagingTableName, const StagingColumnNames& stagingColumnNames, const char* const stmt );

    void resizeStagedParams( const StagingIndex size );
    StagedParams& stagedParams( const StagingIndex index );

    Params& params();
    const Cols& cols() const;

public:
    void exec();
    bool fetch();

private:
    CreateStagingTable createStagingTable_;
    TypedStatement< std::tuple<>, std::tuple<> > deleteFromStagingTable_;
    TypedStatement< std::vector< StagingColumns >, std::tuple<> > insertIntoStagingTable_;
    TypedStatement< Params, Cols > stmt_;
};

}
