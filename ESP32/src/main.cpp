#include <Arduino.h>
#include <PID_v1.h>
#include <ArduinoJson.h>
#include <Servo.h>
#include "MotorESP32.h"

String uart_received;                                                   // String to store response
String motorDirection = "fwd";
double targetRPM = 0, leftRPM, leftPWM, rightRPM, rightPWM;
float kp=1.8, ki=2.25, kd=0.1;                                          // PID Parameters

Motor motor_R = Motor(0, 23, 18, 19, 35, 11, 34.02);                    // Right drive channel
Motor motor_L = Motor(1, 5, 16, 17, 34, 11, 34.02);                     // Left drive channel

PID motorPID_L(&leftRPM, &leftPWM, &targetRPM, kp, ki, kd, DIRECT);
PID motorPID_R(&rightRPM, &rightPWM, &targetRPM, kp, ki, kd, DIRECT);

Servo steering;                                                         // Steering servo
Servo gimbalTilt;                                                       // Camera gimbal tilt servo
Servo gimbalPan;                                                        // Camera gimbal pan servo

bool initComms(int timeout) {
  String received;
  // Wait for RPi to boot and send "Pi Ready"
  while (received != "Pi Ready.") {
    if (Serial.available()) {
      received = Serial.readString();
    }
    delay(5);
  }

  Serial.print("OK");           // Reply to "Pi Ready" with "OK"
  Serial.print("ESP Ready.");   // Send "ESP Ready"

  long start_time = millis();
  // Wait for "OK" reply from RPi
  while (received != "OK") {
    if (millis() - start_time >= timeout) {
      return false;
    }
    else if (Serial.available()) {
      received = Serial.readString();
    }
    delay(5);
  }
  // ---------- UART Handshake with RPi Complete! ---------- //
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(20);
  if (!initComms(3000)) {
    Serial.println("\nUART Handshake with Pi failed... Restarting in 5 seconds");
    delay(5000);
    ESP.restart();
  }

  steering.attach(4, 2, 0, 180, 680, 2240);       // Attach steering servo to GPIO 4
  gimbalTilt.attach(14, 3, 0, 180);               // Attach gimbal titlt servo to GPIO 14
  gimbalPan.attach(13, 4, 0, 180);                // Attach gimbal pan servo to GPIO 13
  steering.write(90);                             // Set steering servo initial position to 90 degrees (wheels straight)
  gimbalTilt.write(70);                           // Set tilt servo initial positon to 90 degrees
  gimbalPan.write(90);                            // Set pan servo initial position to 90 degrees

  // Setup PID
  motorPID_R.SetSampleTime(40);
  motorPID_L.SetSampleTime(40);
  motorPID_R.SetOutputLimits(0, 255);
  motorPID_L.SetOutputLimits(0, 255);
  motorPID_R.SetMode(AUTOMATIC);
  motorPID_L.SetMode(AUTOMATIC);
}

void loop() {
  StaticJsonDocument<90> controlJson;

  if (Serial.available()) {
    uart_received = Serial.readString();              // Read command string
    // Check to see if string could be JSON
    if (uart_received.indexOf('{') != -1 || uart_received.indexOf('[') != -1) {
     deserializeJson(controlJson, uart_received);     // deserialise JSON string
     targetRPM = controlJson["target RPM"];           // Update target RPM
    }
  }

  leftRPM = motor_L.getRPM();
  rightRPM = motor_R.getRPM();
  motorPID_R.Compute();
  motorPID_L.Compute();
  if (targetRPM == 0) {
    motor_R.brake();
    motor_L.brake();
  }
  else {
    motor_R.forward(rightPWM);
    motor_L.forward(leftPWM);
  }
}