//
// This program is property of its copyright holder. All rights reserved.
//

cplusplus {{
#include "inet/common/clock/contract/IClock.h"

namespace inet { class IClock; }

}}

import inet.common.INETDefs;
import inet.common.clock.common.ClockTime;

namespace inet;

class IClock {
    @existingClass;
    @descriptor(false);
    @cppType(inet::IClock);
}

message ClockEvent
{
    IClock *clockModule;
    cSimpleModule *schedulerModule;
    clocktime_t arrivalClockTime;
    bool relative;
}

cplusplus(ClockEvent) {{
    virtual void execute() override;
}}

cplusplus(cc) {{
    void ClockEvent::execute()
    {
        if (clockModule)
            clockModule->arrived(this);
        cMessage::execute();
    }
}}

