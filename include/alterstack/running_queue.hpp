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

#pragma once

#include "task_buffer.hpp"
#include "task_stack.hpp"

namespace alterstack
{
/**
 * @brief Task Queue ready to run.
 *
 * Task* get_task() noexcept; dequeue single Task* from queue if exists
 * or return nullptr
 *
 * void put_task(Task* task) noexcept; enqueue single Task* in queue
 */
class RunningQueue
{
public:
    //RunningTaskQueue() noexcept;
    /**
     * @brief dequeue Task* or nullptr
     * @return single Task* (not list) or nullptr if no Task* in queue
     */
    Task* get_task() noexcept;
    /**
     * @brief enqueue single Task* in queue
     *
     * @param task Task* to store
     */
    void put_task(Task* task) noexcept;

    private:
    TaskBuffer task_buffer_;
    TaskStack  task_stack_;
};

Task *RunningQueue::get_task() noexcept
{
    Task* task = task_buffer_.get_task();
    if( task != nullptr )
    {
        return task;
    }
    Task* task_list = task_stack_.pop_all();
    if( task_list != nullptr )
    {
        task = task_list;
        task_list = task_list->next_;
        task->next_ = nullptr;
        if( task_list != nullptr )
        {
            task_buffer_.put_task(task_list);
        }
    }
    return task;
}

inline void RunningQueue::put_task(Task *task) noexcept
{
    if( task_stack_.push(task) )
    {
        // FIXME: notify BgRunner here (first task in stack)
    }
}

}
