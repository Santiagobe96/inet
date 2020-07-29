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

#ifndef __INET_SETTABLELINEARCLOCK_H
#define __INET_SETTABLELINEARCLOCK_H

#include "inet/clock/LinearClock.h"
#include "inet/common/scenario/IScriptable.h"

namespace inet {

/**
 * Models a settable clock with a constant clock drift rate.
 */
class INET_API SettableLinearClock : public LinearClock, public IScriptable, public cListener
{
  protected:
    std::vector<ClockEvent *> events;

  protected:
    // IScriptable implementation
    virtual void processCommand(const cXMLElement& node) override;

  public:
    virtual void scheduleClockEventAt(clocktime_t time, ClockEvent *event) override;
    virtual void scheduleClockEventAfter(clocktime_t delay, ClockEvent *event) override;
    virtual ClockEvent *cancelClockEvent(ClockEvent *event) override;

    virtual void setClockTime(clocktime_t time);

    virtual void handleClockEventOccured(ClockEvent *event) override;

    virtual void receiveSignal(cComponent *source, int signal, cObject *obj, cObject *details) override;
};

} // namespace inet

#endif // ifndef __INET_SETTABLELINEARCLOCK_H

