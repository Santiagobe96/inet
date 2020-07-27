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

#include<bits/stdc++.h>

#include "inet/common/clock/SettableGranularityClock.h"

namespace inet {

Define_Module(SettableGranularityClock);

void SettableGranularityClock::initialize()
{
    GranularityClock::initialize();
}

void SettableGranularityClock::scheduleClockEventAt(clocktime_t t, ClockEvent *msg)
{
    GranularityClock::scheduleClockEventAt(t, msg);
    timers.push_back(msg);
}

void SettableGranularityClock::scheduleClockEventAfter(clocktime_t t, ClockEvent *msg)
{
    GranularityClock::scheduleClockEventAfter(t, msg);
    timers.push_back(msg);
}

cMessage *SettableGranularityClock::cancelClockEvent(ClockEvent *msg)
{
    std::remove(timers.begin(), timers.end(), msg);
    return getTargetModule()->cancelEvent(msg);
}

void SettableGranularityClock::rescheduleTimers(clocktime_t clockDelta)
{
    simtime_t now = simTime();
    for (auto it: timers) {
        cSimpleModule *targetModule = const_cast<cSimpleModule *>(it->getSchedulerModule());
        if (it->getRelative()) {
            it->setArrivalClockTime(it->getArrivalClockTime() + clockDelta);
            simtime_t newDelta = toSimTime(it->getArrivalClockTime()) - now;
            {
                cContextSwitcher tmp(targetModule);
                targetModule->rescheduleAfter(newDelta, it);
            }
        }
        else {
            clocktime_t ct = it->getArrivalClockTime();
            simtime_t newTime = toSimTime(ct);
            if (newTime < now)   //TODO or cancel event or notify scheduler module?
                newTime = now;
            {
                cContextSwitcher tmp(targetModule);
                targetModule->rescheduleAt(newTime, it);
            }
        }
    }
}

void SettableGranularityClock::setDriftRate(double newDriftRate)
{
    simtime_t atSimtime = simTime();
    clocktime_t nowClock = LinearClock::fromSimTime(atSimtime);
    EV_DEBUG << "set driftRate from " << driftRate << " to " << newDriftRate << " at simtime " << atSimtime << ", clock " << nowClock << endl;
    // modify 'origin' to current values before change the driftRate
    origin.simtime = atSimtime;
    origin.clocktime = nowClock;
    driftRate = newDriftRate;
    rescheduleTimers(CLOCKTIME_ZERO);
}

void SettableGranularityClock::setClockTime(clocktime_t t)
{
    clocktime_t oldClock = getClockTime();
    simtime_t atSimtime = toSimTime(oldClock);
    t = granularize(t);
    EV_DEBUG << "set clock time from " << oldClock << " to " << t << " at simtime " << atSimtime << endl;
    origin.simtime = atSimtime;
    origin.clocktime = t;
    rescheduleTimers(t - oldClock);
}

void SettableGranularityClock::processCommand(const cXMLElement& node)
{
    Enter_Method("processCommand");

    if (!strcmp(node.getTagName(), "set-clock")) {
        if (const char *clockTimeStr = node.getAttribute("time")) {
            EV_DEBUG << "processCommand: set clock time to " << clockTimeStr << endl;
            clocktime_t t = ClockTime::parse(clockTimeStr);
            setClockTime(t);
        }
        if (const char *driftRateStr = node.getAttribute("driftRate")) {
            EV_DEBUG << "processCommand: set drift rate to " << driftRateStr << endl;
            double rate = strtod(driftRateStr, nullptr);
            setDriftRate(rate);
        }
    }
    else
        throw cRuntimeError("invalid command node for %s at %s", getClassName(), node.getSourceLocation());
}

void SettableGranularityClock::arrived(ClockEvent *msg)
{
    GranularityClock::arrived(msg);
    std::remove(timers.begin(), timers.end(), msg);
}

} // namespace inet

