Explorer Bot

Autonomous environmental rover powered by ESP32 and Arduino.
It can explore dangerous areas, collect real-time data, and stream live video while being controlled from a distance.

Features

Manual or autonomous driving

Long-range wireless communication (HC-12)

Live video streaming (ESP32-CAM + TFT display)

Obstacle detection with ultrasonic + IR sensors

Gas, temperature, humidity, soil moisture and rain sensors

Magnetic modular design for quick sensor/camera swap

Two control options: joystick controller or wrist motion controller

Hardware Overview

On the Rover

ESP32 Wroom (main controller)

L298N motor driver + 4 TT motors

Ultrasonic + IR obstacle sensors

MQ-135 gas, DHT22 environment, light, rain + soil moisture sensors

Servo-based modules for camera and ground sensors

LED headlight with automatic control

Controllers

Joystick controller: Arduino Nano, HC-12, LCD 16×2

Wrist controller: ESP32-C3, MPU6050 motion sensor

How It Works

In manual mode, movement is controlled via joysticks or hand rotation.
In auto mode, sensors detect obstacles and the rover navigates by itself.
Live video is streamed to the controller for full visibility.

Development Progress

The rover went through multiple upgrades:

Body: wood → custom 3D-printed modular chassis (sponsored by JLCPCB)

Movement: servos → TT motors for stability + power

Communication: Bluetooth → HC-12 for long-range control

Main MCU: Arduino Nano → ESP32 for more I/O and processing

Roadmap

App control through Wi-Fi

GPS and internet monitoring

More sensor modules

Terrain stabilization

For more info check out youtube channel Robo Crafts.
