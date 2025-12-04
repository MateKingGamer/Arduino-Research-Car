<h1 align="center">🤖 Explorer Bot</h1>
<p align="center">
  Autonomous modular rover powered by ESP32 & Arduino  
  <br>
  Designed for exploring environments unsafe for humans 🌍
</p>

<p align="center">
  <img src="https://img.shields.io/badge/Hardware-ESP32-green?style=flat-square">
  <img src="https://img.shields.io/badge/Wireless-HC--12-blue?style=flat-square">
  <img src="https://img.shields.io/badge/Camera-ESP32--CAM-red?style=flat-square">
  <img src="https://img.shields.io/badge/Status-In%20Active%20Development-yellow?style=flat-square">
</p>

---

## 📌 Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Hardware](#hardware)
- [How It Works](#how-it-works)
- [Development Progress](#development-progress)
- [Roadmap](#roadmap)
- [Media](#media)
- [License](#license)

---

## 🔍 Overview
Explorer Bot is an **autonomous environmental rover**.  
It navigates independently or via a custom controller, while streaming **live video** and sending **environmental data** in real time.

The platform is **magnetic and modular**, making sensors and tools easy to swap depending on the mission.

---

## ✨ Features
✔ Autonomous obstacle avoidance (ultrasonic + IR)  
✔ Manual control with joysticks **or** wrist-based motion  
✔ Long-range HF communication (**HC-12**, up to kilometers)  
✔ Live video streaming with camera pan/tilt  
✔ Gas, temperature, humidity, soil moisture and rain sensing  
✔ Automatic light activation in dark conditions  
✔ Custom 3D-printed body with magnetic attachments  

---

## 🧠 Hardware

### 🤖 Rover
| Component | Function |
|---|---|
| ESP32 Wroom | Main controller + sensor interface |
| 4× TT Motors + L298N | Driving and direction |
| 3× Ultrasonic + 1× IR | Obstacle detection |
| MQ-135 | Gas quality monitoring |
| DHT22 | Temperature & humidity |
| Light Sensor | Auto-light control |
| Soil & Rain Sensors | Terrain and weather detection |
| 2× Servos | Camera and sensor movement |
| ESP32-CAM | Video streaming |
| LED Headlight | Dark area visibility |

### 🎮 Controllers
| Controller | Components | Purpose |
|---|---|---|
| Joystick Controller | Arduino Nano, HC-12, LCD, 2× Joysticks, Buttons | Driving + camera + data display |
| Wrist Controller | ESP32-C3, MPU6050, OLED, Buttons | Motion-based driving |

---

## ⚙️ How It Works
- **Manual Mode** → Joystick or wrist rotation controls the rover  
- **Auto Mode** → Sensors scan the environment and the rover avoids obstacles  
- Video from **ESP32-CAM** streams to the TFT display  
- Controller LCD shows live environmental readings  

The system communicates using **HC-12 RF**, giving far greater range than Bluetooth.

---

## 📈 Development Progress

| Area | Was | Now | Upgrade Result |
|---|---|---|---|
| Body | Wood plates | Custom 3D-print (JLCPCB sponsored) | Strong, modular, clean |
| Drive | Continuous servos | TT motors + L298N | Stable, faster, more torque |
| Comms | Bluetooth | HC-12 | Long-range, responsive |
| Brain | Arduino Nano | ESP32 Wroom | More sensors, faster processing |

Project is constantly evolving — new upgrades are being tested.

---

## 🧭 Roadmap
- Mobile app via Wi-Fi
- GPS navigation + data logging
- Advanced terrain stabilization
- More interchangeable sensor/camera modules
- Cloud monitoring

> Contributions & ideas are welcome!

