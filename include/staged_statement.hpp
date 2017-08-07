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
#include "typed_statement.hpp"

namespace rodbc
{

/**
 * @brief The StagedStatement class template
 */
template< typename StagedParams, typename Params, typename Cols >
class StagedStatement
{
public:
    using StagingColumns = typename CreateTable< std::pair< std::int32_t, StagedParams >, 0 >::ColumnNames;

    StagedStatement( Connection& conn, const char* const stagingTable, const StagingColumns& stagingColumns, const char* const stmt );

    void resizeStagedParams( const std::int32_t size );
    StagedParams& stagedParams( const std::int32_t index );

    Params& params();
    const Cols& cols() const;

public:
    void exec();
    bool fetch();

private:
    CreateTable< std::pair< std::int32_t, StagedParams >, 0 > createStagingTable_;
    TypedStatement< std::tuple<>, std::tuple<> > deleteFromStagingTable_;
    TypedStatement< std::vector< std::pair< std::int32_t, StagedParams > >, std::tuple<> > insertIntoStagingTable_;
    TypedStatement< Params, Cols > stmt_;
};

}
