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

namespace alterstack
{
class BgRunner;
/**
 * @brief Single Task background runner thread.
 */
class CpuCore
{
public:
    CpuCore() = delete;
    /**
     * @brief start one OS thread
     * @param bg_runner reference to BgRunner
     */
    CpuCore(BgRunner& bg_runner);
    /**
     * @brief destructor stops OS thread, return when it's stopped
     */
    ~CpuCore();

private:
    void thread_function();

    BgRunner&         m_bg_runner;
    std::thread       m_thread;
    std::atomic<bool> m_thread_started;
    std::atomic<bool> m_thread_stopped;
};

}
