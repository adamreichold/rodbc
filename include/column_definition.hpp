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

#include "types.hpp"

namespace rodbc
{
namespace detail
{

template< typename Type >
struct ColumnDefinition;

template<>
struct ColumnDefinition< std::int8_t >
{
    static constexpr const char* type = "TINYINT";
    static constexpr std::size_t size = 0;
    static constexpr const char* constraint = "NOT NULL";
};

template<>
struct ColumnDefinition< std::int16_t >
{
    static constexpr const char* type = "SMALLINT";
    static constexpr std::size_t size = 0;
    static constexpr const char* constraint = "NOT NULL";
};

template<>
struct ColumnDefinition< std::int32_t >
{
    static constexpr const char* type = "INT";
    static constexpr std::size_t size = 0;
    static constexpr const char* constraint = "NOT NULL";
};

template<>
struct ColumnDefinition< std::int64_t >
{
    static constexpr const char* type = "BIGINT";
    static constexpr std::size_t size = 0;
    static constexpr const char* constraint = "NOT NULL";
};

template<>
struct ColumnDefinition< std::uint8_t >
{
    static constexpr const char* type = "TINYINT UNSIGNED";
    static constexpr std::size_t size = 0;
    static constexpr const char* constraint = "NOT NULL";
};

template<>
struct ColumnDefinition< std::uint16_t >
{
    static constexpr const char* type = "SMALLINT UNSIGNED";
    static constexpr std::size_t size = 0;
    static constexpr const char* constraint = "NOT NULL";
};

template<>
struct ColumnDefinition< std::uint32_t >
{
    static constexpr const char* type = "INT UNSIGNED";
    static constexpr std::size_t size = 0;
    static constexpr const char* constraint = "NOT NULL";
};

template<>
struct ColumnDefinition< std::uint64_t >
{
    static constexpr const char* type = "BIGINT UNSIGNED";
    static constexpr std::size_t size = 0;
    static constexpr const char* constraint = "NOT NULL";
};

template<>
struct ColumnDefinition< float >
{
    static constexpr const char* type = "REAL";
    static constexpr std::size_t size = 0;
    static constexpr const char* constraint = "NOT NULL";
};

template<>
struct ColumnDefinition< double >
{
    static constexpr const char* type = "DOUBLE PRECISION";
    static constexpr std::size_t size = 0;
    static constexpr const char* constraint = "NOT NULL";
};

template< std::size_t Size >
struct ColumnDefinition< String< Size > >
{
    static constexpr const char* type = "VARCHAR";
    static constexpr std::size_t size = Size;
    static constexpr const char* constraint = "NOT NULL";
};

template< std::size_t Size >
struct ColumnDefinition< Number< Size > >
{
    static constexpr const char* type = "NUMBER";
    static constexpr std::size_t size = Size;
    static constexpr const char* constraint = "NOT NULL";
};

template<>
struct ColumnDefinition< Timestamp >
{
    static constexpr const char* type = "TIMESTAMP";
    static constexpr std::size_t size = 0;
    static constexpr const char* constraint = "NOT NULL";
};

template< typename Type >
struct ColumnDefinition< Nullable< Type > >
{
    static constexpr const char* type = ColumnDefinition< Type >::type;
    static constexpr std::size_t size = ColumnDefinition< Type >::size;
    static constexpr const char* constraint = "NULL";
};

}
}
