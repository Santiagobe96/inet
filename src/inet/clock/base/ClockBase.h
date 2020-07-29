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

#ifndef __INET_CLOCKBASE_H
#define __INET_CLOCKBASE_H

#include "inet/clock/common/ClockTime.h"
#include "inet/clock/contract/IClock.h"

namespace inet {

class INET_API ClockBase : public cSimpleModule, public IClock
{
  protected:
    inline cSimpleModule *getTargetModule() const {
        cSimpleModule *target = getSimulation()->getContextSimpleModule();
        if (target == nullptr)
            throw cRuntimeError("scheduleAt()/cancelEvent() must be called with a simple module in context");
        return target;
    }

  public:
    /**
     * Returns the clock time for the specified simulation time according to the
     * current state of the clock. This method implements a monotonic function
     * with respect to the simulation time argument. It's allowed to return a
     * different value for the same argument value if the clock is set between
     * calls.
     */
    virtual clocktime_t computeClockTimeFromSimTime(simtime_t t) const = 0;

    /**
     * Returns the simulation time for the specified clock time according to the
     * current state of the clock. It's allowed to return a different value for
     * the same argument value if the clock is set.
     */
    virtual simtime_t computeSimTimeFromClockTime(clocktime_t t) const = 0;

    virtual clocktime_t getClockTime() const override;
    virtual clocktime_t convertSimTime2ClockTime(simtime_t time) const override;
    virtual simtime_t convertClockTime2SimTime(clocktime_t time) const override;
    virtual void scheduleClockEventAt(clocktime_t t, ClockEvent *msg) override;
    virtual void scheduleClockEventAfter(clocktime_t t, ClockEvent *msg) override;
    virtual ClockEvent *cancelClockEvent(ClockEvent *msg) override;
    virtual void handleClockEventOccured(ClockEvent *msg) override;
};

} // namespace inet

#endif // ifndef __INET_CLOCKBASE_H

