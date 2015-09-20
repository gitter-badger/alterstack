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

#include <atomic>

namespace alterstack
{

class Task;
/**
 * @brief Lockfree Task queue
 *
 * TaskQueue implemented as lockfree pop-less stack. You can push one Task*,
 * list of tasks or get whole stored tasklist.
 */
class TaskQueue
{


public:
    TaskQueue() noexcept;
    TaskQueue(const TaskQueue&) = delete;
    TaskQueue(TaskQueue&&)      = delete;

    TaskQueue& operator=(const TaskQueue&) = delete;
    TaskQueue& operator=(TaskQueue&&)      = delete;

    /**
     * @brief Atomicaly push single Task in stack (queue)
     * @param task Task* to push
     */
    void  push_one(Task* task) noexcept;
    /**
     * @brief Atomicaly push Task list in stack (queue)
     * @param task_list head of task list to push
     */
    void  push_many(Task* task_list) noexcept;
    /**
     * @brief Atomically get stored task list and store empty list (nullptr)
     * @return head of task list
     */
    Task* get_all() noexcept;
private:
    ::std::atomic<Task*> m_head;
};

inline TaskQueue::TaskQueue() noexcept
{
    m_head.store(nullptr,::std::memory_order_release);
}

inline Task *TaskQueue::get_all() noexcept
{
    return m_head.exchange(nullptr, ::std::memory_order_acq_rel);
}

}
