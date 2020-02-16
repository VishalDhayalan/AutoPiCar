#include "MotorESP32.h"

Motor::Motor(int pwm_channel, int pwm_pin, int IN1_pin, int IN2_pin, int enc_pin, int PPR, double ratio) {
    // Initialise private attribute values
    pwm_ch = pwm_channel;
    IN1 = IN1_pin;
    IN2 = IN2_pin;
    _PPR = PPR;
    reduction_ratio = ratio;

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

// Returns and replaces oldest value in encoder buffer with new measurement
uint16_t IRAM_ATTR Motor::updateBuffer(uint16_t value) {
    uint16_t item = enc_timeBuffer[bufferIndex];            // Store oldest value in temp variable 'item'
    enc_timeBuffer[bufferIndex] = value;                    // Replace with new value
    bufferIndex = (bufferIndex + 1) % encoderBufferSize;    // Increment index (within bounds of the buffer's size)
    return item;                                            // Return oldest value
}

// ISR calculates RPM using average time period of encoder pulses
void IRAM_ATTR Motor::encoder_ISR() {
    volatile unsigned long now = micros();                      // Current timestamp (number of microsecs since start of execution)
    volatile int new_period = now - enc_time;                   // Calculate time period of pulse

    if (new_period > 10000) {
        bufferInitialised = false;
        bufferIndex = 0;
        enc_time = now;
        return;
    }

    volatile int old_period = updateBuffer(now - enc_time);     // Replace oldest period with new period

    if (bufferInitialised) {
        //If buffer already initialised, calculate average pulse period iteratively and use it to calculate RPM
        enc_averagePeriod = enc_averagePeriod - ((old_period - new_period) / (double)encoderBufferSize);
        RPM = 60000000.0 / (enc_averagePeriod * _PPR * reduction_ratio);
    }
    // If index is back to 0, it indicates one full pass through the buffer, thus buffer filled with measured values
    else if (bufferIndex == 0) {
        // Calculate an initial average for pulse period by summing values and dividing by number of values
        volatile unsigned int sum = 0;
        for (int item = 0; item < encoderBufferSize; item++) {
            sum = sum + enc_timeBuffer[item];
        }
        enc_averagePeriod = sum / (double)encoderBufferSize;
        RPM = 60000000.0 / (enc_averagePeriod * _PPR * reduction_ratio);    // Calculate RPM using initial average
        bufferInitialised = true;                                           // Set 'buffer initialised' flag to true
    }
    enc_time = now;                 // Set pulse edge timestamp to time at start of ISR (when pulse edge was detected)
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
    ledcWrite(pwm_ch, 0);               // Set PWM duty cycle to 0
}

// Actively brake motors to a halt
void Motor::brake() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, HIGH);
    ledcWrite(pwm_ch, 0);               // Set PWM duty cycle to 0
}

double Motor::getRPM() {
    if (micros() - enc_time > 15000) {
        RPM = 0.0;
    }
    return RPM;                         // Return RPM
}