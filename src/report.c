#include "report.h"
#include "soc.h"
#include <stdio.h>
#include <string.h>

Status_t analyseSession(const Sample_t samples[], int sample_count,
                        int rejected_count, EventLog_t *log,
                        SessionSummary_t *summary)
{
    if (samples == NULL || log == NULL || summary == NULL)
    {
        return STATUS_NULL_POINTER;
    }

    if (sample_count <= 0)
    {
        return STATUS_NO_DATA;
    }

    Status_t status = initEventLog(log);

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    memset(summary, 0, sizeof(SessionSummary_t));

    double min_v, max_v;

    status = findCellVoltageExtremes(&samples[0], &min_v, &max_v);

    if (status != STATUS_SUCCESS)
    {
        return status;
    }

    double soc = estimateInitialSoc(min_v);
    double start_soc = soc;

    double min_voltage = min_v;
    double max_voltage = max_v;

    double peak_temp = samples[0].temperature_c;

    double peak_current = (samples[0].current_a >= 0) ?
                          samples[0].current_a : -samples[0].current_a;

    FaultType_t prev_fault = FAULT_NONE;
    PackState_t state = PACK_STANDBY;

    for (int i = 0; i < sample_count; i++)
    {
        if (i > 0)
        {
            double dt = samples[i].time_s - samples[i - 1].time_s;

            soc = updateSoc(soc, samples[i - 1].current_a,
                            dt, NOMINAL_CAPACITY_AH);
        }

        double current_min, current_max;

        status = findCellVoltageExtremes(&samples[i],
                                         &current_min, &current_max);

        if (status != STATUS_SUCCESS)
        {
            return status;
        }

        FaultType_t fault;

        status = detectFault(&samples[i], &fault);

        if (status != STATUS_SUCCESS)
        {
            return status;
        }

        if (fault != FAULT_NONE && fault != prev_fault)
        {
            status = addFaultEvent(log, samples[i].time_s,
                                   fault, soc);

            if (status != STATUS_SUCCESS)
            {
                return status;
            }
        }

        prev_fault = fault;

        state = determinePackState(samples[i].current_a, fault);

        if (current_min < min_voltage)
        {
            min_voltage = current_min;
        }

        if (current_max > max_voltage)
        {
            max_voltage = current_max;
        }

        if (samples[i].temperature_c > peak_temp)
        {
            peak_temp = samples[i].temperature_c;
        }

        double abs_current = (samples[i].current_a >= 0) ?
                             samples[i].current_a : -samples[i].current_a;

        if (abs_current > peak_current)
        {
            peak_current = abs_current;
        }
    }

    summary->samples_loaded = sample_count;
    summary->samples_rejected = rejected_count;
    summary->duration_s = samples[sample_count - 1].time_s -
                          samples[0].time_s;
    summary->soc_start_pct = start_soc;
    summary->soc_end_pct = soc;
    summary->min_cell_v = min_voltage;
    summary->max_cell_v = max_voltage;
    summary->peak_temperature_c = peak_temp;
    summary->peak_current_a = peak_current;
    summary->final_state = state;

    for (int i = 1; i < FAULT_TYPE_COUNT; i++)
    {
        status = countEventsByType(log, (FaultType_t)i,
                                   &summary->fault_counts[i]);

        if (status != STATUS_SUCCESS)
        {
            return status;
        }
    }

    return STATUS_SUCCESS;
}

Status_t writeSummaryReport(const char *path,
                            const SessionSummary_t *summary)
{
    if (path == NULL || summary == NULL)
    {
        return STATUS_NULL_POINTER;
    }

    FILE *fp = fopen(path, "w");

    if (fp == NULL)
    {
        return STATUS_FILE_ERROR;
    }

    if (fprintf(fp, "BATTERY PACK SESSION SUMMARY\n") < 0)
    {
        fclose(fp);
        return STATUS_FILE_ERROR;
    }

    if (fprintf(fp, "----------------------------\n") < 0)
    {
        fclose(fp);
        return STATUS_FILE_ERROR;
    }

    if (fprintf(fp, "Samples Loaded: %d\n",
                summary->samples_loaded) < 0)
    {
        fclose(fp);
        return STATUS_FILE_ERROR;
    }

    if (fprintf(fp, "Samples Rejected: %d\n",
                summary->samples_rejected) < 0)
    {
        fclose(fp);
        return STATUS_FILE_ERROR;
    }

    if (fprintf(fp, "Duration: %.2f s\n",
                summary->duration_s) < 0)
    {
        fclose(fp);
        return STATUS_FILE_ERROR;
    }

    if (fprintf(fp, "Starting SoC: %.2f %%\n",
                summary->soc_start_pct) < 0)
    {
        fclose(fp);
        return STATUS_FILE_ERROR;
    }

    if (fprintf(fp, "Ending SoC: %.2f %%\n",
                summary->soc_end_pct) < 0)
    {
        fclose(fp);
        return STATUS_FILE_ERROR;
    }

    if (fprintf(fp, "Minimum Cell Voltage: %.2f V\n",
                summary->min_cell_v) < 0)
    {
        fclose(fp);
        return STATUS_FILE_ERROR;
    }

    if (fprintf(fp, "Maximum Cell Voltage: %.2f V\n",
                summary->max_cell_v) < 0)
    {
        fclose(fp);
        return STATUS_FILE_ERROR;
    }

    if (fprintf(fp, "Peak Temperature: %.2f C\n",
                summary->peak_temperature_c) < 0)
    {
        fclose(fp);
        return STATUS_FILE_ERROR;
    }

    if (fprintf(fp, "Peak Current: %.2f A\n",
                summary->peak_current_a) < 0)
    {
        fclose(fp);
        return STATUS_FILE_ERROR;
    }

    if (fprintf(fp, "Final Pack State: %s\n",
                packStateToString(summary->final_state)) < 0)
    {
        fclose(fp);
        return STATUS_FILE_ERROR;
    }

    if (fprintf(fp, "\nFAULT COUNTS\n") < 0)
    {
        fclose(fp);
        return STATUS_FILE_ERROR;
    }

    for (int i = 1; i < FAULT_TYPE_COUNT; i++)
    {
        if (fprintf(fp, "%s: %d\n",
                    faultTypeToString((FaultType_t)i),
                    summary->fault_counts[i]) < 0)
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