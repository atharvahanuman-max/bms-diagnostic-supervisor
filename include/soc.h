#ifndef SOC_H
#define SOC_H

#include "bms_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Initial state of charge (percent) from the lowest cell voltage,
 * using a straight line from SOC_EMPTY_VOLTAGE_V to SOC_FULL_VOLTAGE_V.
 * Result is limited to SOC_MIN_PCT .. SOC_MAX_PCT.
 */
double estimateInitialSoc(double cell_voltage_v);

/*
 * New state of charge (percent) after dt_s seconds at current_a
 * (discharge positive) for a pack of capacity_ah. Limited to 0..100.
 * If capacity_ah <= 0 or dt_s < 0, soc_pct is returned unchanged.
 */
double updateSoc(double soc_pct, double current_a, double dt_s, double capacity_ah);

#ifdef __cplusplus
}
#endif

#endif
