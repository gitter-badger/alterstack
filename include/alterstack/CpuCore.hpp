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
#include <thread>
#include <mutex>
#include <condition_variable>

namespace alterstack
{
class BgRunner;
/**
 * @brief Single Task background runner thread.
 */
class CpuCore
{
public:
    CpuCore(const CpuCore&) = delete;
    CpuCore(CpuCore&&) = delete;
    CpuCore& operator=(const CpuCore&) = delete;
    CpuCore& operator=(CpuCore&&) = delete;
    /**
     * @brief start one OS thread
     * @param bg_runner reference to BgRunner
     */
    CpuCore();
    /**
     * @brief destructor stops OS thread, return when it's stopped
     */
    ~CpuCore();
    /**
     * @brief ask this CpuCore to stop
     *
     * do not wait for thread stop
     */
    void request_stop();
    /**
     * @brief stop thread function, returns when thread stopped
     */
    void stop_thread();
    void wake_up();

private:
    void thread_function();
    /**
     * @brief ensure that thread function started, wait until started
     */
    void ensure_thread_started();
    /**
     * @brief ensure that thread function stopped, wait until stop
     */
    void ensure_thread_stopped();
    bool is_stop_requested();

    std::thread       m_thread;         //!< OS thread
    std::atomic<bool> m_thread_started; //!< true if thread_function started
    std::atomic<bool> m_thread_stopped; //!< true if thread_function stopped
    bool              m_stop_requested; //!< true when current CpuCore need to stop
    ::std::mutex      m_task_avalable_mutex; //!< mutex to sleep on conditional_variable
    ::std::condition_variable m_task_avalable; //!< conditional_variable to wait on
};

inline void CpuCore::request_stop()
{
    m_stop_requested = true;
}

inline bool CpuCore::is_stop_requested()
{
    return  __builtin_expect( m_stop_requested, false );
}

}
