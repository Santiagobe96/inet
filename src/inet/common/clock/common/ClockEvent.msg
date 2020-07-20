//
// This program is property of its copyright holder. All rights reserved.
//

import inet.common.INETDefs;
import inet.common.clock.common.ClockTime;

namespace inet;

message ClockEvent
{
    clocktime_t arrivalClockTime;
    bool relative;
}

