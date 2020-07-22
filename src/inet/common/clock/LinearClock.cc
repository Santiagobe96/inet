//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "inet/common/clock/LinearClock.h"

namespace inet {

Define_Module(LinearClock);

void LinearClock::initialize()
{
    origin.simtime = simTime();
    simtime_t clock = par("timeShift");
    origin.clocktime = ClockTime::from(simTime() + clock);
    driftRate = par("driftRate").doubleValue() / 1e6;
}

clocktime_t LinearClock::fromSimTime(simtime_t t) const
{
    return origin.clocktime + ClockTime::from((t - origin.simtime) * (1.0 + driftRate));
}

simtime_t LinearClock::toSimTime(clocktime_t clock) const
{
    return origin.simtime + (clock - origin.clocktime).asSimTime() / (1.0 + driftRate);
}

} // namespace inet

