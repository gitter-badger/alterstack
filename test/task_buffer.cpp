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
#include <algorithm>
#include <iostream>
#include <vector>
#include <set>

#include <catch.hpp>

#include "alterstack/task_buffer.hpp"

namespace alterstack
{
class Task
{
private:
    friend class TaskBuffer<Task>;
    friend class UnitTestAccessor;
    Task* next_ = nullptr;
};

class UnitTestAccessor
{
public:
    static alterstack::Task* get_next(alterstack::Task* task)
    {
        return task->next_;
    }
};
}

using alterstack::TaskBuffer;
using alterstack::Task;
TEST_CASE("API check")
{
    TaskBuffer<Task> buffer;
    SECTION( "TaskBuffer fits in 64 bytes" ) {
        REQUIRE( sizeof(TaskBuffer<Task>) <= 64 );
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
        SECTION( "and after that returns nullptr" )
        {
            for(int i = 0; i < 100; ++i )
            {
                REQUIRE( buffer.get_task() == nullptr );
            }
        }
    }
    SECTION( "get_task returns single Task*" )
    {
        alterstack::Task task;
        buffer.put_task(&task);
        alterstack::Task* got_task = buffer.get_task();
        REQUIRE( alterstack::UnitTestAccessor::get_next(got_task) == nullptr );
    }
    constexpr int TASKS_COUNT = 100;
    std::vector<alterstack::Task> tasks(TASKS_COUNT);
    SECTION( "many get_task return the same set of tasks, that put_task store" )
    {
        for(auto& task: tasks)
        {
            buffer.put_task(&task);
        }
        alterstack::Task* task;
        std::set<alterstack::Task*> task_set;
        while( (task = buffer.get_task()) != nullptr )
        {
            REQUIRE( alterstack::UnitTestAccessor::get_next(task) == nullptr );
            task_set.insert(task);
        }
        REQUIRE( task_set.size() == TASKS_COUNT );
        for(auto& task: tasks)
        {
            REQUIRE( task_set.find(&task) != task_set.end() );
        }
    }
}
