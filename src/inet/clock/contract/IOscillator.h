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

#ifndef __INET_IOSCILLATOR_H
#define __INET_IOSCILLATOR_H

#include "inet/common/INETDefs.h"

namespace inet {

/**
 * TODO
 */
class INET_API IOscillator
{
  public:
    static simsignal_t driftRateChangedSignal;

  public:
    virtual ~IOscillator() {}

    /**
     * Returns the nominal time interval between subsequent ticks. This value
     * is usually different from the actual amount of elapsed time between ticks.
     */
    virtual simtime_t getNominalTickLength() const = 0;

    /**
     * Returns the number of ticks for the specified time interval measured from
     * the last emitted driftRateChangedSignal???
     * TODO: it's about the future and not the past!!!
     */
    virtual int64_t computeClockTicksForInterval(simtime_t timeInterval) const = 0;

    /**
     *
     */
    virtual simtime_t computeIntervalForClockTicks(int64_t numTicks) const = 0;
};

} // namespace inet

#endif // ifndef __INET_IOSCILLATOR_H

