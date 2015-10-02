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

#include <algorithm>
#include <cstdint>
#include <atomic>
namespace alterstack
{
class Task
{
private:
    friend class TaskBuffer;
    Task* next_ = nullptr;
};

/**
 * @brief Almost fifo lockfree buffer to hold Task* in RunningTaskQueue
 *
 * TaskBuffer is fifo like (Task* order can change) lockfree buffer which
 * hold Task* in RunningTaskQueue.
 */
class alignas(64) TaskBuffer
{
public:
    TaskBuffer();
    Task* get_task();
    void put_task(Task* task);
private:
    /**
     * @brief store Task* in any empty slot in buffer
     * @param task Task* to store
     * @return true if empty slot found and Task* stored there
     */
    bool store_in_empty_slot(Task *task);
    void store_in_occuped_slot(Task *task_list);
    void store_tail(Task* task_list);

    static constexpr uint32_t buffer_size_ = 7; // to fit in 64 bytes cache line
    std::atomic<uint32_t> get_position_;
    std::atomic<uint32_t> put_position_;
    std::atomic<Task*>    buffer_[buffer_size_];
};

TaskBuffer::TaskBuffer()
{
    std::fill(std::begin(buffer_), std::end(buffer_), nullptr);
    get_position_.store(0, std::memory_order_relaxed);
    put_position_.store(0, std::memory_order_release);
}

inline Task* TaskBuffer::get_task()
{
    Task* task;
    uint32_t index = get_position_.load(std::memory_order_acquire);
    uint32_t i = 0;
    while( i < buffer_size_ )
    {
        task = buffer_[(index + i) % buffer_size_].exchange(
                    nullptr, std::memory_order_relaxed);
        if( task != nullptr )
        {
            get_position_.fetch_add(1, std::memory_order_relaxed);
            // FIXME: here I need to store task->next back in buffer if not nullptr
            if( task->next_ != nullptr )
            {
                Task* task_tail = task->next_;
                store_tail(task_tail);
                task->next_ = nullptr;
            }
            break;
        }
        uint32_t next_index = get_position_.load(std::memory_order_acquire);
        if( index != next_index )
        {
            index = next_index;
            i = 0;
        }
        else
        {
            ++i;
        }
    }
    return task;
}

bool TaskBuffer::store_in_empty_slot(Task *task)
{
    uint32_t index = put_position_.load(std::memory_order_acquire);
    uint32_t i = 0;
    while( i < buffer_size_ )
    {
        Task* expected = nullptr;
        bool change_done = buffer_[(index + i) % buffer_size_].compare_exchange_strong(
                    expected, task
                    ,std::memory_order_relaxed, std::memory_order_relaxed);
        if( change_done )
        {
            put_position_.fetch_add(1, std::memory_order_release);
            return true;
        }
        uint32_t next_index = put_position_.load(std::memory_order_acquire);
        if( index != next_index )
        {
            index = next_index;
            i = 0;
        }
        else
        {
            ++i;
        }
    }
    return false;
}

inline void TaskBuffer::store_tail(Task *task_list)
{
    while( task_list != nullptr)
    {
        Task* task = task_list;
        task_list = task_list->next_;
        task->next_ = nullptr;
        if( !store_in_empty_slot(task) )
        {
            task->next_ = task_list;
            store_in_occuped_slot(task);
            break;
        }
    }
}

inline void TaskBuffer::store_in_occuped_slot(Task *task_list)
{
    Task* last_task = task_list;
    while( last_task->next_ != nullptr )
    {
        last_task = last_task->next_;
    }
    Task** last_next_ptr = &last_task->next_;

    uint32_t index = put_position_.load(std::memory_order_acquire);
    Task* stored = buffer_[index % buffer_size_].load(std::memory_order_acquire);
    *last_next_ptr = stored;
    while( !buffer_[index % buffer_size_].compare_exchange_weak(
               stored, task_list
               ,std::memory_order_release, std::memory_order_relaxed) )
    {
        *last_next_ptr = stored;
    }
    put_position_.fetch_add(1, std::memory_order_release);
}

inline void TaskBuffer::put_task(Task *task)
{
    if( store_in_empty_slot(task) )
    {
        return;
    }
    store_in_occuped_slot(task);
}
}
