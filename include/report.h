#ifndef REPORT_H
#define REPORT_H

#include "bms_config.h"
#include "status.h"
#include "sample.h"
#include "diagnostics.h"
#include "event_log.h"

typedef struct
{
    int samples_loaded;
    int samples_rejected;
    double duration_s;
    double soc_start_pct;
    double soc_end_pct;
    double min_cell_v;
    double max_cell_v;
    double peak_temperature_c;
    double peak_current_a;                     /* largest magnitude */
    int fault_counts[FAULT_TYPE_COUNT];        /* events per fault type */
    PackState_t final_state;
} SessionSummary_t;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Run the whole analysis over loaded samples: SoC tracking, fault
 * detection, pack state, event recording (an event is added when a
 * fault begins or changes type, not for every sample), statistics.
 * Returns: STATUS_SUCCESS, STATUS_NULL_POINTER,
 *          STATUS_NO_DATA (sample_count <= 0)
 */
Status_t analyseSession(const Sample_t samples[], int sample_count, int rejected_count,
                        EventLog_t *log, SessionSummary_t *summary);

/*
 * Write the summary as a readable text report.
 * Returns: STATUS_SUCCESS, STATUS_NULL_POINTER, STATUS_FILE_ERROR
 */
Status_t writeSummaryReport(const char *path, const SessionSummary_t *summary);

#ifdef __cplusplus
}
#endif

#endif
