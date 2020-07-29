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

#include "inet/clock/LinearClock.h"

namespace inet {

Define_Module(LinearClock);

void LinearClock::initialize()
{
    origin.simtime = simTime(); // 1000
    simtime_t timeShift = par("timeShift"); // 100 the simulation time when the clock's value is zero
    // TODO: is this actually false? the simulation time when the clock's value is zero
    origin.clocktime = ClockTime::from(simTime() + timeShift); // 1000 + 100 = 1100 -> when clock is 0 then simtime is 100?
    driftRate = par("driftRate").doubleValue() / 1e6; // THERE'S A DRIFT!
    // why not the following?
    origin.simtime = timeShift;
    origin.clocktime = 0;
}

clocktime_t LinearClock::computeClockTimeFromSimTime(simtime_t t) const
{
    return origin.clocktime + ClockTime::from((t - origin.simtime) * (1.0 + driftRate));
}

simtime_t LinearClock::computeSimTimeFromClockTime(clocktime_t clock) const
{
    return origin.simtime + (clock - origin.clocktime).asSimTime() / (1.0 + driftRate);
}

} // namespace inet

