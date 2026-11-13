#include <gtest/gtest.h>

#include "soc.h"

TEST(SocTests, UT09_MapEmptyAndFullVoltageToSocLimits)
{
    EXPECT_DOUBLE_EQ(estimateInitialSoc(3.0), 0.0);
    EXPECT_DOUBLE_EQ(estimateInitialSoc(4.2), 100.0);
}

TEST(SocTests, UT10_MapMidpointVoltageToFiftyPercent)
{
    EXPECT_DOUBLE_EQ(estimateInitialSoc(3.6), 50.0);
}

TEST(SocTests, UT11_ClampInitialSocOutsideVoltageMapping)
{
    EXPECT_DOUBLE_EQ(estimateInitialSoc(2.0), 0.0);
    EXPECT_DOUBLE_EQ(estimateInitialSoc(5.0), 100.0);
}

TEST(SocTests, UT12_CoulombCountDischarge)
{
    // 10 A for 360 seconds from a 10 Ah battery removes 10% SoC.
    EXPECT_DOUBLE_EQ(updateSoc(50.0, 10.0, 360.0, 10.0), 40.0);
}

TEST(SocTests, UT13_ClampSocAtEmptyDuringDischarge)
{
    EXPECT_DOUBLE_EQ(updateSoc(5.0, 10.0, 3600.0, 10.0), 0.0);
}

TEST(SocTests, UT14_ChargingRaisesSocButDoesNotExceedOneHundred)
{
    EXPECT_DOUBLE_EQ(updateSoc(50.0, -10.0, 360.0, 10.0), 60.0);
    EXPECT_DOUBLE_EQ(updateSoc(95.0, -10.0, 3600.0, 10.0), 100.0);
}
