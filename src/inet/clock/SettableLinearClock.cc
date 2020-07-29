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

#include "inet/clock/SettableLinearClock.h"

namespace inet {

Define_Module(SettableLinearClock);

void SettableLinearClock::scheduleClockEventAt(clocktime_t t, ClockEvent *msg)
{
    LinearClock::scheduleClockEventAt(t, msg);
    events.push_back(msg);
}

void SettableLinearClock::scheduleClockEventAfter(clocktime_t t, ClockEvent *msg)
{
    LinearClock::scheduleClockEventAfter(t, msg);
    events.push_back(msg);
}

ClockEvent *SettableLinearClock::cancelClockEvent(ClockEvent *msg)
{
    events.erase(std::remove(events.begin(), events.end(), msg), events.end());
    return static_cast<ClockEvent *>(getTargetModule()->cancelEvent(msg));
}

void SettableLinearClock::rescheduleEvents(clocktime_t clockDelta)
{
    // TODO: split into two functions? separate the calls from setClockTime (doesn't affect relative clocks) and setDriftRate
    simtime_t currentSimTime = simTime();
    for (auto event : events) {
        cSimpleModule *targetModule = check_and_cast<cSimpleModule *>(event->getArrivalModule());
        if (event->getRelative()) {
            // TODO: this may result in the rescheduling of events due to imprecision of the calculation if clockDelta != 0
            event->setArrivalClockTime(event->getArrivalClockTime() + clockDelta);
            simtime_t simTimeDelay = computeSimTimeFromClockTime(event->getArrivalClockTime()) - currentSimTime;
            if (simTimeDelay < 0) {
                switch (event->getExpiredClockEventHandlingMode()) {
                    case EXECUTE:
                        EV_WARN << "Executing expired clock event " << event->getName() << " immediately.\n";
                        simTimeDelay = 0;
                        break;
                    case SKIP:
                        EV_WARN << "Skipping expired clock event " << event->getName() << ".\n";
                        cancelClockEvent(event);
                        continue;
                    case ERROR:
                        throw cRuntimeError("Clock event expired");
                    default:
                        throw cRuntimeError("Unknown expired clock event handling mode");
                }
            }
            cContextSwitcher contextSwitcher(targetModule);
            targetModule->rescheduleAfter(simTimeDelay, event);
        }
        else {
            clocktime_t arrivalClockTime = event->getArrivalClockTime();
            simtime_t arrivalSimTime = computeSimTimeFromClockTime(arrivalClockTime);
            if (arrivalSimTime < currentSimTime) {
                switch (event->getExpiredClockEventHandlingMode()) {
                    case EXECUTE:
                        EV_WARN << "Executing expired clock event " << event->getName() << " immediately.\n";
                        arrivalSimTime = currentSimTime;
                        break;
                    case SKIP:
                        EV_WARN << "Skipping expired clock event " << event->getName() << ".\n";
                        cancelClockEvent(event);
                        continue;
                    case ERROR:
                        throw cRuntimeError("Clock event expired");
                    default:
                        throw cRuntimeError("Unknown expired clock event handling mode");
                }
            }
            cContextSwitcher contextSwitcher(targetModule);
            targetModule->rescheduleAt(arrivalSimTime, event);
        }
    }
}

void SettableLinearClock::setDriftRate(double newDriftRate)
{
    simtime_t currentSimTime = simTime();
    clocktime_t currentClockTime = getClockTime();
    EV_DEBUG << "Setting the clock drift rate from " << driftRate << " to " << newDriftRate << " at simtime " << currentSimTime << " and clock time " << currentClockTime <<  ".\n";
    // modify 'origin' to current values before change the driftRate
    origin.simtime = currentSimTime;
    origin.clocktime = currentClockTime;
    driftRate = newDriftRate;
    rescheduleEvents(CLOCKTIME_ZERO);
}

void SettableLinearClock::setClockTime(clocktime_t newClockTime)
{
    clocktime_t oldClockTime = getClockTime();
    simtime_t currentSimTime = simTime();
    EV_DEBUG << "Setting the clock time from " << oldClockTime << " to " << newClockTime << " at simtime " << currentSimTime << ".\n";
    origin.simtime = currentSimTime;
    origin.clocktime = newClockTime;
    rescheduleEvents(newClockTime - oldClockTime);
}

void SettableLinearClock::processCommand(const cXMLElement& node)
{
    Enter_Method("processCommand");
    if (!strcmp(node.getTagName(), "set-clock")) {
        if (const char *clockTimeStr = node.getAttribute("time")) {
            clocktime_t t = ClockTime::parse(clockTimeStr);
            setClockTime(t);
        }
        if (const char *driftRateStr = node.getAttribute("driftRate")) {
            double rate = strtod(driftRateStr, nullptr);
            setDriftRate(rate);
        }
    }
    else
        throw cRuntimeError("Invalid command: %s", node.getTagName());
}

void SettableLinearClock::handleClockEventOccured(ClockEvent *msg)
{
    LinearClock::handleClockEventOccured(msg);
    events.erase(std::remove(events.begin(), events.end(), msg), events.end());
}

} // namespace inet

