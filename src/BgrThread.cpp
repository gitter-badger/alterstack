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

#include "alterstack/AtomicGuard.hpp"
#include "alterstack/BgrThread.hpp"
#include "alterstack/BgRunner.hpp"
#include "alterstack/Scheduler.hpp"
#include "alterstack/Task.hpp"
#include "alterstack/Logger.hpp"

namespace alterstack
{

void BgrThread::thread_function()
{
    m_thread_started.store(true, ::std::memory_order_release);
    AtomicReturnBoolGuard thread_stopped_guard(m_thread_stopped);
    Scheduler::create_native_task();
    Scheduler::m_thread_info->native_runner = false;
    LOG << "BgrThread::thread_function: started\n";

    while( true )
    {
        Task* next_task = Scheduler::get_next_from_queue();
        if( next_task != nullptr )
        {
            LOG << "BgrThread::thread_function: got new task, switching on " << next_task << "\n";
            Scheduler::switch_to(next_task);
        }

        ::std::unique_lock<std::mutex> queue_guard(Scheduler::m_queue_mutex);
        if(  __builtin_expect( m_bg_runner.m_stop, false ) )
        {
            return;
        }
        LOG << "BgrThread::thread_function: waiting...\n";
        Scheduler::m_task_ready.wait(queue_guard);
        LOG << "BgrThread::thread_function: waked up\n";
        if(  __builtin_expect( m_bg_runner.m_stop, false ) )
        {
            return;
        }
        queue_guard.unlock();
    }
}

BgrThread::BgrThread(BgRunner &bg_runner)
    :m_bg_runner(bg_runner)
{
    LOG << "BgrThread::BgrThread\n";
    m_thread_started.store(false, ::std::memory_order_relaxed);
    m_thread_stopped.store(false, ::std::memory_order_release);
    m_thread = ::std::thread(&BgrThread::thread_function, this);
}

BgrThread::~BgrThread()
{
    m_bg_runner.m_stop = true; // FIXME: m_stop is reference to BgRunner's m_stop member
    Scheduler::m_task_ready.notify_all();
    while(!m_thread_started.load())
    {
        LOG << "BgrThread::~BgrThread(): waiting thread_function to start\n";
        std::this_thread::yield();
    }
    while( !m_thread_stopped.load() )
    {
        LOG << "BgrThread::~BgrThread(): waiting thread_function to stop\n";
        std::this_thread::sleep_for(::std::chrono::microseconds(1));
        Scheduler::m_task_ready.notify_all();
    }
    m_thread.join();
    LOG << "Processor finished\n";
}

}
