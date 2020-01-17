#ifndef MotorESP32_h
#define MotorESP32_h

#include <Arduino.h>

class Motor {
    private:
        int pwm_ch, IN1, IN2;

    public:
        Motor(int pwm_channel, int pwm_pin, int IN1_pin, int IN2_pin);
        void forward(int pwm_speed);
        void reverse(int pwm_speed);
        void coast();
        void brake();
};
#endif