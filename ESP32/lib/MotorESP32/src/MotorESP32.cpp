#include "MotorESP32.h"

Motor::Motor(int pwm_channel, int pwm_pin, int IN1_pin, int IN2_pin, int enc_pin, int PPR, double ratio) {
    // Initialise private attribute values
    pwm_ch = pwm_channel;
    IN1 = IN1_pin;
    IN2 = IN2_pin;
    _PPR = PPR;
    reduction_ratio = ratio;
    enc_pulses = 0;

    ledcSetup(pwm_ch, 5000, 8);         // Setup PWM 5KHz 8-bit PWM channel
    ledcAttachPin(pwm_pin, pwm_ch);     // Attach motor driver PWM pin to PWM channel
    // Set IN1 and IN2 I/O pins to OUTPUT
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    coast();                            // Initial state of motors set to coast

    pinMode(enc_pin, INPUT);            // Set encoder pin to INPUT
    // Allocate hardware interrupt and attach ISR to encoder pin. Interrupt tiggers at falling edge of each pulse
    attachInterrupt(digitalPinToInterrupt(enc_pin), std::bind(&Motor::encoder_ISR, this), FALLING);
    enc_time = micros();                // Set start time to current time (in microseconds)
}

void IRAM_ATTR Motor::encoder_ISR() {
    // If output shaft is yet to make another full revolution...
    if (enc_pulses < int(_PPR * reduction_ratio)) {
        enc_pulses++;               // ...Increment number of encoder pulses by 1
    }
    // Else use time period of revolution to calculate RPM
    else {
        RPM = 60000000.0 / (micros()-enc_time);
        enc_time = micros();        // Set encoder timer to current time (for use in next revolution)
        enc_pulses = 0;             // Reset encoder pulses to 0
    }
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

float Motor::getRPM() {
    return RPM;                     // Return RPM
}