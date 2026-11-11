#ifndef EVENT_LOG_H
#define EVENT_LOG_H

#include "bms_config.h"
#include "status.h"
#include "diagnostics.h"

typedef struct
{
    double time_s;          /* when the fault began */
    FaultType_t type;       /* which fault */
    double soc_pct;         /* state of charge at that moment */
} FaultEvent_t;

typedef struct
{
    FaultEvent_t events[MAX_EVENTS];
    int count;              /* number of stored events */
} EventLog_t;

#ifdef __cplusplus
extern "C" {
#endif

/* Make the log empty. Returns: STATUS_SUCCESS, STATUS_NULL_POINTER */
Status_t initEventLog(EventLog_t *log);

/*
 * Append an event.
 * Returns: STATUS_SUCCESS, STATUS_NULL_POINTER,
 *          STATUS_INVALID_ARGUMENT (type is FAULT_NONE or not a real fault),
 *          STATUS_CAPACITY_FULL
 */
Status_t addFaultEvent(EventLog_t *log, double time_s, FaultType_t type, double soc_pct);

/*
 * Count events of one type. count: output.
 * Returns: STATUS_SUCCESS, STATUS_NULL_POINTER, STATUS_INVALID_ARGUMENT
 */
Status_t countEventsByType(const EventLog_t *log, FaultType_t type, int *count);

/*
 * Print events to the screen. FAULT_NONE as filter means "show all".
 * Returns: STATUS_SUCCESS, STATUS_NULL_POINTER, STATUS_NO_DATA (no events)
 */
Status_t printEventsByType(const EventLog_t *log, FaultType_t filter);

/*
 * Write the log to a text file: one header line, then one line per event.
 * Returns: STATUS_SUCCESS, STATUS_NULL_POINTER, STATUS_FILE_ERROR
 */
Status_t saveFaultLog(const char *path, const EventLog_t *log);

#ifdef __cplusplus
}
#endif

#endif
