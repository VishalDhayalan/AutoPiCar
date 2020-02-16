#ifndef MotorESP32_h
#define MotorESP32_h

#include <Arduino.h>
#include <FunctionalInterrupt.h>

#define encoderBufferSize 64            // Size (# items) of encoder buffer

class Motor {
    private:
        int pwm_ch, IN1, IN2;
        volatile int _PPR, bufferIndex = 0;
        volatile double reduction_ratio, RPM, enc_averagePeriod = 0.0;
        volatile unsigned long enc_time;                                // Timestamp (in microsecs) of previous encoder pulse
        volatile uint16_t enc_timeBuffer[encoderBufferSize] = {};       // Circular buffer used to store period of encoder pulses (initialised with zeros)
        bool bufferInitialised = false;                                 // Flag indicating buffer filled with measured values (i.e all zeros replaced)

        void IRAM_ATTR encoder_ISR();                                   // Interrupt Service Routine for calculating RPM using motor encoder
        uint16_t IRAM_ATTR updateBuffer(uint16_t value);                // Returns and replaces oldest value in encoder buffer with new measurement

    public:
        Motor(int pwm_channel, int pwm_pin, int IN1_pin, int IN2_pin, int enc_pin, int PPR, double ratio);
        void forward(int pwm_speed);
        void reverse(int pwm_speed);
        void coast();
        void brake();
        double getRPM();
};
#endif