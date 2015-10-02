/*
 * Copyright 2015 Alexey Syrnikov <san@masterspline.net>
 *
 * This file is part of Alterstack.
 *
 * Alterstack is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Alterstack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Alterstack.  If not, see <http://www.gnu.org/licenses/>
 */

#include <iostream>

#include <catch.hpp>

#include "alterstack/task_buffer.hpp"

using alterstack::TaskBuffer;
TEST_CASE("API check"/*,"[hide]"*/)
{
    TaskBuffer buffer;
    SECTION( "sizeof(TaskBuffer) fits 64 bytes" ) {
        REQUIRE( sizeof(TaskBuffer) <= 64 );
    }
    SECTION( "empty TaskBuffer returns nullptr" )
    {
        for(int i = 0; i < 16; ++i )
        {
            REQUIRE( buffer.get_task() == nullptr );
        }
    }
    SECTION( "filled by one element returns it" )
    {
        alterstack::Task task;
        buffer.put_task(&task);
        REQUIRE( buffer.get_task() == &task );
        for(int i = 0; i < 100; ++i )
        {
            REQUIRE( buffer.get_task() == nullptr );
        }
    }
}
