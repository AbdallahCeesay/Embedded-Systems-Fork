#include "mbed.h"

// Pin definitions
AnalogIn tempSensor(A0);
PwmOut led(PC_6);             

// Constants
const float V_REF = 3.3;              // Reference voltage for the analog input
const float TEMP_MIN = 20.0;          // Minimum temperature for LED brightness adjustment
const float TEMP_MAX = 40.0;          // Maximum temperature for LED brightness adjustment
const float TEMP_SENSOR_SCALE = 100.0 / V_REF; // Temperature range scale (e.g., 100°C at 3.3V)
const int SAMPLE_INTERVAL_MS = 100;   // Polling interval in milliseconds

int main() {
    
    led.period(0.001f); // Set PWM frequency to 1 kHz

    while (true) {
        // Read the analog input from the temperature sensor
        float sensorVoltage = tempSensor.read() * V_REF;
        float temperature = sensorVoltage * TEMP_SENSOR_SCALE;

        // Map temperature to LED brightness
        if (temperature < TEMP_MIN) {
            led.write(0.0f); // LED off
        } else if (temperature >= TEMP_MIN && temperature <= TEMP_MAX) {
            // Scale temperature to PWM duty cycle between 0 and 1
            float brightness = (temperature - TEMP_MIN) / (TEMP_MAX - TEMP_MIN);
            led.write(brightness); // Set LED brightness
        } else {
            led.write(1.0f); // LED fully on
        }

        // Polling delay
        thread_sleep_for(SAMPLE_INTERVAL_MS); 
    }
}
