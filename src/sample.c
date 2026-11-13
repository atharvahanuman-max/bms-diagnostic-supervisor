
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "bms_config.h"
#include "status.h"
#include "sample.h"

Status_t validateSampleRange(const Sample_t *sample)
{
    if (sample == NULL)
    {
        return STATUS_NULL_POINTER;
    }

    for (int i = 0; i < CELLS_IN_SERIES; i++)
    {
        if (!(sample->cell_v[i] >= PLAUS_CELL_VOLTAGE_MIN_V &&
              sample->cell_v[i] <= PLAUS_CELL_VOLTAGE_MAX_V))
        {
            return STATUS_OUT_OF_RANGE;
        }
    }

    if (!(sample->current_a >= PLAUS_CURRENT_MIN_A &&
          sample->current_a <= PLAUS_CURRENT_MAX_A))
    {
        return STATUS_OUT_OF_RANGE;
    }

    if (!(sample->temperature_c >= PLAUS_TEMPERATURE_MIN_C &&
          sample->temperature_c <= PLAUS_TEMPERATURE_MAX_C))
    {
        return STATUS_OUT_OF_RANGE;
    }

    return STATUS_SUCCESS;
}

Status_t findCellVoltageExtremes(const Sample_t *sample,
                                 double *min_v, double *max_v)
{
    if (sample == NULL || min_v == NULL || max_v == NULL)
    {
        return STATUS_NULL_POINTER;
    }

    *min_v = sample->cell_v[0];
    *max_v = sample->cell_v[0];

    for (int i = 1; i < CELLS_IN_SERIES; i++)
    {
        if (sample->cell_v[i] < *min_v)
        {
            *min_v = sample->cell_v[i];
        }

        if (sample->cell_v[i] > *max_v)
        {
            *max_v = sample->cell_v[i];
        }
    }

    return STATUS_SUCCESS;
}

Status_t parseSampleLine(const char *line, Sample_t *out)
{
    if (line == NULL || out == NULL)
    {
        return STATUS_NULL_POINTER;
    }

    double values[SAMPLE_FIELD_COUNT];
    char *end;
    const char *p = line;

    for (int i = 0; i < SAMPLE_FIELD_COUNT; i++)
    {
        values[i] = strtod(p, &end);

        if (end == p)
        {
            return STATUS_PARSE_ERROR;
        }

        if (i < SAMPLE_FIELD_COUNT - 1)
        {
            if (*end != ',')
            {
                return STATUS_PARSE_ERROR;
            }

            p = end + 1;
        }
        else
        {
            while (*end == ' ' || *end == '\r' || *end == '\n')
            {
                end++;
            }

            if (*end != '\0')
            {
                return STATUS_PARSE_ERROR;
            }
        }
    }

    Sample_t temp;

    temp.time_s = values[0];
    temp.current_a = values[1];
    temp.temperature_c = values[2];

    for (int i = 3; i < SAMPLE_FIELD_COUNT; i++)
    {
        temp.cell_v[i - 3] = values[i];
    }

    *out = temp;

    return STATUS_SUCCESS;
}

Status_t loadSamples(const char *path, Sample_t *samples,
                     int max_samples, int *loaded_count,
                     int *rejected_count)
{
    if (path == NULL || samples == NULL ||
        loaded_count == NULL || rejected_count == NULL)
    {
        return STATUS_NULL_POINTER;
    }

    if (max_samples <= 0)
    {
        return STATUS_INVALID_ARGUMENT;
    }

    *loaded_count = 0;
    *rejected_count = 0;

    FILE *fp = fopen(path, "r");

    if (fp == NULL)
    {
        return STATUS_FILE_ERROR;
    }

    char line[MAX_LINE_LENGTH];

    if (fgets(line, MAX_LINE_LENGTH, fp) == NULL)
    {
        fclose(fp);
        return STATUS_SUCCESS;
    }

    while (fgets(line, MAX_LINE_LENGTH, fp) != NULL)
    {
        int blank = 1;

        for (int i = 0; line[i] != '\0'; i++)
        {
            if (!isspace((unsigned char)line[i]))
            {
                blank = 0;
                break;
            }
        }

        if (blank)
        {
            continue;
        }

        if (strchr(line, '\n') == NULL && !feof(fp))
        {
            (*rejected_count)++;

            int ch;

            while ((ch = getc(fp)) != '\n' && ch != EOF)
            {
            }

            continue;
        }

        Sample_t sample;

        if (parseSampleLine(line, &sample) != STATUS_SUCCESS)
        {
            (*rejected_count)++;
            continue;
        }

        if (validateSampleRange(&sample) != STATUS_SUCCESS)
        {
            (*rejected_count)++;
            continue;
        }

        if (*loaded_count >= max_samples)
        {
            fclose(fp);
            return STATUS_CAPACITY_FULL;
        }

        samples[*loaded_count] = sample;
        (*loaded_count)++;
    }

    fclose(fp);

    return STATUS_SUCCESS;
}
