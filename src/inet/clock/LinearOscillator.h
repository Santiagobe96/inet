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

#ifndef __INET_LINEAROSCILLATOR_H
#define __INET_LINEAROSCILLATOR_H

#include "inet/clock/contract/IOscillator.h"
#include "inet/common/INETMath.h"
#include "inet/common/scenario/IScriptable.h"

namespace inet {

class INET_API LinearOscillator : public cSimpleModule, public IOscillator, public IScriptable
{
  protected:
    simtime_t nominalTickLength;
    double driftRate = NaN;
    simtime_t baseTickTime;
    simtime_t lastTickTime;
    simtime_t previousTickDistanceAtLastStateChange;
    simtime_t nextTickDistanceAtLastStateChange;

  protected:
    virtual void initialize() override;

    // IScriptable implementation
    virtual void processCommand(const cXMLElement& node) override;

  public:
    virtual simtime_t getNominalTickLength() const override { return nominalTickLength; }

    virtual void setDriftRate(double driftRate);

    virtual int64_t computeClockTicksForInterval(simtime_t timeInterval) const override;

    virtual simtime_t computeIntervalForClockTicks(int64_t numTicks) const override;
};

} // namespace inet

#endif // ifndef __INET_LINEAROSCILLATOR_H

