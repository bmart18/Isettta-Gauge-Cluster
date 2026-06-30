// curtis_serial.h
#ifndef CURTIS_SERIAL_H
#define CURTIS_SERIAL_H

#include <Arduino.h>

// Global telemetry variables safely accessible across both ESP32 cores
extern volatile float live_mph;
extern volatile int battery_soc;
extern volatile int motor_temp;

/**
 * Initializes Hardware Serial 2 on the ESP32.
 * @param rxPin ESP32 Pin wired to the TX output of the 74HC14 Inverter/Curtis Port.
 * @param txPin ESP32 Pin wired to the RX input of the 74HC14 Inverter/Curtis Port.
 * @param useSimulation Set to true to inject bench-test data without a physical controller.
 */
void initCurtisSerial(int rxPin, int txPin, bool useSimulation = false);

/**
 * Executes on Core 0. Processes raw serial frames, flips inverted bits,
 * parses hex data streams, and updates global parameters.
 */
void updateCurtisData();

#endif // CURTIS_SERIAL_H
