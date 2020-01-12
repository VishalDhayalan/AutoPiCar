#include <Arduino.h>

String uart_received;                 // String to store response

bool initComms(int timeout) {
  // Wait for RPi to boot and send "Pi Ready"
  while (uart_received != "Pi Ready.") {
    if (Serial.available()) {
      uart_received = Serial.readString();
    }
    delay(5);
  }

  Serial.print("OK");           // Reply to "Pi Ready" with "OK"
  Serial.print("ESP Ready.");   // Send "ESP Ready"

  long start_time = millis();
  // Wait for "OK" reply from RPi
  while (uart_received != "OK") {
    if (millis() - start_time >= timeout) {
      return false;
    }
    else if (Serial.available()) {
      uart_received = Serial.readString();
    }
    delay(5);
  }
  // ---------- UART Handshake with RPi Complete! ---------- //
  uart_received = "";
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
}

void loop() {
  
}