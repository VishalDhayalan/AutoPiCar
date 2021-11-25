# AutoPiCar
A Computer Vision based Raspberry Pi self-driving car project capable of navigating straight lanes, hairpin bends and 45 degree turns while detecting and responding to road signs, pedestrian crossings and obstacles in real-time.

## Main Components
* Raspberry Pi 3B+
* Pi Camera V2
* ESP32 (used as a secondary processor especially for interrupt handling)
* VL53L1X Time-of-Flight sensor
* 2300mAh 3S LiPo battery
* 4x 350 RPM geared DC motors w/ encoders
* 2x SG90 servos (used in Pi Camera gimbal)
* MG995 (steering) servo
* XY-160D 7A Dual DC motor driver
