# Detailed Design: BMS Diagnostic Supervisor (v2, simplified)

This document replaces the requirement list and module list in `project_proposal.md` (v1). The scope was reduced to fit the available time. Remove or update anything in the proposal that conflicts with this document.

> Educational software simulation only. Not an automotive-certified controller.

---

## 1. Scope Changes from Proposal v1

| Removed | Replaced by |
|---|---|
| Rate-of-change plausibility check | Range check only |
| Debounce and hysteresis | Plain strict-threshold comparison (listed as future work) |
| OCV table with interpolation | One straight-line voltage-to-SoC mapping |
| Pack state machine | One function that picks the state from current and fault |
| Event sorting | Filtering by type only |

Kept: plausibility vs fault limits, SoC by Coulomb counting, five fault types, pack state, fault log file, summary report, menu, gTest, Makefile.

---

## 2. Requirements

### 2.1 Functional Requirements

| ID | Requirement | Priority | Verification |
|---|---|---|---|
| FR-01 | The system shall load a battery session file containing time, pack current, temperature and cell voltages. | High | Demo, Test |
| FR-02 | The system shall report and skip malformed lines without terminating, and count them as rejected. | High | Test |
| FR-03 | The system shall reject a sample whose values lie outside the plausibility ranges. | High | Test |
| FR-04 | The system shall estimate initial state of charge from the lowest cell voltage using a linear mapping. | High | Test |
| FR-05 | The system shall update state of charge over time by integrating pack current. | High | Test |
| FR-06 | The system shall keep state of charge between 0 % and 100 %. | High | Test |
| FR-07 | The system shall detect cell over-voltage and under-voltage. | High | Test |
| FR-08 | The system shall detect over-temperature and over-current. | High | Test |
| FR-09 | The system shall detect cell imbalance when the voltage spread exceeds its limit. | Medium | Test |
| FR-10 | The system shall report one fault per sample using a fixed priority order. | Medium | Test |
| FR-11 | The system shall determine the pack state (standby, discharging, charging, fault) for each sample. | High | Test |
| FR-12 | The system shall record a fault event with time, type and SoC when a fault begins or changes type. | High | Test, Demo |
| FR-13 | The system shall display recorded fault events, all or filtered by type. | Medium | Demo |
| FR-14 | The system shall save the fault events to a text file. | High | Test, Demo |
| FR-15 | The system shall generate a session summary report file. | High | Demo |
| FR-16 | The system shall show a menu and reject invalid choices without crashing. | High | Negative testing |
| FR-17 | The system shall report a missing or unreadable file and return to the menu. | High | Test, Demo |
| FR-18 | The system shall report when sample or event storage is full instead of overflowing. | Medium | Test |

### 2.2 Non-Functional Requirements

| ID | Requirement | Verification |
|---|---|---|
| NFR-01 | The code shall compile without warnings (`-Wall -Wextra -Wpedantic`). | Build output |
| NFR-02 | The project shall use a modular multi-file structure. | Code review |
| NFR-03 | Invalid input shall not crash the application. | Negative testing |
| NFR-04 | Important functions shall be tested with gTest. | Test output |
| NFR-05 | The project shall build with a Makefile (`all`, `run`, `test`, `clean`). | Build demo |
| NFR-06 | The project shall be kept in a Git/GitHub repository. | Repository review |
| NFR-07 | All limits and sizes shall be named constants. | Code review |
| NFR-08 | The code shall not use unsafe input functions. | Code review |

---

## 3. Architecture

### 3.1 Modules and Dependencies

| Module | Files | Responsibility |
|---|---|---|
| Sample | `sample.c/.h` | Data type, text parsing, plausibility check, file loading |
| SoC | `soc.c/.h` | Initial SoC estimate and Coulomb counting |
| Diagnostics | `diagnostics.c/.h` | Fault detection, voltage spread, pack state, enum names |
| Event log | `event_log.c/.h` | Store, count, print and save fault events |
| Report | `report.c/.h` | Run the full analysis and write the summary report |
| Main | `main.c` | Menu and high-level workflow only |
| Shared headers | `bms_config.h`, `status.h` | Constants and status codes |

```
main.c --> report --> event_log --> diagnostics --> sample --> (bms_config, status)
              |                          
              +--> soc
```
Lower modules never include higher ones. `report` is the only module that combines the others.

### 3.2 Main Workflow

1. Start: create the sample array, an empty event log, an empty summary, and state flags (`data_loaded`, `analysed`).
2. Show the menu: 1 Load session file, 2 Run analysis, 3 View fault events, 4 Save fault log, 5 Write summary report, 0 Exit.
3. Read the choice as text, convert it safely, validate the range.
4. Call the matching module function and print a success or error message based on the returned `Status_t`.
5. Options 2 to 5 refuse to run until their prerequisite is done (`STATUS_NO_DATA`).
6. Repeat until the user exits.

---

## 4. Data Design

### 4.1 Constants

All are defined in `include/bms_config.h`: pack and storage sizes, plausibility ranges, fault thresholds, standby band, SoC mapping voltages.

| Group | Constants |
|---|---|
| Sizes | `CELLS_IN_SERIES` 4, `NOMINAL_CAPACITY_AH` 10.0, `MAX_SAMPLES` 3600, `MAX_EVENTS` 256, `MAX_LINE_LENGTH` 128, `MAX_PATH_LENGTH` 128 |
| Plausibility | Cell voltage 2.0 to 4.5 V, current -100 to 100 A, temperature -40 to 125 C |
| Fault thresholds | Over-voltage above 4.20 V, under-voltage below 2.80 V, over-temperature above 60 C, over-current above 30 A magnitude, imbalance spread above 0.10 V |
| State and SoC | Standby band 0.5 A, SoC 0 % at 3.0 V and 100 % at 4.2 V |

These are illustrative values for a generic Li-ion pack and are stated as assumptions.

### 4.2 Types

| Type | Kind | Purpose |
|---|---|---|
| `Status_t` | enum | Result code of every function that can fail |
| `FaultType_t` | enum | NONE, OVER_VOLTAGE, UNDER_VOLTAGE, OVER_TEMPERATURE, OVER_CURRENT, IMBALANCE, plus `FAULT_TYPE_COUNT` sentinel |
| `PackState_t` | enum | STANDBY, DISCHARGING, CHARGING, FAULT |
| `Sample_t` | struct | One time-stamped measurement |
| `FaultEvent_t` | struct | Time, fault type, SoC at the moment a fault began |
| `EventLog_t` | struct | Fixed array of events plus a count |
| `SessionSummary_t` | struct | Statistics produced by the analysis |

### 4.3 Data Dictionary (Sample_t)

| Item | C type | Valid range | Validation rule | Description |
|---|---|---|---|---|
| time_s | double | 0 or more | Must parse as a number | Seconds since session start |
| current_a | double | -100.0 to 100.0 | Plausibility range | Pack current, discharge positive |
| temperature_c | double | -40.0 to 125.0 | Plausibility range | Pack temperature |
| cell_v[4] | double[4] | 2.0 to 4.5 each | Plausibility range for every cell | Cell voltages |

---

## 5. Function Specifications

### 5.1 Sample module

| Function | Purpose and rules | Return values | FR |
|---|---|---|---|
| `Status_t parseSampleLine(const char *line, Sample_t *out)` | Read exactly 3 + `CELLS_IN_SERIES` comma-separated numbers. Too few fields, extra fields, non-numeric text or an empty line is a parse error. Allow a trailing newline. Do not range-check here. `out` is changed only on success. | SUCCESS, NULL_POINTER, PARSE_ERROR | FR-01, FR-02 |
| `Status_t validateSampleRange(const Sample_t *sample)` | Check every cell voltage, current and temperature against the plausibility range. Limits themselves are valid. | SUCCESS, NULL_POINTER, OUT_OF_RANGE | FR-03 |
| `Status_t findCellVoltageExtremes(const Sample_t *sample, double *min_v, double *max_v)` | Lowest and highest cell voltage. | SUCCESS, NULL_POINTER | FR-04, FR-09 |
| `Status_t loadSamples(const char *path, Sample_t samples[], int max_samples, int *loaded_count, int *rejected_count)` | Open the file, skip the header line, skip blank lines, parse and validate each line. Bad lines increment `rejected_count` and are skipped. Stop with CAPACITY_FULL if the array is full. Always close the file. Counts are set even on early return. | SUCCESS, NULL_POINTER, INVALID_ARGUMENT (`max_samples` <= 0), FILE_ERROR, CAPACITY_FULL | FR-01, FR-02, FR-03, FR-17, FR-18 |

*Hints:* `fgets` with a `MAX_LINE_LENGTH` buffer; `strtod` with its end pointer detects text such as `12abc`; think about a line longer than the buffer.

### 5.2 SoC module

| Function | Purpose and rules | FR |
|---|---|---|
| `double estimateInitialSoc(double cell_voltage_v)` | Straight line: 0 % at 3.0 V, 100 % at 4.2 V, clamped. Example: 3.6 V gives 50 %. | FR-04, FR-06 |
| `double updateSoc(double soc_pct, double current_a, double dt_s, double capacity_ah)` | Charge removed in percent = (current x dt / 3600) / capacity x 100. Subtract it (discharge positive, so charging raises SoC). Clamp to 0..100. If capacity is not positive or `dt_s` is negative, return the old SoC (clamped). | FR-05, FR-06 |

### 5.3 Diagnostics module

| Function | Purpose and rules | Return values | FR |
|---|---|---|---|
| `Status_t calculateVoltageSpread(const Sample_t *sample, double *spread_v)` | Highest minus lowest cell voltage. | SUCCESS, NULL_POINTER | FR-09 |
| `Status_t detectFault(const Sample_t *sample, FaultType_t *fault)` | Set `*fault` to the first matching fault in this priority: over-temperature, over-voltage, under-voltage, over-current (use the magnitude of the current), imbalance. Comparisons are strict, so a value exactly at the limit is healthy. No fault gives `FAULT_NONE`. | SUCCESS, NULL_POINTER | FR-07, FR-08, FR-09, FR-10 |
| `PackState_t determinePackState(double current_a, FaultType_t fault)` | Any fault gives FAULT. Otherwise current above the standby band is DISCHARGING, below minus the band is CHARGING, else STANDBY. | n/a | FR-11 |
| `const char *faultTypeToString(FaultType_t fault)` | Readable name; an unknown value returns a fixed fallback text, never NULL. | n/a | FR-13, FR-14 |
| `const char *packStateToString(PackState_t state)` | Same for pack states. | n/a | FR-15 |

### 5.4 Event log module

| Function | Purpose and rules | Return values | FR |
|---|---|---|---|
| `Status_t initEventLog(EventLog_t *log)` | Set count to zero. | SUCCESS, NULL_POINTER | FR-12 |
| `Status_t addFaultEvent(EventLog_t *log, double time_s, FaultType_t type, double soc_pct)` | Append an event. `FAULT_NONE` and values outside the real fault range are invalid. Check capacity before writing. | SUCCESS, NULL_POINTER, INVALID_ARGUMENT, CAPACITY_FULL | FR-12, FR-18 |
| `Status_t countEventsByType(const EventLog_t *log, FaultType_t type, int *count)` | Number of events of one type. | SUCCESS, NULL_POINTER, INVALID_ARGUMENT | FR-13 |
| `Status_t printEventsByType(const EventLog_t *log, FaultType_t filter)` | Print matching events; `FAULT_NONE` means all. Say clearly when there is nothing to show. | SUCCESS, NULL_POINTER, NO_DATA | FR-13 |
| `Status_t saveFaultLog(const char *path, const EventLog_t *log)` | Write one header line and one line per event: time, fault name, SoC. Check that the file opened and closed correctly. | SUCCESS, NULL_POINTER, FILE_ERROR | FR-14, FR-17 |

### 5.5 Report module

| Function | Purpose and rules | Return values | FR |
|---|---|---|---|
| `Status_t analyseSession(const Sample_t samples[], int sample_count, int rejected_count, EventLog_t *log, SessionSummary_t *summary)` | See the algorithm below. | SUCCESS, NULL_POINTER, NO_DATA | FR-04 to FR-12 |
| `Status_t writeSummaryReport(const char *path, const SessionSummary_t *summary)` | Write loaded and rejected counts, duration, SoC start and end, min and max cell voltage, peak temperature and current, fault counts by type, final pack state. | SUCCESS, NULL_POINTER, FILE_ERROR | FR-15 |

**`analyseSession` algorithm (in words):**
1. Reject null pointers and `sample_count` <= 0. Reset the log and the summary.
2. Initial SoC from the lowest cell voltage of the first sample.
3. For each sample: detect its fault; if the fault is not NONE and differs from the fault of the previous sample, add an event (a fault that continues over many samples is one event); work out the pack state; if this is not the first sample, update SoC using the real time difference to the previous sample (the sample file contains gaps); update the minimum and maximum cell voltage, the peak temperature and the largest current magnitude.
4. Fill the summary: counts per fault type (use `countEventsByType`), duration (last time minus first time), final SoC and final state.

### 5.6 Main

`main.c` only owns the data (sample array, event log, summary, flags), shows the menu, reads a choice with `fgets` plus a safe conversion, calls the module functions, and prints messages based on the returned status. It must contain no calculations.

---

## 6. Input Validation Plan

| Input | Source | Rule | Invalid example | Behaviour |
|---|---|---|---|---|
| Menu choice | User | Whole number 0 to 5 | `7`, `abc`, empty | Message, show menu again |
| File name | User | Not empty, shorter than `MAX_PATH_LENGTH` | empty | Message, return to menu |
| CSV line | File | Exactly 7 numeric fields | `12,abc,...`, 5 fields | Count as rejected, continue |
| Sample values | File | Inside plausibility ranges | temperature 300 | Count as rejected, continue |
| Fault type argument | Code | Real fault (1 to `FAULT_TYPE_COUNT` - 1) | `FAULT_NONE` for an event | `INVALID_ARGUMENT` |
| Pointers | Code | Not NULL | `NULL` | `NULL_POINTER`, no dereference |

## 7. File Design

| File | Purpose | Format | Created by | Read by |
|---|---|---|---|---|
| `data/sample_session.csv` | Input session | Text, one header line then `time_s,current_A,temp_C,v1,v2,v3,v4` | Student (provided) | `loadSamples` |
| `fault_log.txt` | Fault events | Text, header line then `time_s,fault,soc_pct` | `saveFaultLog` | User |
| `summary_report.txt` | Session summary | Plain text | `writeSummaryReport` | User |

| Error condition | Detection | Behaviour |
|---|---|---|
| File does not exist or cannot open | `fopen` returns NULL | `STATUS_FILE_ERROR`, message, menu continues |
| File empty or only a header | Zero samples loaded | Analysis refuses with `STATUS_NO_DATA` |
| Invalid line | Parse or range failure | Counted as rejected |
| Write or close fails | Return value of `fprintf` / `fclose` | `STATUS_FILE_ERROR` |

## 8. Error Catalogue

| Error | Where detected | Status | User-facing behaviour |
|---|---|---|---|
| Null pointer argument | Any function | `STATUS_NULL_POINTER` | Function returns, no crash |
| Bad argument value | `loadSamples`, `addFaultEvent`, `countEventsByType` | `STATUS_INVALID_ARGUMENT` | Message |
| Malformed line | `parseSampleLine` | `STATUS_PARSE_ERROR` | Counted as rejected |
| Implausible value | `validateSampleRange` | `STATUS_OUT_OF_RANGE` | Counted as rejected |
| File failure | `loadSamples`, `saveFaultLog`, `writeSummaryReport` | `STATUS_FILE_ERROR` | Message |
| Storage full | `loadSamples`, `addFaultEvent` | `STATUS_CAPACITY_FULL` | Message, keep what was stored |
| Action before data | `analyseSession`, menu | `STATUS_NO_DATA` | Message asking to load or analyse first |

---

## 9. Unit-Test Plan

Test files: `tests/test_sample.cpp`, `tests/test_soc.cpp`, `tests/test_diagnostics.cpp`, `tests/test_event_log.cpp`. Include the C headers normally (they already carry the C linkage guards). Tests that write files must use a temporary file and delete it at the end. Minimum for the guideline is 10 tests on 3 functions; this plan has 20 so you can drop the weakest.

| Test ID | Function | Category | Condition | Expected |
|---|---|---|---|---|
| UT-01 | parseSampleLine | Positive | `"5,12.0,30.0,3.9,3.9,3.9,3.9"` | SUCCESS, fields equal the input |
| UT-02 | parseSampleLine | Negative | Only 5 fields | PARSE_ERROR |
| UT-03 | parseSampleLine | Negative | Text in the current field | PARSE_ERROR |
| UT-04 | parseSampleLine | Negative | NULL line or NULL output | NULL_POINTER |
| UT-05 | validateSampleRange | Positive | All values inside | SUCCESS |
| UT-06 | validateSampleRange | Negative | One cell at 1.5 V | OUT_OF_RANGE |
| UT-07 | validateSampleRange | Boundary | Cell exactly 2.0 V | SUCCESS |
| UT-08 | validateSampleRange | Boundary | Temperature just above 125 | OUT_OF_RANGE |
| UT-09 | estimateInitialSoc | Boundary | 3.0 V, 4.2 V | 0 %, 100 % |
| UT-10 | estimateInitialSoc | Positive | 3.6 V | 50 % |
| UT-11 | estimateInitialSoc | Boundary | 2.0 V and 5.0 V (outside mapping) | Clamped to 0 and 100 |
| UT-12 | updateSoc | Positive | 50 %, 10 A, 360 s, 10 Ah | 40 % |
| UT-13 | updateSoc | Boundary | Discharge past empty | Stays at 0 % |
| UT-14 | updateSoc | Positive | Negative current (charging) | SoC rises, never above 100 % |
| UT-15 | detectFault | Positive | A cell at 4.25 V, others normal | FAULT_OVER_VOLTAGE |
| UT-16 | detectFault | Boundary | Cell exactly 4.20 V | FAULT_NONE |
| UT-17 | detectFault | Positive | Over-temperature and over-voltage together | FAULT_OVER_TEMPERATURE (priority) |
| UT-18 | detectFault | Negative | NULL sample | NULL_POINTER |
| UT-19 | determinePackState | Positive | Current -5 A, no fault | PACK_CHARGING |
| UT-20 | addFaultEvent | Negative | Add to a full log | CAPACITY_FULL |

Optional extras: `saveFaultLog` writing to a temporary file and reading it back; `countEventsByType`.

---

## 10. Requirement Traceability

| Requirement | Module | Functions | Tests |
|---|---|---|---|
| FR-01, FR-02, FR-03 | sample | `parseSampleLine`, `validateSampleRange`, `loadSamples` | UT-01 to UT-08 |
| FR-04, FR-06 | soc | `estimateInitialSoc`, `updateSoc` | UT-09 to UT-11, UT-13, UT-14 |
| FR-05 | soc | `updateSoc` | UT-12 |
| FR-07, FR-08, FR-09, FR-10 | diagnostics | `detectFault`, `calculateVoltageSpread` | UT-15 to UT-18 |
| FR-11 | diagnostics | `determinePackState` | UT-19 |
| FR-12, FR-18 | event_log, report | `addFaultEvent`, `analyseSession` | UT-20 |
| FR-13, FR-14 | event_log | `printEventsByType`, `saveFaultLog` | Demonstration |
| FR-15 | report | `writeSummaryReport` | Demonstration |
| FR-16, FR-17 | main, sample | menu, `loadSamples` | Negative testing |

---

## 11. Expected Behaviour of the Sample File (hand-calculated, verify with your program)

`data/sample_session.csv` has 30 data lines. Two are invalid on purpose (line at time 12 has text, line at time 16 has 300 C). Expected: 28 loaded, 2 rejected, initial SoC about 78.3 %, and five fault events, one of each type: over-current at 8 s, imbalance at 13 s, over-temperature at 20 s, over-voltage at 26 s, under-voltage at 29 s. The sample at 28 s has a cell exactly at 4.20 V and must **not** be a fault.

If your program disagrees, find out whether the data, the expectation or the code is wrong before changing anything.

---

## 12. Makefile Plan (you write it)

| Item | What to include |
|---|---|
| Variables | `CC`, `CXX`, `CFLAGS` (`-std=c11 -Wall -Wextra -Wpedantic -Iinclude -MMD -MP`), `CXXFLAGS` (same warnings, `-std=c++17`), `TEST_LIBS` (`-lgtest -lgtest_main -pthread`), `BUILD_DIR` |
| Source lists | `src/*.c` objects into `build/`; the library objects are all of them **except** `main.o` |
| Rules | Pattern rule `build/%.o: src/%.c`, another for `tests/%.cpp`, the build folder as a target |
| Dependencies | `-include` the generated `.d` files so a changed header rebuilds objects |
| Targets | `all` builds the app, `run` runs it, `test` builds and runs the test executable (linked with library objects, not `main.o`), `clean` removes `build/`, and a `.PHONY` line |

Build the app and the tests separately first, by hand, then turn the commands into rules.

## 13. Repository Structure and Git Plan

```
bms-diagnostic-supervisor/
|-- src/      main.c sample.c soc.c diagnostics.c event_log.c report.c
|-- include/  bms_config.h status.h sample.h soc.h diagnostics.h event_log.h report.h
|-- tests/    test_sample.cpp test_soc.cpp test_diagnostics.cpp test_event_log.cpp
|-- data/     sample_session.csv
|-- docs/     project_proposal.md detailed_design.md architecture.png
|-- Makefile  README.md  .gitignore  LICENSE
```

Planned commits: *Create initial project structure*, *Add project proposal and design*, *Add shared headers*, *Implement sample parsing and validation*, *Add sample unit tests*, *Implement SoC module and tests*, *Implement diagnostics and tests*, *Implement event log*, *Implement session analysis and report*, *Add main menu*, *Complete Makefile*, *Add README*.

`.gitignore` should cover `build/`, `*.o`, `*.d`, generated `fault_log.txt` and `summary_report.txt`.

## 14. Implementation Order

1. Copy the headers into `include/`, copy the sample data, run the header syntax check.
2. `sample.c`, then `tests/test_sample.cpp`.
3. `soc.c` and its tests.
4. `diagnostics.c` and its tests.
5. `event_log.c` and its tests.
6. `report.c`.
7. `main.c`.
8. Finish the Makefile, run all four targets, check for warnings.
9. README, architecture picture, results, fresh-clone check.
