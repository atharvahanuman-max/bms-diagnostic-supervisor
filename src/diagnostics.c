#include <stddef.h>
#include "diagnostics.h"
#include <math.h>

Status_t calculateVoltageSpread(const Sample_t *sample,
                                double *spread_v)
{
    if (sample == NULL || spread_v == NULL)
    {
        return STATUS_NULL_POINTER;
    }

    double min_v, max_v;

    Status_t status = findCellVoltageExtremes(sample, &min_v, &max_v);

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    *spread_v = max_v - min_v;

    return STATUS_SUCCESS;
}

Status_t detectFault(const Sample_t *sample, FaultType_t *fault)
{
    if (sample == NULL || fault == NULL)
    {
        return STATUS_NULL_POINTER;
    }

    double min_v, max_v;

    Status_t status = findCellVoltageExtremes(sample, &min_v, &max_v);

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    double spread_v = max_v - min_v;

    *fault = FAULT_NONE;

    if (sample->temperature_c > FAULT_OVER_TEMPERATURE_C)
    {
        *fault = FAULT_OVER_TEMPERATURE;
    }
    else if (max_v > FAULT_OVER_VOLTAGE_V)
    {
        *fault = FAULT_OVER_VOLTAGE;
    }
    else if (min_v < FAULT_UNDER_VOLTAGE_V)
    {
        *fault = FAULT_UNDER_VOLTAGE;
    }
    else if (fabs(sample->current_a) > FAULT_OVER_CURRENT_A)
    {
        *fault = FAULT_OVER_CURRENT;
    }
    else if (spread_v > FAULT_IMBALANCE_V)
    {
        *fault = FAULT_IMBALANCE;
    }

    return STATUS_SUCCESS;
}

PackState_t determinePackState(double current_a, FaultType_t fault)
{
    if (fault != FAULT_NONE)
    {
        return PACK_FAULT;
    }
    else if (current_a > STANDBY_CURRENT_BAND_A)
    {
        return PACK_DISCHARGING;
    }
    else if (current_a < -STANDBY_CURRENT_BAND_A)
    {
        return PACK_CHARGING;
    }
    else
    {
        return PACK_STANDBY;
    }
}

const char *faultTypeToString(FaultType_t fault)
{
    switch (fault)
    {
        case FAULT_NONE:
            return "NONE";

        case FAULT_OVER_TEMPERATURE:
            return "OVER_TEMPERATURE";

        case FAULT_OVER_VOLTAGE:
            return "OVER_VOLTAGE";

        case FAULT_UNDER_VOLTAGE:
            return "UNDER_VOLTAGE";

        case FAULT_OVER_CURRENT:
            return "OVER_CURRENT";

        case FAULT_IMBALANCE:
            return "IMBALANCE";

        default:
            return "UNKNOWN";
    }
}

const char *packStateToString(PackState_t state)
{
    switch (state)
    {
        case PACK_FAULT:
            return "FAULT";

        case PACK_DISCHARGING:
            return "DISCHARGING";

        case PACK_CHARGING:
            return "CHARGING";

        case PACK_STANDBY:
            return "STANDBY";

        default:
            return "UNKNOWN";
    }
}