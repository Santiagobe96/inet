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

void SettableLinearClock::scheduleClockEventAt(clocktime_t t, ClockEvent *event)
{
    LinearClock::scheduleClockEventAt(t, event);
    events.push_back(event);
}

void SettableLinearClock::scheduleClockEventAfter(clocktime_t t, ClockEvent *event)
{
    LinearClock::scheduleClockEventAfter(t, event);
    events.push_back(event);
}

ClockEvent *SettableLinearClock::cancelClockEvent(ClockEvent *event)
{
    events.erase(std::remove(events.begin(), events.end(), event), events.end());
    return static_cast<ClockEvent *>(getTargetModule()->cancelEvent(event));
}

void SettableLinearClock::setClockTime(clocktime_t newClockTime)
{
    Enter_Method("setClockTime");
    clocktime_t oldClockTime = getClockTime();
    if (newClockTime != oldClockTime) {
        simtime_t currentSimTime = simTime();
        EV_DEBUG << "Setting the clock time from " << oldClockTime << " to " << newClockTime << " at simtime " << currentSimTime << ".\n";
        originSimTime = currentSimTime;
        originClockTime = newClockTime;
        clocktime_t clockDelta = newClockTime - oldClockTime;
        for (auto event : events) {
            if (event->getRelative())
                // NOTE: the simulation time of event execution is affected
                event->setArrivalClockTime(event->getArrivalClockTime() + clockDelta);
            else {
                clocktime_t arrivalClockTime = event->getArrivalClockTime();
                simtime_t arrivalSimTime = computeSimTimeFromClockTime(arrivalClockTime);
                if (arrivalSimTime < currentSimTime) {
                    switch (event->getOverdueClockEventHandlingMode()) {
                        case EXECUTE:
                            EV_WARN << "Executing overdue clock event " << event->getName() << " immediately.\n";
                            arrivalSimTime = currentSimTime;
                            break;
                        case SKIP:
                            EV_WARN << "Skipping overdue clock event " << event->getName() << ".\n";
                            cancelClockEvent(event);
                            continue;
                        case ERROR:
                            throw cRuntimeError("Clock event is overdue");
                        default:
                            throw cRuntimeError("Unknown overdue clock event handling mode");
                    }
                }
                cSimpleModule *targetModule = check_and_cast<cSimpleModule *>(event->getArrivalModule());
                cContextSwitcher contextSwitcher(targetModule);
                targetModule->rescheduleAt(arrivalSimTime, event);
            }
        }
    }
}

void SettableLinearClock::processCommand(const cXMLElement& node)
{
    Enter_Method("processCommand");
    if (!strcmp(node.getTagName(), "set-clock")) {
        if (const char *clockTimeStr = node.getAttribute("time")) {
            clocktime_t t = ClockTime::parse(clockTimeStr);
            setClockTime(t);
        }
    }
    else
        throw cRuntimeError("Invalid command: %s", node.getTagName());
}

void SettableLinearClock::handleClockEventOccured(ClockEvent *event)
{
    LinearClock::handleClockEventOccured(event);
    events.erase(std::remove(events.begin(), events.end(), event), events.end());
}

void SettableLinearClock::receiveSignal(cComponent *source, int signal, cObject *obj, cObject *details)
{
    if (signal == IOscillator::driftRateChangedSignal) {
        simtime_t currentSimTime = simTime();
        clocktime_t currentClockTime = getClockTime();
        // modify 'origin' to current values before change the driftRate
        originSimTime = currentSimTime;
        originClockTime = currentClockTime;
        for (auto event : events) {
            cSimpleModule *targetModule = check_and_cast<cSimpleModule *>(event->getArrivalModule());
            if (event->getRelative()) {
                simtime_t simTimeDelay = computeSimTimeFromClockTime(event->getArrivalClockTime()) - currentSimTime;
                if (simTimeDelay < 0) {
                    switch (event->getOverdueClockEventHandlingMode()) {
                        case EXECUTE:
                            EV_WARN << "Executing overdue clock event " << event->getName() << " immediately.\n";
                            simTimeDelay = 0;
                            break;
                        case SKIP:
                            EV_WARN << "Skipping overdue clock event " << event->getName() << ".\n";
                            cancelClockEvent(event);
                            continue;
                        case ERROR:
                            throw cRuntimeError("Clock event is overdue");
                        default:
                            throw cRuntimeError("Unknown overdue clock event handling mode");
                    }
                }
                cContextSwitcher contextSwitcher(targetModule);
                targetModule->rescheduleAfter(simTimeDelay, event);
            }
            else {
                clocktime_t arrivalClockTime = event->getArrivalClockTime();
                simtime_t arrivalSimTime = computeSimTimeFromClockTime(arrivalClockTime);
                if (arrivalSimTime < currentSimTime) {
                    switch (event->getOverdueClockEventHandlingMode()) {
                        case EXECUTE:
                            EV_WARN << "Executing overdue clock event " << event->getName() << " immediately.\n";
                            arrivalSimTime = currentSimTime;
                            break;
                        case SKIP:
                            EV_WARN << "Skipping overdue clock event " << event->getName() << ".\n";
                            cancelClockEvent(event);
                            continue;
                        case ERROR:
                            throw cRuntimeError("Clock event is overdue");
                        default:
                            throw cRuntimeError("Unknown overdue clock event handling mode");
                    }
                }
                cContextSwitcher contextSwitcher(targetModule);
                targetModule->rescheduleAt(arrivalSimTime, event);
            }
        }
    }
}

} // namespace inet

