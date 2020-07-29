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
    simtime_t initialClockTime = par("initialClockTime");
    originSimTime = simTime();
    originClockTick = initialClockTime / oscillator->getNominalTickLength();
    originClockTime = ClockTime::from(initialClockTime);
    originClockOffset = ClockTime::from(initialClockTime - originClockTick * oscillator->getNominalTickLength());
}

clocktime_t LinearClock::computeClockTimeFromSimTime(simtime_t t) const
{
    ASSERT(t >= simTime());
    return originClockTime + originClockOffset + ClockTime::from(oscillator->computeClockTicksForInterval(t - originSimTime) * oscillator->getNominalTickLength());
}

simtime_t LinearClock::computeSimTimeFromClockTime(clocktime_t t) const
{
    ASSERT(t >= getClockTime());
    return originSimTime + oscillator->computeIntervalForClockTicks(floor((t - originClockTime - originClockOffset).dbl() / oscillator->getNominalTickLength().dbl()));
}

} // namespace inet

