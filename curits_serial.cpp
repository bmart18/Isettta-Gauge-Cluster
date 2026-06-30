// curtis_serial.cpp
#include "curtis_serial.h"

// Define global telemetry variables
volatile float live_mph = 0.0;
volatile int battery_soc = 100;
volatile int motor_temp = 25;

// Local tracking variables
static bool _simulationMode = false;
static unsigned long _lastUpdate = 0;

// Vehicle Configuration Math Constants
const float TIRE_DIAMETER_INCHES = 17.2; // Derived from budget 3.50-10 scooter tires
const float TIRE_CIRCUMFERENCE = TIRE_DIAMETER_INCHES * 3.14159;
const float GEAR_RATIO = 10.3;           // Standard differential configuration for golf cart transaxles

void initCurtisSerial(int rxPin, int txPin, bool useSimulation) {
    _simulationMode = useSimulation;
    
    if (!_simulationMode) {
        // Curtis Enhanced Serial Protocol runs strictly at 9600 Baud, 8-N-1
        Serial2.begin(9600, SERIAL_8N1, rxPin, txPin);
    } else {
        Serial.println("[SYSTEM] Curtis Serial initialized in BENCH SIMULATION Mode.");
    }
}

void updateCurtisData() {
    // ---- MODE A: BENCH SIMULATION (No vehicle attached) ----
    if (_simulationMode) {
        unsigned long currentMillis = millis();
        if (currentMillis - _lastUpdate >= 100) { // Refresh internal metrics at 10Hz
            _lastUpdate = currentMillis;

            // Generate a sweeping simulation pattern for testing needle transitions
            static float angle = 0;
            angle += 0.02;
            
            // Map simulated RPM up to a typical 15mph limit
            int simulatedRPM = (sin(angle) * 1100) + 1100; 
            
            // Apply physics calculations: RPM to MPH
            live_mph = ((float)simulatedRPM * TIRE_CIRCUMFERENCE) / (GEAR_RATIO * 1056.0);
            if (live_mph < 0.0) live_mph = 0.0;

            // Simulate minor continuous battery drain
            static long loopCounter = 0;
            if (loopCounter++ % 100 == 0) {
                if (battery_soc > 15) battery_soc--;
                else battery_soc = 100; // Reset loop
            }

            // Slowly scale operational heat
            motor_temp = 32 + (int)(sin(angle) * 5); 
        }
        return;
    }

    // ---- MODE B: PHYSICAL LIVE LINK (Curtis 1268 Hardware Interface) ----
    if (Serial2.available()) {
        // Simple ring-buffer framework to lock valid data packets
        static uint8_t buffer[16];
        static uint8_t index = 0;

        uint8_t incomingByte = Serial2.read();
        
        // SOFTWARE INVERSION FALLBACK: 
        // If an external hardware inverter gate (like a 74HC14) isn't utilized,
        // bitwise invert the raw incoming byte stream directly in software.
        // incomingByte = ~incomingByte; 

        buffer[index++] = incomingByte;

        // Process data frames when standard packet boundaries are populated
        if (index >= 6) { 
            // Placeholder validation for typical Curtis master-slave response strings
            // Format structure aligns with standard parameter blocks (e.g., [Header][ID][DataHigh][DataLow][Checksum])
            if (buffer[0] == 0x3A) { // Hex identifier byte ':'
                int rawRPM = (buffer[2] << 8) | buffer[3];
                int rawSoC = buffer[4];

                // Global conversion scaling execution
                live_mph = ((float)rawRPM * TIRE_CIRCUMFERENCE) / (GEAR_RATIO * 1056.0);
                battery_soc = constrain(rawSoC, 0, 100);
            }
            index = 0; // Clear index window
        }
    }
}
