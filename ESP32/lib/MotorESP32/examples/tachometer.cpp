#include <Arduino.h>
#include "MotorESP32.h"

Motor drive_R = Motor(0, 23, 18, 19, 35, 11, 34.02);       // Right drive channel
Motor drive_L = Motor(1, 5, 16, 17, 34, 11, 34.02);        // Left drive channel

String dir = "fwd", rec;
int pwm = 0;
long timestamp;

void drive(int pwm_val, String direction) {
    if (direction == "fwd") {
        // Drive motor forwards at given pwm if direction is set to forwards
        drive_R.forward(pwm_val);
        drive_L.forward(pwm_val);
    }
    else {
        // Drive motor in reverse at given pwm
        drive_R.reverse(pwm_val);
        drive_L.reverse(pwm_val);
    }
}

void setup() {
    Serial.begin(115200);
    timestamp = millis();
}

void loop() {
    if (Serial.available()) {
        rec = Serial.readString();              // Read command string
        // If command is direction of motor, set dir varaible accordingly
        if (rec == "fwd" or rec == "rev") {
            // If direction has to be changed, brake motors and allow them to come to a halt first
            if (dir != rec) {
                drive_R.brake();
                drive_L.brake();
                delay(200);
            }
            dir = rec;
            drive(pwm, dir);
        }
        // Command is a % speed
        else if (rec.endsWith("%")) {
            Serial.println((rec.substring(0, rec.length()-1)));
            // parse command for % value and calculate pwm value
            int percentage = (rec.substring(0, rec.length()-1)).toInt();
            pwm = int(255 * (constrain(percentage, 0, 100) / 100.0));
            drive(pwm, dir);
        }
        else {
            pwm = rec.toInt();
            pwm = constrain(pwm, 0, 255);
            drive(pwm, dir);
        }

    }

    if (millis() - timestamp >= 50) {       // Only print every 50ms (i.e. at 20 Hz)
        Serial.println((String)"PWM: " + pwm + "\tLeft RPM: " + drive_L.getRPM() + "\tRight RPM: " + drive_R.getRPM());
        timestamp = millis();
    }
}