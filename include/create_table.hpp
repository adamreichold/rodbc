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

#include <types.hpp>

namespace rodbc
{

class Connection;

constexpr unsigned DROP_TABLE_IF_EXISTS = 1 << 0;
constexpr unsigned TEMPORARY_TABLE = 1 << 1;

/**
 * @brief The CreateTable struct template
 */
template< typename Columns >
struct CreateTable
{
    using ColumnNames = std::initializer_list< const char* >;
    using PrimaryKey = std::initializer_list< std::size_t >;

    CreateTable( Connection& conn, const char* const tableName, const ColumnNames& columnNames, const PrimaryKey& primaryKey = {}, const unsigned flags = 0 );
};

}
