#include <gtest/gtest.h>

#include "bms_config.h"
#include "event_log.h"
#include "status.h"

TEST(EventLogTests, UT20_AddingEventToFullLogReturnsCapacityFull)
{
    EventLog_t log{};

    ASSERT_EQ(initEventLog(&log), STATUS_SUCCESS);

    // Fill the log using the public API rather than modifying its internals.
    for (int i = 0; i < MAX_EVENTS; ++i)
    {
        ASSERT_EQ(
            addFaultEvent(&log, static_cast<double>(i),
                          FAULT_OVER_CURRENT, 75.0),
            STATUS_SUCCESS
        ) << "Failed while inserting event " << i;
    }

    EXPECT_EQ(
        addFaultEvent(&log, static_cast<double>(MAX_EVENTS),
                      FAULT_OVER_CURRENT, 75.0),
        STATUS_CAPACITY_FULL
    );
}
