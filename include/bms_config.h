#ifndef BMS_CONFIG_H
#define BMS_CONFIG_H

/* ---- Pack and storage sizes ---- */
#define CELLS_IN_SERIES            4       /* series cells in the pack */
#define NOMINAL_CAPACITY_AH        10.0    /* pack nominal capacity, Ah */
#define MAX_SAMPLES                3600    /* samples held in memory */
#define MAX_EVENTS                 256     /* fault events held in memory */
#define MAX_LINE_LENGTH            128     /* longest accepted text line */
#define MAX_PATH_LENGTH            128     /* longest accepted file name */

/* ---- Plausibility ranges: is the READING believable? ---- */
#define PLAUS_CELL_VOLTAGE_MIN_V   2.0
#define PLAUS_CELL_VOLTAGE_MAX_V   4.5
#define PLAUS_CURRENT_MIN_A        (-100.0)
#define PLAUS_CURRENT_MAX_A        100.0
#define PLAUS_TEMPERATURE_MIN_C    (-40.0)
#define PLAUS_TEMPERATURE_MAX_C    125.0

/* ---- Fault thresholds: is the BATTERY unsafe? ---- */
#define FAULT_OVER_VOLTAGE_V       4.20    /* fault if a cell is above this */
#define FAULT_UNDER_VOLTAGE_V      2.80    /* fault if a cell is below this */
#define FAULT_OVER_TEMPERATURE_C   60.0    /* fault if above this */
#define FAULT_OVER_CURRENT_A       30.0    /* fault if |current| above this */
#define FAULT_IMBALANCE_V          0.10    /* fault if cell spread above this */

/* ---- Pack state and state of charge ---- */
#define STANDBY_CURRENT_BAND_A     0.5     /* |current| at or below = standby */
#define SOC_EMPTY_VOLTAGE_V        3.0     /* cell voltage mapped to 0 % */
#define SOC_FULL_VOLTAGE_V         4.2     /* cell voltage mapped to 100 % */
#define SOC_MIN_PCT                0.0
#define SOC_MAX_PCT                100.0
#define SECONDS_PER_HOUR           3600.0

#endif
