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

#include "inet/clock/base/ClockBase.h"

namespace inet {

clocktime_t ClockBase::convertSimTime2ClockTime(simtime_t time) const
{
    return ClockTime::from(time);
}

simtime_t ClockBase::convertClockTime2SimTime(clocktime_t time) const
{
    return time.asSimTime();
}

clocktime_t ClockBase::getClockTime() const
{
    return computeClockTimeFromSimTime(simTime());
}

void ClockBase::scheduleClockEventAt(clocktime_t t, ClockEvent *msg)
{
    ASSERT(msg->getClock() == nullptr);
    if (t < getClockTime())
        throw cRuntimeError("TODO");
    cSimpleModule *targetModule = getTargetModule();
    msg->setClock(this);
    msg->setRelative(false);
    msg->setArrivalClockTime(t);
    targetModule->scheduleAt(computeSimTimeFromClockTime(t), msg);
}

void ClockBase::scheduleClockEventAfter(clocktime_t clockDelay, ClockEvent *msg)
{
    ASSERT(msg->getClock() == nullptr);
    if (clockDelay < 0)
        throw cRuntimeError("TODO");
    cSimpleModule *targetModule = getTargetModule();
    msg->setClock(this);
    msg->setRelative(true);
    clocktime_t nowClock = getClockTime();
    clocktime_t arrivalClockTime = nowClock + clockDelay;
    msg->setArrivalClockTime(arrivalClockTime);
    simtime_t delay = clockDelay.isZero() ? SIMTIME_ZERO : computeSimTimeFromClockTime(arrivalClockTime) - simTime();
    targetModule->scheduleAfter(delay, msg);
}

ClockEvent *ClockBase::cancelClockEvent(ClockEvent *msg)
{
    msg->setClock(nullptr);
    return static_cast<ClockEvent *>(getTargetModule()->cancelEvent(msg));
}

void ClockBase::handleClockEventOccured(ClockEvent *msg)
{
    msg->setClock(nullptr);
}

} // namespace inet

