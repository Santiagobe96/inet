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

#ifndef __INET_GRANULARITYCLOCK_H
#define __INET_GRANULARITYCLOCK_H

#include "inet/clock/LinearClock.h"

namespace inet {

/**
 * Models a clock with a constant granularity.
 */
// TODO: shouldn't we add clock granularity support to LinearClock and SettableLinearClock instead of duplicating them in a class hierarchy?
// TODO: support/require conversion between of duration between simtime and clock time (e.g. 20ms -> 20 clock ticks)
// TODO: need parameter for scale conversion?
// TODO: parameter for rounding mode up/down/round/none where none means exact values are expected
class INET_API GranularityClock : public LinearClock
{
  protected:
    // TODO: add rounding mode?
    clocktime_t granularity;
    // TODO: this is not needed, all functions in raw() are inline
    int64_t granularityRaw;    // cached granularity.raw()

  protected:
    virtual bool isClockTick(); // decides if the current simulation time is a clock tick or not

  public:
    virtual void initialize() override;

    // TODO: how is getClockTime not overrided??? shouldn't we apply clock granularity to the current clock time? I hope so...
    virtual clocktime_t granularize(clocktime_t clock) const;
    virtual clocktime_t granularizeUp(clocktime_t clock) const;
    virtual clocktime_t computeClockTimeFromSimTime(simtime_t t) const override;
    virtual simtime_t computeSimTimeFromClockTime(clocktime_t t) const override;
    virtual void scheduleClockEventAt(clocktime_t t, ClockEvent *msg) override;
    virtual void scheduleClockEventAfter(clocktime_t t, ClockEvent *msg) override;
};

} // namespace inet

#endif // ifndef __INET_GRANULARITYCLOCK_H

