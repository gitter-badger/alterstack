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

#include "alterstack/BgRunner.hpp"

#include "alterstack/AtomicGuard.hpp"
#include "alterstack/Scheduler.hpp"
#include "alterstack/Task.hpp"
#include "alterstack/CpuCore.hpp"
#include "alterstack/Logger.hpp"

namespace alterstack
{

BgRunner::BgRunner(uint32_t running)
{
    LOG << "BgRunner::BgRunner()\n";
    for(uint32_t i = 0; i < running; ++i)
    {
        m_cpu_core_list.push_back(::std::make_unique<CpuCore>());
    }
}

BgRunner::~BgRunner()
{
    for( auto& core: m_cpu_core_list)
    {
        core->request_stop();
    }
    LOG << "BgRunner::~BgRunner()\n";
}

void BgRunner::wake_up_all()
{
    if( CpuCore::sleep_count() )
    {
        for( auto& core: m_cpu_core_list)
        {
            core->wake_up();
        }
    }
}

}
