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

#include "inet/clock/SettableGranularityClock.h"

// TODO: we should not copy the code from SettableLinearClock here
namespace inet {

Define_Module(SettableGranularityClock);

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

ClockEvent *SettableGranularityClock::cancelClockEvent(ClockEvent *msg)
{
    std::remove(timers.begin(), timers.end(), msg);
    return static_cast<ClockEvent *>(getTargetModule()->cancelEvent(msg));
}

void SettableGranularityClock::rescheduleTimers(clocktime_t clockDelta)
{
    simtime_t now = simTime();
    for (auto it: timers) {
        // TODO: replace with getArrivalModule() and delete schedulerModule
        cSimpleModule *targetModule = check_and_cast<cSimpleModule *>(it->getArrivalModule());
        if (it->getRelative()) {
            it->setArrivalClockTime(it->getArrivalClockTime() + clockDelta);
            simtime_t newDelta = computeSimTimeFromClockTime(it->getArrivalClockTime()) - now;
            {
                cContextSwitcher tmp(targetModule);
                targetModule->rescheduleAfter(newDelta, it);
            }
        }
        else {
            clocktime_t ct = it->getArrivalClockTime();
            simtime_t newTime = computeSimTimeFromClockTime(ct);
            if (newTime < now)   //TODO or cancel event or notify scheduler module?
                newTime = now;
            {
                cContextSwitcher tmp(targetModule);
                targetModule->rescheduleAt(newTime, it);
            }
        }
    }
}

void SettableGranularityClock::setClockTime(clocktime_t t)
{
    clocktime_t oldClock = getClockTime();
    simtime_t atSimtime = computeSimTimeFromClockTime(oldClock);
    t = granularize(t);
    EV_DEBUG << "set clock time from " << oldClock << " to " << t << " at simtime " << atSimtime << endl;
    originSimTime = atSimtime;
    originClockTime = t;
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
//        if (const char *driftRateStr = node.getAttribute("driftRate")) {
//            EV_DEBUG << "processCommand: set drift rate to " << driftRateStr << endl;
//            double rate = strtod(driftRateStr, nullptr);
//            setDriftRate(rate);
//        }
    }
    else
        throw cRuntimeError("invalid command node for %s at %s", getClassName(), node.getSourceLocation());
}

void SettableGranularityClock::handleClockEventOccured(ClockEvent *msg)
{
    GranularityClock::handleClockEventOccured(msg);
    std::remove(timers.begin(), timers.end(), msg);
}

} // namespace inet

