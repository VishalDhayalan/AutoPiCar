#include "MotorESP32.h"

Motor::Motor(int pwm_channel, int pwm_pin, int IN1_pin, int IN2_pin) {
    // Initialise private attribute values
    pwm_ch = pwm_channel;
    IN1 = IN1_pin;
    IN2 = IN2_pin;

    ledcSetup(pwm_ch, 5000, 8);         // Setup PWM 5KHz 8-bit PWM channel
    ledcAttachPin(pwm_pin, pwm_ch);      // Attach motor driver PWM pin to PWM channel
    // Set IN1 and IN2 I/O pins to OUTPUT
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    coast();                            // Initial state of motors set to coast
}

// Drive motor forwards at set PWM duty cycle
void Motor::forward(int pwm_speed) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    ledcWrite(pwm_ch, pwm_speed);       // Set PWM duty cycle
}

// Drive motor in reverse at set PWM duty cycle
void Motor::reverse(int pwm_speed) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    ledcWrite(pwm_ch, pwm_speed);       // Set PWM duty cycle
}

// Coast motors: allow them to naturally come to a halt (no active braking)
void Motor::coast() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    ledcWrite(pwm_ch, 0);           // Set PWM duty cycle to 0
}

// Actively brake motors to a halt
void Motor::brake() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, HIGH);
    ledcWrite(pwm_ch, 0);           // Set PWM duty cycle to 0
}