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

#include "inet/clock/LinearOscillator.h"

namespace inet {

Define_Module(LinearOscillator);

void LinearOscillator::initialize()
{
    nominalTickLength = par("nominalTickLength");
    driftRate = par("driftRate").doubleValue() / 1E+6;
    baseTickTime = 0;
    lastTickTime = 0;
    previousTickDistanceAtLastStateChange = 0;
    nextTickDistanceAtLastStateChange = nominalTickLength / (1.0 + driftRate);
}

void LinearOscillator::setDriftRate(double newDriftRate)
{
    Enter_Method("setDriftRate");
    if (newDriftRate != driftRate) {
        simtime_t currentSimTime = simTime();
        simtime_t currentTickLength = nominalTickLength / (1.0 + driftRate);
        lastTickTime = currentSimTime - fmod(currentSimTime - lastTickTime, currentTickLength);
        nextTickDistanceAtLastStateChange = lastTickTime + currentTickLength - simTime();
        nextTickDistanceAtLastStateChange = nextTickDistanceAtLastStateChange * (1.0 + driftRate) / (1.0 + newDriftRate);
        driftRate = newDriftRate;
        emit(driftRateChangedSignal, driftRate);
    }
}

int64_t LinearOscillator::computeClockTicksForInterval(simtime_t timeInterval) const
{
    // TODO: check if this works with all kinds of scales for timeInterval, nominalTickLength and driftRate
    return (int64_t)floor(((timeInterval - nextTickDistanceAtLastStateChange).dbl() / nominalTickLength.dbl()) * (1.0 + driftRate));
}

simtime_t LinearOscillator::computeIntervalForClockTicks(int64_t numTicks) const
{
    return nominalTickLength * numTicks / (1.0 + driftRate);
}

void LinearOscillator::processCommand(const cXMLElement& node)
{
    Enter_Method("processCommand");
    if (!strcmp(node.getTagName(), "set-oscillator")) {
        if (const char *driftRateStr = node.getAttribute("driftRate")) {
            double newDriftRate = strtod(driftRateStr, nullptr);
            setDriftRate(newDriftRate);
        }
    }
    else
        throw cRuntimeError("Invalid command: %s", node.getTagName());
}

} // namespace inet

