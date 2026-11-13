#include "event_log.h"
#include <stdio.h>

Status_t initEventLog(EventLog_t *log)
{
    if (log == NULL)
    {
        return STATUS_NULL_POINTER;
    }

    log->count = 0;

    return STATUS_SUCCESS;
}

Status_t addFaultEvent(EventLog_t *log, double time_s,
                       FaultType_t type, double soc_pct)
{
    if (log == NULL)
    {
        return STATUS_NULL_POINTER;
    }

    if (type <= FAULT_NONE || type >= FAULT_TYPE_COUNT)
    {
        return STATUS_INVALID_ARGUMENT;
    }

    if (log->count >= MAX_EVENTS)
    {
        return STATUS_CAPACITY_FULL;
    }

    log->events[log->count].time_s = time_s;
    log->events[log->count].type = type;
    log->events[log->count].soc_pct = soc_pct;

    log->count++;

    return STATUS_SUCCESS;
}

Status_t countEventsByType(const EventLog_t *log, FaultType_t type,
                           int *count)
{
    if (log == NULL || count == NULL)
    {
        return STATUS_NULL_POINTER;
    }

    if (type <= FAULT_NONE || type >= FAULT_TYPE_COUNT)
    {
        return STATUS_INVALID_ARGUMENT;
    }

    *count = 0;

    for (int i = 0; i < log->count; i++)
    {
        if (log->events[i].type == type)
        {
            (*count)++;
        }
    }

    return STATUS_SUCCESS;
}

Status_t printEventsByType(const EventLog_t *log, FaultType_t type)
{
    if (log == NULL)
    {
        return STATUS_NULL_POINTER;
    }

    int printed = 0;

    for (int i = 0; i < log->count; i++)
    {
        if (type == FAULT_NONE || log->events[i].type == type)
        {
            printf("Time: %.2f s | Fault: %s | SoC: %.2f %%\n",
                   log->events[i].time_s,
                   faultTypeToString(log->events[i].type),
                   log->events[i].soc_pct);

            printed++;
        }
    }

    if (printed == 0)
    {
        printf("No events found.\n");
        return STATUS_NO_DATA;
    }

    return STATUS_SUCCESS;
}

Status_t saveFaultLog(const char *path, const EventLog_t *log)
{
    if (path == NULL || log == NULL)
    {
        return STATUS_NULL_POINTER;
    }

    FILE *fp = fopen(path, "w");

    if (fp == NULL)
    {
        return STATUS_FILE_ERROR;
    }

    if (fprintf(fp, "Timestamp(s),Fault Type,SoC(%%)\n") < 0)
    {
        fclose(fp);
        return STATUS_FILE_ERROR;
    }

    for (int i = 0; i < log->count; i++)
    {
        if (fprintf(fp, "%.2f,%s,%.2f\n",
                    log->events[i].time_s,
                    faultTypeToString(log->events[i].type),
                    log->events[i].soc_pct) < 0)
        {
            fclose(fp);
            return STATUS_FILE_ERROR;
        }
    }

    if (fclose(fp) != 0)
    {
        return STATUS_FILE_ERROR;
    }

    return STATUS_SUCCESS;
}