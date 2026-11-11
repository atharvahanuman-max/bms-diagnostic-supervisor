#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H

#include "bms_config.h"
#include "status.h"
#include "sample.h"

typedef enum
{
    FAULT_NONE = 0,
    FAULT_OVER_VOLTAGE,
    FAULT_UNDER_VOLTAGE,
    FAULT_OVER_TEMPERATURE,
    FAULT_OVER_CURRENT,
    FAULT_IMBALANCE,
    FAULT_TYPE_COUNT           /* number of values; not a real fault */
} FaultType_t;

typedef enum
{
    PACK_STANDBY = 0,
    PACK_DISCHARGING,
    PACK_CHARGING,
    PACK_FAULT
} PackState_t;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Difference between highest and lowest cell voltage.
 * spread_v: output
 * Returns: STATUS_SUCCESS, STATUS_NULL_POINTER
 */
Status_t calculateVoltageSpread(const Sample_t *sample, double *spread_v);

/*
 * Decide the single most important fault in a sample.
 * Priority: over-temperature, over-voltage, under-voltage,
 * over-current, imbalance. Thresholds are strict (value must
 * pass the limit, not equal it). *fault is FAULT_NONE if healthy.
 * Returns: STATUS_SUCCESS, STATUS_NULL_POINTER
 */
Status_t detectFault(const Sample_t *sample, FaultType_t *fault);

/*
 * Pack state: any fault gives PACK_FAULT; otherwise the current
 * and STANDBY_CURRENT_BAND_A decide discharging, charging or standby.
 */
PackState_t determinePackState(double current_a, FaultType_t fault);

/* Text names for printing and file output (never NULL). */
const char *faultTypeToString(FaultType_t fault);
const char *packStateToString(PackState_t state);

#ifdef __cplusplus
}
#endif

#endif
