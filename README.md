# BMS Diagnostic Supervisor

**A C-based Battery Management System Diagnostic Simulation**

## 1. About the Project

As part of the Embedded Automotive Systems minor course, I developed this project to understand how some of the basic monitoring and diagnostic functions of a Battery Management System (BMS) can be implemented using C programming.

A Battery Management System plays an important role in electric vehicles. It continuously monitors battery parameters such as cell voltage, current and temperature to ensure that the battery operates within safe limits.

The idea behind this project was to develop a simple software-based simulation that takes battery operating data as input, analyses it and identifies possible abnormal conditions. It also estimates the State of Charge (SoC), keeps a record of detected faults and generates a summary of the battery session.

The project is mainly focused on applying C programming concepts, modular programming, file handling, error handling and software testing to an automotive-related application.

**Note:** This is an educational software simulation. It is not a real-time BMS and is not intended to be used for controlling or protecting an actual battery pack.

## 2. What Does the Project Do?

The program takes battery data from a CSV file. Each sample contains a timestamp, current, temperature and the voltage readings of four individual cells.

Using this data, the program performs the following operations:

- Reads battery samples from a CSV file.
- Checks whether the input data is valid and within the defined limits.
- Finds the minimum and maximum cell voltages.
- Calculates the voltage difference between the cells.
- Estimates the initial State of Charge and updates it using Coulomb counting.
- Checks for different types of battery faults.
- Determines the overall operating state of the battery pack.
- Records fault events along with their timestamps.
- Generates a summary report after analysing the session.

The program uses a menu-driven interface, so these operations can be performed individually.

## 3. Fault Detection

One of the main parts of this project is identifying abnormal battery operating conditions.

I have implemented five basic fault conditions based on predefined thresholds.

| Fault | Condition |
|---|---|
| Overvoltage | Maximum cell voltage greater than 4.20 V |
| Undervoltage | Minimum cell voltage less than 2.80 V |
| Overtemperature | Temperature greater than 60°C |
| Overcurrent | Absolute current greater than 30 A |
| Cell Imbalance | Difference between maximum and minimum cell voltage greater than 0.10 V |

If more than one fault condition is present in a sample, the program follows a predefined priority order and reports only the highest-priority fault.

The comparisons are strict. For example, a cell voltage of exactly 4.20 V does not trigger an overvoltage fault.

## 4. State of Charge (SoC) Estimation

Another important part of the project is estimating the battery's State of Charge.

I have implemented two approaches for this.

**Initial SoC estimation using voltage**

The initial SoC is calculated using a simple linear relationship between cell voltage and SoC.

- 3.0 V represents 0% SoC.
- 4.2 V represents 100% SoC.

This is a simplified estimation method used for the purpose of this project.

**SoC update using Coulomb counting**

After calculating the initial SoC, the program updates it using the measured battery current and the time difference between consecutive samples.

The equation used is:

\[
SoC_{new}=SoC_{old}-\frac{I\Delta t}{3600C}\times100
\]

Where:

- \(I\) is the battery current in amperes.
- \(\Delta t\) is the time interval in seconds.
- \(C\) is the nominal battery capacity in Ah.

Positive current represents discharging, while negative current represents charging. The estimated SoC is always restricted to the range of 0–100%.

## 5. Project Structure

I divided the program into different modules instead of writing the entire application in a single C file. This makes the code easier to understand, test and modify.

Each module has a separate responsibility:

| File | Purpose |
|---|---|
| `sample.c` | Reading CSV data, parsing samples and validating input |
| `soc.c` | Initial SoC estimation and Coulomb counting |
| `diagnostics.c` | Fault detection and battery pack state determination |
| `event_log.c` | Storing and managing detected fault events |
| `report.c` | Analysing the session and generating summary reports |
| `main.c` | Main program, menu and integration of all modules |

The corresponding header files are maintained separately in the `include/` directory. They contain the function declarations, data types and configuration constants required by the different modules.

### Repository Structure

```text
bms-diagnostic-supervisor/
│
├── include/
│   ├── bms_config.h
│   ├── diagnostics.h
│   ├── event_log.h
│   ├── report.h
│   ├── sample.h
│   ├── soc.h
│   └── status.h
│
├── src/
│   ├── diagnostics.c
│   ├── event_log.c
│   ├── main.c
│   ├── report.c
│   ├── sample.c
│   └── soc.c
│
├── tests/
│   ├── test_diagnostics.cpp
│   ├── test_event_log.cpp
│   ├── test_sample.cpp
│   └── test_soc.cpp
│
├── data/
│   └── sample_session.csv
│
├── docs/
│   └── detailed_design.md
│
├── Makefile
├── README.md
├── LICENSE
└── .gitignore
```

## 6. Technologies and Tools Used

The following tools and technologies were used while developing the project:

- **C Programming:** Main language used to implement the application.
- **C++ and Google Test:** Used to write and execute automated unit tests for the C modules.
- **GCC and G++:** Compilers used to build the application and test programs.
- **GNU Make:** Used to automate compilation and linking through a Makefile.
- **Ubuntu on WSL:** Development and execution environment.
- **Git and GitHub:** Used for version control and maintaining the project repository.

## 7. How to Run the Project

The project can be built and executed on an Ubuntu-based Linux environment.

First, install the required packages:

```bash
sudo apt update
sudo apt install build-essential libgtest-dev
```

Clone the repository:

```bash
git clone https://github.com/atharvahanuman-max/bms-diagnostic-supervisor.git
cd bms-diagnostic-supervisor
```

Compile the application:

```bash
make all
```

Run the application:

```bash
make run
```

The program displays a menu with the following options:

| Option | Operation |
|---|---|
| 1 | Load Sample Data |
| 2 | Analyse Session |
| 3 | View Fault Events |
| 4 | Save Fault Log |
| 5 | Generate Summary Report |
| 0 | Exit |

When asked for the input CSV file, enter:

```text
data/sample_session.csv
```

The session needs to be loaded and analysed before viewing the corresponding fault events or generating reports.

## 8. Testing

As part of the Embedded Automotive Systems course, we were also introduced to Makefiles and Google Test. I used these concepts to build and test the project.

Instead of checking only whether the complete program runs, I wrote separate unit tests for the individual modules.

The tests cover:

- CSV sample parsing and input validation.
- Cell voltage minimum and maximum calculations.
- Initial SoC estimation.
- Coulomb counting and SoC limits.
- Different fault conditions and their priority.
- Battery pack state determination.
- Fault event logging and event log capacity.

There are **20 Google Test cases**, and all 20 passed during testing.

To execute the tests, run:

```bash
make test
```

## 9. Sample Output and Results

I created a sample CSV dataset containing both normal operating values and some deliberately introduced abnormal readings. This helped me check whether the diagnostic functions were identifying faults correctly.

The sample session produced the following results:

| Parameter | Result |
|---|---:|
| Samples successfully loaded | 28 |
| Rejected samples | 2 |
| Session duration | 29 seconds |
| Initial SoC | 78.33% |
| Final SoC | 77.56% |
| Minimum cell voltage | 2.75 V |
| Maximum cell voltage | 4.26 V |
| Peak temperature | 64°C |
| Peak current magnitude | 35 A |
| Final pack state | FAULT |

The following fault events were detected during the session:

| Time | Detected fault |
|---|---|
| 8 s | Overcurrent |
| 13 s | Cell Imbalance |
| 20 s | Overtemperature |
| 26 s | Overvoltage |
| 29 s | Undervoltage |

The two rejected samples were intentionally included in the input dataset to check the program's handling of invalid data.

## 10. Limitations and Future Improvements

Since this project was developed as a software simulation, some practical aspects of an actual BMS are outside its scope.

For example, the current version does not include physical sensor interfacing, real-time battery monitoring, CAN communication or direct control of battery protection hardware.

The SoC estimation is also simplified. Actual battery applications may require more advanced estimation techniques and battery-specific characteristics.

Some improvements that could be explored in future versions are:

- Interfacing the software with a microcontroller and actual voltage, current and temperature sensors.
- Adding CAN communication for exchanging battery information.
- Implementing more advanced SoC estimation techniques.
- Adding graphical plots to visualise battery parameters over time.
- Extending the diagnostic system with additional fault detection methods.

## 11. What I Learned

Working on this project gave me an opportunity to apply C programming to a problem related to electric vehicles and battery management.

Apart from implementing the diagnostic functions, I also got practical experience with dividing a program into multiple source and header files, handling CSV input, managing errors, using pointers and structures, and maintaining a consistent interface between modules.

I also got to work with Makefiles for building the project and Google Test for checking individual functions. This helped me understand that developing a working application is not just about writing code, but also about organising, building and testing it properly.

The project is a small step towards understanding how software is used in automotive embedded systems.

---

**Project:** BMS Diagnostic Supervisor  
**Course:** Embedded Automotive Systems Minor  
**Branch:** Electrical Engineering  
**Repository:** https://github.com/atharvahanuman-max/bms-diagnostic-supervisor