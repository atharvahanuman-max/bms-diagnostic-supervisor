#include <gtest/gtest.h>

#include "bms_config.h"
#include "sample.h"
#include "status.h"

TEST(SampleTests, UT01_ParseValidSampleLine)
{
    const char *line = "5,12.0,30.0,3.9,3.9,3.9,3.9";
    Sample_t sample{};

    EXPECT_EQ(parseSampleLine(line, &sample), STATUS_SUCCESS);
    EXPECT_DOUBLE_EQ(sample.time_s, 5.0);
    EXPECT_DOUBLE_EQ(sample.current_a, 12.0);
    EXPECT_DOUBLE_EQ(sample.temperature_c, 30.0);

    for (int i = 0; i < CELLS_IN_SERIES; ++i)
    {
        EXPECT_DOUBLE_EQ(sample.cell_v[i], 3.9);
    }
}

TEST(SampleTests, UT02_ParseTooFewFields)
{
    const char *line = "5,12.0,30.0,3.9,3.9";
    Sample_t sample{};

    EXPECT_EQ(parseSampleLine(line, &sample), STATUS_PARSE_ERROR);
}

TEST(SampleTests, UT03_ParseNonNumericCurrent)
{
    const char *line = "5,abc,30.0,3.9,3.9,3.9,3.9";
    Sample_t sample{};

    EXPECT_EQ(parseSampleLine(line, &sample), STATUS_PARSE_ERROR);
}

TEST(SampleTests, UT04_ParseNullArguments)
{
    Sample_t sample{};
    const char *valid_line = "5,12.0,30.0,3.9,3.9,3.9,3.9";

    EXPECT_EQ(parseSampleLine(nullptr, &sample), STATUS_NULL_POINTER);
    EXPECT_EQ(parseSampleLine(valid_line, nullptr), STATUS_NULL_POINTER);
}

TEST(SampleTests, UT05_ValidateSampleInsidePlausibilityRanges)
{
    Sample_t sample{};
    sample.time_s = 5.0;
    sample.current_a = 12.0;
    sample.temperature_c = 30.0;

    for (int i = 0; i < CELLS_IN_SERIES; ++i)
    {
        sample.cell_v[i] = 3.9;
    }

    EXPECT_EQ(validateSampleRange(&sample), STATUS_SUCCESS);
}

TEST(SampleTests, UT06_RejectCellVoltageBelowPlausibilityMinimum)
{
    Sample_t sample{};
    sample.current_a = 0.0;
    sample.temperature_c = 25.0;

    for (int i = 0; i < CELLS_IN_SERIES; ++i)
    {
        sample.cell_v[i] = 3.8;
    }
    sample.cell_v[0] = 1.5;

    EXPECT_EQ(validateSampleRange(&sample), STATUS_OUT_OF_RANGE);
}

TEST(SampleTests, UT07_AcceptCellVoltageAtPlausibilityMinimum)
{
    Sample_t sample{};
    sample.current_a = 0.0;
    sample.temperature_c = 25.0;

    for (int i = 0; i < CELLS_IN_SERIES; ++i)
    {
        sample.cell_v[i] = 3.8;
    }
    sample.cell_v[0] = 2.0;

    EXPECT_EQ(validateSampleRange(&sample), STATUS_SUCCESS);
}

TEST(SampleTests, UT08_RejectTemperatureAbovePlausibilityMaximum)
{
    Sample_t sample{};
    sample.current_a = 0.0;
    sample.temperature_c = 125.01;

    for (int i = 0; i < CELLS_IN_SERIES; ++i)
    {
        sample.cell_v[i] = 3.8;
    }

    EXPECT_EQ(validateSampleRange(&sample), STATUS_OUT_OF_RANGE);
}
