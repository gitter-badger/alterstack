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

#include <assert.h>

#include <algorithm>
#include <cstdint>
#include <atomic>

class Task
{
private:
    friend class TaskBuffer;
    Task* next_;
};

class alignas(64) TaskBuffer
{
public:
    TaskBuffer();
    Task* get_task();
    void put_task(Task* task);
private:
    bool store_in_empty_slot(uint32_t &pos, Task *task);
    void store_in_occuped_slot(uint32_t pos, Task *task_list);
    void store_tail(Task* task_list);

    static constexpr uint32_t buffer_size_ = 4;
    static constexpr uint32_t buffer_mask_ = buffer_size_ - 1;
    std::atomic<uint32_t> get_position_;
    std::atomic<uint32_t> put_position_;
    std::atomic<Task*>    buffer_[buffer_size_];
};

TaskBuffer::TaskBuffer()
{
    assert((buffer_size_ >= 2) && ((buffer_size_ & (buffer_size_ - 1)) == 0));
    std::fill(std::begin(buffer_), std::end(buffer_), nullptr);
    get_position_.store(0, std::memory_order_relaxed);
    put_position_.store(0, std::memory_order_release);
}

inline Task* TaskBuffer::get_task()
{
    Task* task;
    uint32_t pos = get_position_.load(std::memory_order_acquire);
    uint32_t i = 0;
    while( i < buffer_size_ )
    {
        task = buffer_[(pos + i) & buffer_mask_].exchange(
                    nullptr, std::memory_order_relaxed);
        if( task != nullptr )
        {
            get_position_.fetch_add(1, std::memory_order_relaxed);
            // FIXME: here I need to store task->next back in buffer if not nullptr
            if( task->next_ != nullptr )
            {
                Task* tail = task->next_;
                store_tail(tail);
                task->next_ = nullptr;
            }
            break;
        }
        uint32_t pos_next = get_position_.load(std::memory_order_acquire);
        if( pos != pos_next )
        {
            pos = pos_next;
            i = 0;
        }
        else
        {
            ++i;
        }
    }
    return task;
}

bool TaskBuffer::store_in_empty_slot(uint32_t& pos, Task *task)
{
    uint32_t i = 0;
    while( i < buffer_size_ )
    {
        Task* expected = nullptr;
        bool change_done = buffer_[(pos + i) & buffer_mask_].compare_exchange_strong(
                    expected, task
                    ,std::memory_order_relaxed, std::memory_order_relaxed);
        if( change_done )
        {
            put_position_.fetch_add(1, std::memory_order_relaxed);
            return true;
        }
        uint32_t pos_next = put_position_.load(std::memory_order_acquire);
        if( pos != pos_next )
        {
            pos = pos_next;
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
        uint32_t pos = put_position_.load(std::memory_order_acquire);
        if( !store_in_empty_slot(pos, task) )
        {
            task->next_ = task_list;
        }
        store_in_occuped_slot(pos, task);
    }
}

inline void TaskBuffer::store_in_occuped_slot(uint32_t pos, Task *task_list)
{
    Task* last_task = task_list;
    while( last_task->next_ != nullptr )
    {
        last_task = last_task->next_;
    }
    Task** last_next = &last_task->next_;

    Task* stored = buffer_[pos & buffer_mask_].load(std::memory_order_acquire);
    *last_next = stored;
    while( buffer_[pos & buffer_mask_].compare_exchange_weak(
               stored, task_list
               ,std::memory_order_release, std::memory_order_relaxed) )
    {
        *last_next = stored;
    }
}

inline void TaskBuffer::put_task(Task *task)
{
    uint32_t pos = put_position_.load(std::memory_order_acquire);
    if( store_in_empty_slot(pos, task) )
    {
        return;
    }
    store_in_occuped_slot(pos, task);
}
