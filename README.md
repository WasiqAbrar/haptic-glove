# haptic-glove

A low-cost haptic feedback glove prototype that tracks hand orientation using a 10 DOF IMU sensor and provides vibration feedback through motors when interacting with virtual 3D objects in Unity.

---

## Table of Contents

- [Overview](#overview)
- [Demo](#demo)
- [Hardware Components](#hardware-components)
- [Circuit Wiring](#circuit-wiring)
- [Software](#software)
- [Installation](#installation)
- [How It Works](#how-it-works)
- [Project Structure](#project-structure)
- [Known Issues](#known-issues)
- [License](#license)

---

## Overview

The Haptic Glove connects physical hand movement to a virtual 3D environment. An IMU sensor mounted on the hand reads orientation data, which is sent to Unity over Serial communication. A 3D hand model in Unity mirrors the physical movement in real time. When the virtual hand interacts with a 3D object, Unity sends a command back to the Arduino, which triggers vibration motors on the glove, giving physical tactile feedback.

**Data flow:**
```
IMU Sensor → Arduino → Unity (3D hand rotates) → Arduino → Vibration Motors
```

---

## Demo

- 3D hand model rotates in sync with physical IMU movement
- Hovering mouse over virtual sphere → all 5 motors vibrate (ON)
- Moving mouse away → motors stop (OFF)
- Clicking on the virtual sphere → short vibration pulse (PULSE)

---

## Hardware Components

| Component | Quantity | Purpose |
|---|---|---|
| Arduino Uno | 1 | Main microcontroller |
| 10 DOF IMU Module (LSM303 + ITG3200 + HMC5883L) | 1 | Reads orientation data |
| HC-SR04 Ultrasonic Sensor | 1 | Originally used for distance detection (now unused) |
| N-MOSFET Transistors | 5 | Switch motors on/off |
| Vibration Motors (coin type) | 5 | Haptic feedback on fingers |
| Breadboard | 1 | Prototyping |
| Jumper Wires | 1 set | Connections |
| USB Cable | 1 | Power and Serial communication |

---

## Circuit Wiring

### IMU (10 DOF) → Arduino

| IMU Pin | Arduino Pin |
|---|---|
| VIN | 5V |
| GND | GND |
| SDA | A4 |
| SCL | A5 |

### Motors → Arduino (via MOSFET)

| MOSFET Pin | Connection |
|---|---|
| Gate | Arduino pins 3, 5, 6, 9, 10 |
| Drain | Motor negative terminal |
| Source | GND |

Motor positive terminals connect to 5V rail.

### I2C Addresses (confirmed via scanner)

| Address | Chip | Function |
|---|---|---|
| 0x19 | LSM303 | Accelerometer |
| 0x1E | HMC5883L | Magnetometer |
| 0x69 | ITG3200 | Gyroscope |

---

## Software

| Software | Version | Purpose |
|---|---|---|
| Arduino IDE | 1.8.13+ | Upload code to Arduino |
| Unity | 6 (Built-In Render Pipeline) | 3D visualization and haptic control |
| Wire.h | Built-in | I2C communication (no external libraries needed) |

**No external Arduino libraries required** — only the built-in `Wire.h` is used.

**Unity settings required:**
- Api Compatibility Level → `.NET Framework`
- Physics Raycaster component on Main Camera

---

## Installation

### Arduino Setup

1. Open `arduino/haptic_glove.ino` in Arduino IDE
2. Connect Arduino Uno via USB
3. Go to `Tools` → `Board` → select `Arduino Uno`
4. Go to `Tools` → `Port` → select your COM port
5. Click Upload
6. Keep IMU still during the startup calibration (1 second)

### Unity Setup

1. Open the `unity/` folder as a Unity project in Unity Hub
2. Select `3D (Built-In Render Pipeline)` template
3. Go to `Edit` → `Project Settings` → `Player` → `Other Settings.`
4. Set `Api Compatibility Level` to `.NET Framework`
5. Open `IMUReader.cs` and confirm the port name matches your Arduino COM port:
   ```csharp
   string portName = "COM5"; // change this if needed
   ```
6. Close the Arduino IDE completely before pressing Play in Unity
7. Press Play — the hand model will respond to IMU movement

---

## How It Works

**Arduino side:**
- Reads accelerometer data from LSM303 over I2C
- Calculates roll and pitch angles using atan2 formulas
- Sends angles to Unity over Serial every 10ms
- Listens for motor commands (ON / OFF / PULSE) from Unity
- Uses Wire.setWireTimeout() to prevent I2C freezing during movement

**Unity side:**
- IMUReader.cs reads Serial data every frame
- Maps roll to Z rotation and pitch to X rotation of the HandController object
- Uses Quaternion.Lerp for smooth interpolation between angle updates
- HapticObject.cs detects mouse hover and click on 3D objects
- Sends ON, OFF, or PULSE commands back to Arduino via Serial

**Motor commands:**

| Command | Result |
|---|---|
| ON | All 5 motors vibrate continuously |
| OFF | All motors stop |
| PULSE | Motors vibrate for 150ms then stop — triggered by clicking the sphere |

---

## Project Structure

```
haptic-glove/
│
├── arduino/
│   └── haptic_glove.ino        # Main Arduino sketch
│
├── unity/
│   └── Assets/
│       └── Scripts/
│           ├── IMUReader.cs    # Reads IMU data, rotates hand, sends motor commands
│           └── HapticObject.cs # Triggers motor feedback on mouse interaction
│
└── README.md
```

---

## Known Issues

| Issue | Cause | Status |
|---|---|---|
| Hand movement freezes during IMU motion | I2C connection drops on breadboard during physical movement | Partially fixed using Wire.setWireTimeout() |
| Circular hand movement | IMU mounted upside down causing axis confusion in complementary filter | Partially fixed using pure accelerometer angles |
| Movement lag | Serial baud rate at 9600 limits updates to 20 per second | Can be fixed by changing baud rate to 115200 |
| Wave gesture not demonstrated | Hand model geometry misaligned with Unity coordinate system | Needs correct hand model orientation |

---

## License

This project is open source and free to use for educational and non-commercial purposes.
