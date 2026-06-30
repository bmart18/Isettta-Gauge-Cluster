#include "curtis_serial.h"

void setup() {
    Serial.begin(115200);
    // Initialize in simulation mode to test calculation ranges
    initCurtisSerial(16, 17, true); 
}

void loop() {
    updateCurtisData(); // Simulates inputs for now to test
    
    Serial.print("Speed: "); Serial.print(live_mph);
    Serial.print(" MPH | Battery: "); Serial.print(battery_soc);
    Serial.print("% | Temp: "); Serial.print(motor_temp);
    Serial.println(" °C");
    
    delay(100);
}
