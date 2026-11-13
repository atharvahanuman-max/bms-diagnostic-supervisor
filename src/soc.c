#include "soc.h"

static double clampSoc(double soc)
{
    if (soc < SOC_MIN_PCT)
    {
        return SOC_MIN_PCT;
    }

    if (soc > SOC_MAX_PCT)
    {
        return SOC_MAX_PCT;
    }

    return soc;
}

double estimateInitialSoc(double v)
{
    double soc = (v - SOC_EMPTY_VOLTAGE_V) /
                 (SOC_FULL_VOLTAGE_V - SOC_EMPTY_VOLTAGE_V) * 100.0;

    return clampSoc(soc);
}

double updateSoc(double soc_pct, double current_a,
                 double dt_s, double capacity_ah)
{
    if (capacity_ah <= 0 || dt_s < 0)
    {
        return clampSoc(soc_pct);
    }

    double change = (current_a * dt_s / SECONDS_PER_HOUR) /
                    capacity_ah * 100.0;

    return clampSoc(soc_pct - change);
}