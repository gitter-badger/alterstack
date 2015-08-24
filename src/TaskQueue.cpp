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

#include "alterstack/TaskQueue.hpp"
#include "alterstack/Task.hpp"

namespace alterstack
{

void TaskQueue::push_one(Task *task) noexcept
{
    Task* tail = m_head.load(::std::memory_order_acquire);
    task->m_next = tail;
    while(!m_head.compare_exchange_weak(tail,task,::std::memory_order_release))
    {
        task->m_next = tail;
    }
}

void TaskQueue::push_many(Task *task_list) noexcept
{
    Task *tail = m_head.load(::std::memory_order_acquire);
    Task *last = task_list;
    while( last->m_next != nullptr )
    {
        last = last->m_next;
    }
    last->m_next = tail;
    while(!m_head.compare_exchange_weak(tail,task_list,::std::memory_order_release))
    {
        last->m_next = tail;
    }
}

}
