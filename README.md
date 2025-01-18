# FreeRTOS FPGA Application: Multi-Process Communication and Display Control

[**Demonstration Video on YouTube**](https://www.youtube.com/watch?v=LCpn7amrAR4)

This project demonstrates a FreeRTOS-based application for the Xilinx XC7S50-CSG324A Spartan-7 FPGA. The application implements:

1. **Switch State Monitoring**: Reads and processes the states of 5 switches.
2. **LED State Control**: Updates the states of 3 LEDs based on switch inputs.
3. **7-Segment Display Control**: Displays two digits based on switch states.
4. **Serial Communication**: Combines data from multiple processes and outputs it to the serial console.

---

## Features

### Multi-Process Design
The application consists of four FreeRTOS tasks:

- **Process 1**: Reads switch states and sends them to a queue.
- **Process 2**: Reads LED states and sends them to another queue.
- **Process 3**: Updates 7-segment display values based on the received data.
- **Process 4**: Outputs the combined data to the UART console.

### Hardware Interaction
- **GPIO Interfaces**:
  - Reads inputs from switches.
  - Controls LEDs and 7-segment displays.
- **Peripheral Initialization**:
  - Configures GPIO peripherals for switches, LEDs, and 7-segment displays.

---

## Code Structure

### Global Variables
- **`SwitchStates`**: Structure to hold switch states.
- **`LDStates`**: Structure to hold LED states.
- **`segmentEncoding`**: Array to encode digits for 7-segment display (common cathode).

### Queues
- **`xQueue1`**: Queue for switch states.
- **`xQueue2`**: Queue for LED states.
- **`xQueue3`**: Queue for 7-segment display values.

### Key Functions
1. **`init_perif()`**: Initializes GPIO peripherals for switches, LEDs, and 7-segment displays.
2. **`Proces1`**: Reads switch states and updates the LED states.
3. **`Proces2`**: Reads LED states and forwards them.
4. **`Proces3`**: Updates 7-segment display values based on queue data.
5. **`Proces4`**: Outputs all data to the UART console.

---

## Requirements

### Hardware
1. **Xilinx Spartan-7 FPGA (XC7S50-CSG324A)**
2. **Switch Inputs** (5 switches connected to GPIO)
3. **LED Outputs** (3 LEDs connected to GPIO)
4. **7-Segment Displays** (2 displays controlled via GPIO)

### Software
1. **FreeRTOS**: Real-Time Operating System for task scheduling.
2. **Xilinx SDK**: For application development and peripheral configuration.

---

## Implementation Details

### Task Functionality

#### **Process 1: Switch State Monitoring**
- Reads the state of 5 switches from GPIO.
- Updates the states of LEDs (LEDs 2–4 mirror switches 2–4).
- Sends switch states to `xQueue1`.

#### **Process 2: LED State Monitoring**
- Reads the states of 3 LEDs from GPIO.
- Sends LED states to `xQueue2`.

#### **Process 3: 7-Segment Display Update**
- Reads switch states from `xQueue1`.
- Updates two 7-segment displays based on the first two switch states.

#### **Process 4: Serial Communication**
- Reads data from `xQueue1` and `xQueue2`.
- Combines switch, LED, and 7-segment display states.
- Outputs formatted data to the UART console.

---

## Example Serial Output

When running the application, the following information is displayed on the UART console:

```
--- Citesc valorile switch-urilor ---
Starile intrerupatoarelor sunt:
SW0 = 1
SW1 = 0
SW2 = 1
SW3 = 0
SW4 = 1

Starile LED-urilor 2,3,4 sunt:
LD2 = 1
LD3 = 0
LD4 = 1

Cifra de pe Display dreapta = 1
Cifra de pe Display stanga = 0
```

---

## File Structure
- **`main.c`**: Contains the main application code.
- **`README.md`**: Documentation for the application.

---
