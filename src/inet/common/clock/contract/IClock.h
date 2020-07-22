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

#ifndef __INET_ICLOCK_H
#define __INET_ICLOCK_H

#include "inet/common/clock/common/ClockTime.h"
#include "inet/common/clock/common/ClockEvent_m.h"

namespace inet {

/**
 * This class defines the interface for clocks.
 * See the corresponding NED file for details.
 */
class INET_API IClock
{
  public:
    virtual ~IClock() {}

    /**
     * Return the current time.
     */
    virtual clocktime_t getClockTime() const = 0;

    /**
     * Schedule an event to be delivered to the context module at the given time.
     */
    virtual void scheduleClockEventAt(clocktime_t t, ClockEvent *msg) = 0;

    /**
     * Schedule an event to be delivered to the context module at the given
     * time delta after the current simulation time.
     */
    virtual void scheduleClockEventAfter(clocktime_t t, ClockEvent *msg) = 0;

    /**
     * Cancels an event.
     */
    virtual cMessage *cancelClockEvent(ClockEvent *msg) = 0;

    /**
     * notification about msg arrived to a cSimpleModule.
     */
    virtual void arrived(ClockEvent *msg) = 0;
};

} // namespace inet

#endif // ifndef __INET_ICLOCK_H

