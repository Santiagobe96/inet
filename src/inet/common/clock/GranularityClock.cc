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

#include "inet/common/clock/GranularityClock.h"

namespace inet {

Define_Module(GranularityClock);

void GranularityClock::initialize()
{
    granularity = par("granularity");
    if (granularity < CLOCKTIME_ZERO)
        throw cRuntimeError("incorrect granularity value: %s, granularity must be 0 or positive value", granularity.str().c_str());
    if (granularity == CLOCKTIME_ZERO)
        granularity.setRaw(1);
    granularityRaw = granularity.raw();
    driftRate = par("driftRate").doubleValue() / 1e6;
    origin.simtime = simTime();
    simtime_t clock = par("timeShift");
    origin.clocktime = ClockTime::from(simTime() + clock);
}

clocktime_t GranularityClock::granularize(clocktime_t clock) const
{
    return ClockTime().setRaw((clock.raw() / granularityRaw) * granularityRaw);
}

clocktime_t GranularityClock::granularizeUp(clocktime_t clock) const
{
    return ClockTime().setRaw(((clock.raw() + granularityRaw - 1) / granularityRaw) * granularityRaw);
}

simtime_t GranularityClock::toSimTime(clocktime_t clock) const
{
    return origin.simtime + (granularize(clock) - origin.clocktime).asSimTime() / (1.0 + driftRate);
}

clocktime_t GranularityClock::fromSimTimePrecise(simtime_t t) const
{
    return origin.clocktime + ClockTime::from((t-origin.simtime) * (1.0 + driftRate));
}

clocktime_t GranularityClock::fromSimTime(simtime_t t) const
{
    return granularize(fromSimTimePrecise(t));
}

clocktime_t GranularityClock::getClockTime() const
{
    return fromSimTime(simTime());
}

void GranularityClock::scheduleClockEventAt(clocktime_t clock, ClockEvent *msg)
{
    clock = granularizeUp(clock);
    cSimpleModule *targetModule = getTargetModule();
    msg->setSchedulerModule(targetModule);
    msg->setClockModule(this);
    msg->setRelative(false);
    msg->setArrivalClockTime(clock);
    simtime_t now = simTime();
    simtime_t t;
    t = toSimTime(granularizeUp(clock));
    if (t < now) {
        clocktime_t nowClock = getClockTime();
        if (clock >= nowClock && clock < nowClock + granularity) {
            t = now;
        }
    }
    targetModule->scheduleAt(t, msg);
}

void GranularityClock::scheduleClockEventAfter(clocktime_t clockDelay, ClockEvent *msg)
{
    clockDelay = granularizeUp(clockDelay);
    cSimpleModule *targetModule = getTargetModule();
    msg->setSchedulerModule(targetModule);
    msg->setClockModule(this);
    msg->setRelative(true);
    clocktime_t nowClock = getClockTime();
    clocktime_t arrivalClockTime = nowClock + clockDelay;
    msg->setArrivalClockTime(arrivalClockTime);
    simtime_t delay = clockDelay.isZero() ? SIMTIME_ZERO : toSimTime(arrivalClockTime) - simTime();
    targetModule->scheduleAfter(delay, msg);
}

cMessage *GranularityClock::cancelClockEvent(ClockEvent *msg)
{
    msg->setSchedulerModule(nullptr);
    msg->setClockModule(nullptr);
    return getTargetModule()->cancelEvent(msg);
}

void GranularityClock::arrived(ClockEvent *msg)
{
    msg->setSchedulerModule(nullptr);
    msg->setClockModule(nullptr);
}

} // namespace inet

