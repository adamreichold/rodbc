#pragma once

#include "create_table.hpp"
#include "typed_statement.hpp"

#include <boost/fusion/include/std_pair.hpp>

namespace rodbc
{

/**
 * @brief The StagedStatement class template
 */
template< typename StagedParams, typename Params, typename Cols >
class StagedStatement
{
public:
    StagedStatement( Connection& conn, const char* const stagingTable, const std::vector< const char* >& stagingColumns, const char* const stmt );

    void resizeStagedParams( const std::int32_t size );
    StagedParams& stagedParams( const std::int32_t index );

    Params& params();
    const Cols& cols() const;

public:
    void exec();
    bool fetch();

private:
    CreateTable< std::pair< std::int32_t, StagedParams > > createStagingTable_;
    TypedStatement< std::tuple<>, std::tuple<> > deleteFromStagingTable_;
    TypedStatement< std::vector< std::pair< std::int32_t, StagedParams > >, std::tuple<> > insertIntoStagingTable_;
    TypedStatement< Params, Cols > stmt_;
};

namespace detail
{

std::string deleteFrom( const char* const tableName );
std::string insertInto( const char* const tableName, const std::vector< const char* >& columnNames );

}

template< typename StagedParams, typename Params, typename Cols >
inline StagedStatement< StagedParams, Params, Cols >::StagedStatement( Connection& conn, const char* const stagingTable, const std::vector< const char* >& stagingColumns, const char* const stmt )
: createStagingTable_{ conn, stagingTable, stagingColumns, { 0 }, DROP_TABLE_IF_EXISTS | TEMPORARY_TABLE }
, deleteFromStagingTable_{ conn, detail::deleteFrom( stagingTable ).c_str() }
, insertIntoStagingTable_{ conn, detail::insertInto( stagingTable, stagingColumns ).c_str() }
, stmt_{ conn, stmt }
{
}

template< typename StagedParams, typename Params, typename Cols >
inline void StagedStatement< StagedParams, Params, Cols >::resizeStagedParams( const std::int32_t size )
{
    insertIntoStagingTable_.params().resize( size );
}

template< typename StagedParams, typename Params, typename Cols >
inline StagedParams& StagedStatement< StagedParams, Params, Cols >::stagedParams( const std::int32_t index )
{
    return insertIntoStagingTable_.params()[ index ].second;
}

template< typename StagedParams, typename Params, typename Cols >
inline Params& StagedStatement< StagedParams, Params, Cols >::params()
{
    return stmt_.params();
}

template< typename StagedParams, typename Params, typename Cols >
inline const Cols& StagedStatement< StagedParams, Params, Cols >::cols() const
{
    return stmt_.cols();
}

template< typename StagedParams, typename Params, typename Cols >
inline void StagedStatement< StagedParams, Params, Cols >::exec()
{
    auto& stagedParams = insertIntoStagingTable_.params();

    for ( std::size_t index = 0; index < stagedParams.size(); ++index )
    {
        stagedParams[ index ].first = index;
    }

    deleteFromStagingTable_.exec();
    insertIntoStagingTable_.exec();

    stmt_.exec();
}

template< typename StagedParams, typename Params, typename Cols >
inline bool StagedStatement< StagedParams, Params, Cols >::fetch()
{
    return stmt_.fetch();
}

}
