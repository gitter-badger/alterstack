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

#include "alterstack/running_queue.hpp"

namespace alterstack
{
class Task
{
//private:
public:
    friend class TaskBuffer<Task>;
    friend class TaskStack<Task>;
    friend class RunningQueue<Task>;
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

using alterstack::Task;
using alterstack::RunningQueue;

TEST_CASE("API check")
{
    RunningQueue<Task> queue;
    bool have_more_tasks = false;
    SECTION("RunningQueue aligned 64 bytes")
    {
        REQUIRE( (unsigned long long)(&queue) % 64 == 0);
    }
    SECTION( "empty RunningQueue returns nullptr" )
    {
        for(int i = 0; i < 16; ++i )
        {
            REQUIRE( queue.get_task(have_more_tasks) == nullptr );
        }
    }
    SECTION( "filled by one element returns it" )
    {
        alterstack::Task task;
        queue.put_task(&task);
        REQUIRE( queue.get_task(have_more_tasks) == &task );
        SECTION( "and after that returns nullptr" )
        {
            for(int i = 0; i < 100; ++i )
            {
                REQUIRE( queue.get_task(have_more_tasks) == nullptr );
            }
        }
    }
    SECTION( "get_task returns single Task*" )
    {
        alterstack::Task task;
        queue.put_task(&task);
        alterstack::Task* got_task = queue.get_task(have_more_tasks);
        REQUIRE( alterstack::UnitTestAccessor::get_next(got_task) == nullptr );
    }
    constexpr int TASKS_COUNT = 100;
    std::vector<alterstack::Task> tasks(TASKS_COUNT);
    SECTION( "many get_task return the same set of tasks, that put_task store" )
    {
        for(auto& task: tasks)
        {
            queue.put_task(&task);
        }
        alterstack::Task* task;
        std::set<alterstack::Task*> task_set;
        while( (task = queue.get_task(have_more_tasks)) != nullptr )
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
    SECTION( "get_task will not set have_more flag with one Task*" )
    {
        alterstack::Task task;
        queue.put_task(&task);
        have_more_tasks = false;
        alterstack::Task* got_task = queue.get_task(have_more_tasks);
        REQUIRE( have_more_tasks == false );
    }
    SECTION( "get_task will set have_more flag with two Task*s" )
    {
        alterstack::Task task1;
        queue.put_task(&task1);
        alterstack::Task task2;
        queue.put_task(&task2);
        have_more_tasks = false;

        queue.get_task(have_more_tasks);
        REQUIRE( have_more_tasks == true );
        have_more_tasks = false;
        queue.get_task(have_more_tasks);
        REQUIRE( have_more_tasks == false );
    }
}
