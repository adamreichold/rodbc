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

#include <vector>

namespace rodbc
{
namespace detail
{

template< typename Stmt, typename Cols >
class ResultSetIterator : public boost::iterator_facade< ResultSetIterator< Stmt, Cols >, const Cols, std::input_iterator_tag >
{
public:
    constexpr ResultSetIterator();
    ResultSetIterator( Stmt& stmt );

private:
    friend class boost::iterator_core_access;

    void increment();
    bool equal( const ResultSetIterator& other ) const;
    const Cols& dereference() const;

private:
    Stmt* stmt_;
};

template< typename Stmt, typename Cols >
class ResultSetIterator< Stmt, std::vector< Cols > > : public boost::iterator_facade< ResultSetIterator< Stmt, std::vector< Cols > >, const Cols, std::input_iterator_tag >
{
public:
    constexpr ResultSetIterator();
    ResultSetIterator( Stmt& stmt );

private:
    friend class boost::iterator_core_access;

    void increment();
    bool equal( const ResultSetIterator& other ) const;
    const Cols& dereference() const;

private:
    Stmt* stmt_;
    typename std::vector< Cols >::const_iterator it_;

    void fetch();
};

}

template< typename Stmt, typename Cols >
class ResultSet
{
public:
    ResultSet( Stmt& stmt );

    detail::ResultSetIterator< Stmt, Cols > begin();
    detail::ResultSetIterator< Stmt, Cols > end();

private:
    Stmt& stmt_;
};

template< typename Stmt >
auto resultsOf( Stmt& stmt ) -> ResultSet< Stmt, typename std::decay< decltype( stmt.cols() ) >::type >;

}
