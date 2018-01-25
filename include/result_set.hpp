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

#include <functional>
#include <vector>

namespace rodbc
{
namespace detail
{

template< typename Cols >
class ResultSetIterator : public boost::iterator_facade< ResultSetIterator< Cols >, const Cols, std::input_iterator_tag >
{
public:
    constexpr ResultSetIterator();

    template< typename Stmt >
    ResultSetIterator( Stmt& stmt );

private:
    friend class boost::iterator_core_access;

    void increment();
    bool equal( const ResultSetIterator& other ) const;
    const Cols& dereference() const;

private:
    const Cols* cols_;
    const std::function< bool() > fetch_;
};

template< typename Cols >
class ResultSetIterator< std::vector< Cols > > : public boost::iterator_facade< ResultSetIterator< std::vector< Cols > >, const Cols, std::input_iterator_tag >
{
public:
    constexpr ResultSetIterator();

    template< typename Stmt >
    ResultSetIterator( Stmt& stmt );

private:
    friend class boost::iterator_core_access;

    void increment();
    bool equal( const ResultSetIterator& other ) const;
    const Cols& dereference() const;

private:
    const std::vector< Cols >* cols_;
    const std::function< bool() > fetch_;

    typename std::vector< Cols >::const_iterator it_;

    void fetch();
};

struct ExecStmt
{
    template< typename Stmt >
    ExecStmt( Stmt& stmt );
};

}

template< typename Cols >
class ResultSet : private detail::ExecStmt
{
public:
    template< typename Stmt >
    ResultSet( Stmt& stmt );

    using Iterator = detail::ResultSetIterator< Cols >;

    const Iterator& begin() const;
    constexpr Iterator end() const;

private:
    const Iterator begin_;
};

}
