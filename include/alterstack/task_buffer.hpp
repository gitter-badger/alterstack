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
    friend class TaskStack;
    friend class UnitTestAccessor;
    Task* next_ = nullptr;
};

/**
 * @brief Almost fifo lockfree buffer to hold Task* in RunningTaskQueue
 *
 * TaskBuffer is fifo-like lockfree buffer which hold Task* in RunningTaskQueue.
 * Task* order can change especially if stored Task* count exceeds buffer slots.
 * TaskBuffer uses intrusive Task* list and does not need any memory except
 * already allocated and stack for function local variables.
 *
 * Task* get_task() noexcept; return single Task* from buffer if exists
 * or return nullptr
 *
 * void put_task(Task* task) noexcept; always put Task* (single or list)
 * in buffer
 */
class alignas(64) TaskBuffer
{
public:
    TaskBuffer() noexcept;
    /**
     * @brief get Task* from TaskBuffer
     * @return single Task* (not list) or nullptr if no Task* in buffer
     */
    Task* get_task() noexcept;
    /**
     * @brief store Task* in buffer slot (empty or nonempty)
     *
     * put_task(Task* task) will store task in single slot, it will not
     * distribute it in different buffer slots even if task is list (this
     * will done in get_task() stage).
     * @param task Task* (single or list) to store
     */
    void put_task(Task* task) noexcept;
    Task *last_task_in_list(Task* old_list);

    private:
    /**
     * @brief store Task* (single or list) in any empty slot in buffer
     * @param task Task* to store
     * @return true if empty slot found and Task* stored there
     */
    bool store_in_empty_slot(Task *task) noexcept;
    /**
     * @brief store Task* (single or list) in slot poined by put_position_
     * @param task_list Task* list to store
     */
    void store_in_occupied_slot(Task *task_list) noexcept;
    /**
     * @brief distribute Task* list in empty slots first, and rest as list in occupied
     * @param task_list list to store
     */
    void store_tail(Task* task_list) noexcept;

    static constexpr uint32_t buffer_size_ = 7; // to fit in 64 bytes cache line
    std::atomic<uint32_t> get_position_;
    std::atomic<uint32_t> put_position_;
    std::atomic<Task*>    buffer_[buffer_size_];
};

TaskBuffer::TaskBuffer() noexcept
{
    std::fill(std::begin(buffer_), std::end(buffer_), nullptr);
    get_position_.store(0, std::memory_order_relaxed);
    put_position_.store(0, std::memory_order_release);
}

inline Task* TaskBuffer::get_task() noexcept
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
            get_position_.fetch_add(1+i, std::memory_order_relaxed);
            // store task->next back in buffer if not nullptr
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

bool TaskBuffer::store_in_empty_slot(Task *task) noexcept
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
            put_position_.fetch_add(1+i, std::memory_order_release);
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

inline void TaskBuffer::store_tail(Task *task_list) noexcept
{
    for( uint32_t i = 0; i < buffer_size_-1; ++i)
    {
        if( task_list == nullptr)
        {
            return;
        }
        Task* task = task_list;
        task_list = task_list->next_;
        task->next_ = nullptr;
        if( !store_in_empty_slot(task) )
        {
            task->next_ = task_list;
            store_in_occupied_slot(task);
            return;
        }
    }
    store_in_occupied_slot(task_list);
}

Task* TaskBuffer::last_task_in_list(Task* task_list)
{
    Task* last_task = task_list;
    while( last_task->next_ != nullptr )
    {
        last_task = last_task->next_;
    }
    return last_task;
}

inline void TaskBuffer::store_in_occupied_slot(Task *task_list) noexcept
{
    uint32_t index = put_position_.fetch_add(1, std::memory_order_release);
    Task* old_list = buffer_[index % buffer_size_].exchange(
                task_list, std::memory_order_relaxed);
    if( old_list == nullptr )
    {
        return;
    }

    Task** last_next_ptr = &last_task_in_list(old_list)->next_;

    *last_next_ptr = task_list;
    while( !buffer_[index % buffer_size_].compare_exchange_weak(
               task_list, old_list
               ,std::memory_order_release, std::memory_order_relaxed) )
    {
        *last_next_ptr = task_list;
    }
}

inline void TaskBuffer::put_task(Task *task) noexcept
{
    if( store_in_empty_slot(task) )
    {
        return;
    }
    store_in_occupied_slot(task);
}
}
