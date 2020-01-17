#include <Arduino.h>
#include "MotorESP32.h"

Motor drive_R = Motor(0, 23, 18, 19);       // Right drive channel
Motor drive_L = Motor(1, 5, 16, 17);        // Left drive channel

void setup() {
    
}

void loop() {
    // Forwards at full speed
    drive_R.forward(255);
    drive_L.forward(255);
    delay(2000);                // Maintain at full speed for 2s
    
    // Forwards at half speed
    drive_R.forward(128);
    drive_L.forward(128);
    delay(2000);                // Maintain at half speed for 2s

    // Coast Motors
    drive_R.coast();
    drive_L.coast();
    delay(2000);                // Wait for motors to come to a halt

    // Reverse at full speed
    drive_R.reverse(255);
    drive_L.reverse(255);
    delay(2000);                // Maintain at full speed for 2s

    // Reverse at half speed
    drive_R.reverse(128);
    drive_L.reverse(128);
    delay(2000);                // Maintain at half speed for 2s

    // Brake Motors
    drive_R.brake();
    drive_L.brake();
    delay(5000);                // Wait 5s before repeating
}