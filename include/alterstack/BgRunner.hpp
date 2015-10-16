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

#include "CpuCore.hpp"

#include <atomic>
#include <deque>
#include <memory>
#include <thread>

namespace alterstack
{
/**
 * @brief Creates pool of threads which runs Task in background
 *
 * Number of threads to create defined in constructor and can't change in runtime.
 */
class BgRunner
{
public:
    BgRunner() = delete;
    /**
     * @brief thread pool constructor
     * @param running number of threads to start
     */
    BgRunner(uint32_t running=1);
    ~BgRunner();

private:
    friend class CpuCore;
    friend class Scheduler;
    /**
     * @brief wake up all sleeping CpuCore's
     */
    void notify_all();
    /**
     * @brief notify BgRunner, that there is more Task s in RunningQueue
     *
     * If some CpuCore is sleeping, one will be woked up
     */
    void notify();

    ::std::deque<std::unique_ptr<CpuCore>> m_cpu_core_list;
};

}

