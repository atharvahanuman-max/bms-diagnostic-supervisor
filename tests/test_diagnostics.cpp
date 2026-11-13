#include <gtest/gtest.h>

#include "bms_config.h"
#include "diagnostics.h"
#include "sample.h"
#include "status.h"

namespace
{
Sample_t makeHealthySample()
{
    Sample_t sample{};
    sample.time_s = 1.0;
    sample.current_a = 5.0;
    sample.temperature_c = 25.0;

    for (int i = 0; i < CELLS_IN_SERIES; ++i)
    {
        sample.cell_v[i] = 3.8;
    }

    return sample;
}
} // namespace

TEST(DiagnosticsTests, UT15_DetectCellOverVoltage)
{
    Sample_t sample = makeHealthySample();
    sample.cell_v[0] = 4.25;

    FaultType_t fault = FAULT_NONE;

    ASSERT_EQ(detectFault(&sample, &fault), STATUS_SUCCESS);
    EXPECT_EQ(fault, FAULT_OVER_VOLTAGE);
}

TEST(DiagnosticsTests, UT16_ExactOverVoltageThresholdIsHealthy)
{
    Sample_t sample = makeHealthySample();
    sample.cell_v[0] = FAULT_OVER_VOLTAGE_V;

    FaultType_t fault = FAULT_NONE;

    for (int i = 0; i < CELLS_IN_SERIES; i++)
    {
    sample.cell_v[i] = 4.20;
    }

    ASSERT_EQ(detectFault(&sample, &fault), STATUS_SUCCESS);
    EXPECT_EQ(fault, FAULT_NONE);
}

TEST(DiagnosticsTests, UT17_OverTemperatureHasPriorityOverOverVoltage)
{
    Sample_t sample = makeHealthySample();
    sample.temperature_c = FAULT_OVER_TEMPERATURE_C + 1.0;
    sample.cell_v[0] = FAULT_OVER_VOLTAGE_V + 0.05;

    FaultType_t fault = FAULT_NONE;

    ASSERT_EQ(detectFault(&sample, &fault), STATUS_SUCCESS);
    EXPECT_EQ(fault, FAULT_OVER_TEMPERATURE);
}

TEST(DiagnosticsTests, UT18_DetectFaultRejectsNullSample)
{
    FaultType_t fault = FAULT_NONE;

    EXPECT_EQ(detectFault(nullptr, &fault), STATUS_NULL_POINTER);
}

TEST(DiagnosticsTests, UT19_NegativeCurrentSelectsChargingState)
{
    EXPECT_EQ(determinePackState(-5.0, FAULT_NONE), PACK_CHARGING);
}
