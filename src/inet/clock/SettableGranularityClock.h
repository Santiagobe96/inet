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

#ifndef __INET_SettableGranularityClock_H
#define __INET_SettableGranularityClock_H

#include "inet/clock/GranularityClock.h"
#include "inet/common/scenario/IScriptable.h"

namespace inet {

/**
 * Models a clock with a constant clock drift rate.
 */
// TODO: we should not copy the code from SettableLinearClock here
class INET_API SettableGranularityClock : public GranularityClock, public IScriptable
{
  private:
    std::vector<ClockEvent*> timers;

  protected:
    void purgeTimers();
    void rescheduleTimers(clocktime_t clockDelta);

    // IScriptable implementation
    virtual void processCommand(const cXMLElement& node) override;

  public:
    virtual void scheduleClockEventAt(clocktime_t t, ClockEvent *msg) override;
    virtual void scheduleClockEventAfter(clocktime_t t, ClockEvent *msg) override;
    virtual ClockEvent *cancelClockEvent(ClockEvent *msg) override;

    virtual void setClockTime(clocktime_t t);

    virtual void handleClockEventOccured(ClockEvent *msg) override;
};

} // namespace inet

#endif // ifndef __INET_SettableGranularityClock_H

