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

#include <boost/iterator/iterator_facade.hpp>

namespace rodbc
{

template< typename Stmt, typename Cols >
class ResultSetIterator : public boost::iterator_facade< ResultSetIterator< Stmt, Cols >, const Cols, std::input_iterator_tag >
{
public:
    ResultSetIterator();
    ResultSetIterator( Stmt& stmt );

private:
    friend class boost::iterator_core_access;

    void increment();
    bool equal( const ResultSetIterator& other ) const;
    const Cols& dereference() const;

private:
    Stmt* stmt_;
};

template< typename Params, typename Cols >
class TypedStatement;

template< typename StagedParams, typename Params, typename Cols >
class StagedStatement;

}

namespace std
{

template< typename Params, typename Cols >
rodbc::ResultSetIterator< rodbc::TypedStatement< Params, Cols >, Cols > begin( rodbc::TypedStatement< Params, Cols >& );

template< typename Params, typename Cols >
rodbc::ResultSetIterator< rodbc::TypedStatement< Params, Cols >, Cols > end( rodbc::TypedStatement< Params, Cols >& );

template< typename StagedParams, typename Params, typename Cols >
rodbc::ResultSetIterator< rodbc::StagedStatement< StagedParams, Params, Cols >, Cols > begin( rodbc::StagedStatement< StagedParams, Params, Cols >& );

template< typename StagedParams, typename Params, typename Cols >
rodbc::ResultSetIterator< rodbc::StagedStatement< StagedParams, Params, Cols >, Cols > end( rodbc::StagedStatement< StagedParams, Params, Cols >& );

}
