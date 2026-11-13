#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "bms_config.h"
#include "status.h"
#include "sample.h"
#include "event_log.h"
#include "report.h"
#include "diagnostics.h"

static int readChoice(const char *prompt, int min, int max)
{
    char buffer[100];
    char *end;
    long value;

    printf("%s", prompt);

    if (fgets(buffer, sizeof(buffer), stdin) == NULL)
    {
        return -2;
    }

    if (strchr(buffer, '\n') == NULL && !feof(stdin))
    {
        int ch;

        while ((ch = getchar()) != '\n' && ch != EOF)
        {
        }

        return -1;
    }

    value = strtol(buffer, &end, 10);

    if (end == buffer)
    {
        return -1;
    }

    while (isspace((unsigned char)*end))
    {
        end++;
    }

    if (*end != '\0' || value < min || value > max)
    {
        return -1;
    }

    return (int)value;
}

static const char *statusMessage(Status_t status)
{
    switch (status)
    {
        case STATUS_SUCCESS:
            return "Operation completed successfully.";

        case STATUS_NULL_POINTER:
            return "Error: Required pointer is NULL.";

        case STATUS_INVALID_ARGUMENT:
            return "Error: Invalid argument.";

        case STATUS_PARSE_ERROR:
            return "Error: Data parsing failed.";

        case STATUS_OUT_OF_RANGE:
            return "Error: Value is out of range.";

        case STATUS_FILE_ERROR:
            return "Error: File operation failed.";

        case STATUS_CAPACITY_FULL:
            return "Error: Storage capacity is full.";

        case STATUS_NO_DATA:
            return "Error: Required data is not available.";

        default:
            return "Error: Unknown status.";
    }
}

int main(void)
{
    Sample_t samples[MAX_SAMPLES];
    EventLog_t log;
    SessionSummary_t summary;

    int loaded = 0;
    int rejected = 0;

    int data_loaded = 0;
    int analysed = 0;

    Status_t status = initEventLog(&log);

    printf("%s\n", statusMessage(status));

    if (status != STATUS_SUCCESS)
    {
        return 1;
    }

    while (1)
    {
        printf("\n========== BATTERY MANAGEMENT SYSTEM ==========\n");
        printf("1. Load Sample Data\n");
        printf("2. Analyse Session\n");
        printf("3. View Fault Events\n");
        printf("4. Save Fault Log\n");
        printf("5. Generate Summary Report\n");
        printf("0. Exit\n");
        printf("===============================================\n");

        int choice = readChoice("Enter your choice: ", 0, 5);

        if (choice == -2)
        {
            printf("\nEnd of input. Exiting program.\n");
            break;
        }

        if (choice == -1)
        {
            printf("Invalid input. Please enter a valid menu option.\n");
            continue;
        }

        switch (choice)
        {
            case 1:
            {
                char path[256];

                printf("Enter sample file path: ");

                if (fgets(path, sizeof(path), stdin) == NULL)
                {
                    printf("End of input. Exiting program.\n");
                    return 0;
                }

                if (strchr(path, '\n') == NULL && !feof(stdin))
                {
                    int ch;

                    while ((ch = getchar()) != '\n' && ch != EOF)
                    {
                    }

                    printf("File path is too long.\n");
                    break;
                }

                path[strcspn(path, "\r\n")] = '\0';

                if (strlen(path) == 0)
                {
                    printf("File path cannot be empty.\n");
                    break;
                }

                loaded = 0;
                rejected = 0;

                status = loadSamples(path, samples, MAX_SAMPLES,
                                     &loaded, &rejected);

                printf("%s\n", statusMessage(status));
                printf("Samples loaded: %d\n", loaded);
                printf("Samples rejected: %d\n", rejected);

                data_loaded = (status == STATUS_SUCCESS ||
                               status == STATUS_CAPACITY_FULL) &&
                              loaded > 0;

                analysed = 0;

                break;
            }

            case 2:
            {
                if (!data_loaded)
                {
                    printf("Please load sample data first.\n");
                    break;
                }

                status = analyseSession(samples, loaded, rejected,
                                         &log, &summary);

                printf("%s\n", statusMessage(status));

                analysed = (status == STATUS_SUCCESS);

                break;
            }

            case 3:
            {
                if (!analysed)
                {
                    printf("Please analyse the session first.\n");
                    break;
                }

                int filter = readChoice(
                    "Enter fault type (0 = All, 1-5 = Specific fault): ",
                    0, FAULT_TYPE_COUNT - 1);

                if (filter == -2)
                {
                    printf("\nEnd of input. Exiting program.\n");
                    return 0;
                }

                if (filter == -1)
                {
                    printf("Invalid fault type.\n");
                    break;
                }

                status = printEventsByType(&log, (FaultType_t)filter);

                printf("%s\n", statusMessage(status));

                break;
            }

            case 4:
            {
                if (!analysed)
                {
                    printf("Please analyse the session first.\n");
                    break;
                }

                status = saveFaultLog("fault_log.txt", &log);

                printf("%s\n", statusMessage(status));

                if (status == STATUS_SUCCESS)
                {
                    printf("Fault log saved to fault_log.txt\n");
                }

                break;
            }

            case 5:
            {
                if (!analysed)
                {
                    printf("Please analyse the session first.\n");
                    break;
                }

                status = writeSummaryReport("summary_report.txt",
                                            &summary);

                printf("%s\n", statusMessage(status));

                if (status == STATUS_SUCCESS)
                {
                    printf("Summary report saved to summary_report.txt\n");
                }

                break;
            }

            case 0:
            {
                printf("Exiting Battery Management System.\n");
                return 0;
            }

            default:
                printf("Invalid choice.\n");
                break;
        }
    }

    return 0;
}