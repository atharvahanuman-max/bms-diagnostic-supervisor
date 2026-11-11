#ifndef SAMPLE_H
#define SAMPLE_H

#include "bms_config.h"
#include "status.h"

/* One time-stamped measurement of the battery pack. */
typedef struct
{
    double time_s;                        /* seconds since session start */
    double current_a;                     /* pack current, discharge positive */
    double temperature_c;                 /* pack temperature, degrees C */
    double cell_v[CELLS_IN_SERIES];       /* cell voltages, volts */
} Sample_t;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Convert one CSV text line into a sample.
 * line: input, "time_s,current_A,temp_C,v1,...,vN"
 * out : output, filled only when STATUS_SUCCESS is returned
 * Returns: STATUS_SUCCESS, STATUS_NULL_POINTER, STATUS_PARSE_ERROR
 */
Status_t parseSampleLine(const char *line, Sample_t *out);

/*
 * Check that every value of a sample is physically believable.
 * Returns: STATUS_SUCCESS, STATUS_NULL_POINTER, STATUS_OUT_OF_RANGE
 */
Status_t validateSampleRange(const Sample_t *sample);

/*
 * Find the lowest and highest cell voltage of a sample.
 * min_v, max_v: outputs
 * Returns: STATUS_SUCCESS, STATUS_NULL_POINTER
 */
Status_t findCellVoltageExtremes(const Sample_t *sample, double *min_v, double *max_v);

/*
 * Load a session file: skip the header, parse and validate each line.
 * Bad lines are counted in rejected_count and skipped.
 * samples: output array of size max_samples
 * Returns: STATUS_SUCCESS, STATUS_NULL_POINTER, STATUS_INVALID_ARGUMENT,
 *          STATUS_FILE_ERROR, STATUS_CAPACITY_FULL
 */
Status_t loadSamples(const char *path, Sample_t samples[], int max_samples,
                     int *loaded_count, int *rejected_count);

#ifdef __cplusplus
}
#endif

#endif
