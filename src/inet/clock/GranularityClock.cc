//
// Copyright (C) OpenSim Ltd.
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

#include "inet/clock/GranularityClock.h"

namespace inet {

Define_Module(GranularityClock);

void GranularityClock::initialize()
{
    LinearClock::initialize();
    granularity = par("granularity");
    if (granularity < CLOCKTIME_ZERO)
        throw cRuntimeError("incorrect granularity value: %s, granularity must be 0 or positive value", granularity.str().c_str());
    if (granularity == CLOCKTIME_ZERO)
        granularity.setRaw(1);
    granularityRaw = granularity.raw();
}

// TODO: rename to clockTimeFloor?
clocktime_t GranularityClock::granularize(clocktime_t clock) const
{
    return ClockTime().setRaw((clock.raw() / granularityRaw) * granularityRaw);
}

// TODO: rename to clockTimeCeil?
clocktime_t GranularityClock::granularizeUp(clocktime_t clock) const
{
    return ClockTime().setRaw(((clock.raw() + granularityRaw - 1) / granularityRaw) * granularityRaw);
}

// TODO: are these computeSimTimeFromClockTime/computeClockTimeFromSimTime consistent wrt. granularity?
simtime_t GranularityClock::computeSimTimeFromClockTime(clocktime_t clock) const
{
    return origin.simtime + (granularize(clock) - origin.clocktime).asSimTime() / (1.0 + driftRate);
}

clocktime_t GranularityClock::computeClockTimeFromSimTime(simtime_t t) const
{
    return granularize(LinearClock::computeClockTimeFromSimTime(t));
}

void GranularityClock::scheduleClockEventAt(clocktime_t clock, ClockEvent *msg)
{
    // TODO: why up? why not down? why not round? rounding mode? why isn't it up to the client?
    // TODO: it's unclear to me whether we should accept a clock delay that is not a multiple of the clock granularity
    clock = granularizeUp(clock);


    // TODO: do we actually repeat some code here?
    cSimpleModule *targetModule = getTargetModule();
//    msg->setSchedulerModule(targetModule);
    msg->setClock(this);
    msg->setRelative(false);
    msg->setArrivalClockTime(clock);
    simtime_t now = simTime();
    simtime_t t;
    // TODO: rounding up again or what?
    t = computeSimTimeFromClockTime(granularizeUp(clock));
    if (t < now) {
        clocktime_t nowClock = getClockTime();
        if (clock >= nowClock && clock < nowClock + granularity) {
            t = now;
        }
        // TODO: else we schedule in the past?
    }
    targetModule->scheduleAt(t, msg);
}

void GranularityClock::scheduleClockEventAfter(clocktime_t clockDelay, ClockEvent *msg)
{
    // TODO: why up? why not down? why not round? rounding mode? why isn't it up to the client?
    // TODO: it's unclear to me whether we should accept a clock delay that is not a multiple of the clock granularity
    clockDelay = granularizeUp(clockDelay);
    cSimpleModule *targetModule = getTargetModule();
//    msg->setSchedulerModule(targetModule);
    msg->setClock(this);
    msg->setRelative(true);
    clocktime_t nowClock = getClockTime();
    clocktime_t arrivalClockTime = nowClock + clockDelay;
    msg->setArrivalClockTime(arrivalClockTime);
    simtime_t delay = clockDelay.isZero() ? SIMTIME_ZERO : computeSimTimeFromClockTime(arrivalClockTime) - simTime();
    targetModule->scheduleAfter(delay, msg);
}

} // namespace inet

