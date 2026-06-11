# ESP32 Drone Flight Controller

A custom drone flight controller built using an ESP32, MPU6050, and BMP280. The project supports autonomous stabilization, altitude estimation, WiFi-based manual control, and real-time 3D orientation visualization.

## Features

- Roll and Pitch stabilization using MPU6050
- Altitude estimation using BMP280
- WiFi-based control using ESP32 Access Point mode
- Auto Mode (self-stabilizing)
- Manual Mode (phone-controlled)
- Real-time web dashboard
- 3D orientation visualization in browser
- Expandable to ESC and brushless motor control

---

## Hardware Used

### Core Components

- ESP32 Development Board
- MPU6050 IMU
- BMP280 Barometric Pressure Sensor

### Future Flight Hardware

- 4 × Brushless Motors
- 4 × ESCs
- 3S LiPo Battery
- 5V BEC
- Drone Frame

---

## Wiring

### MPU6050

| MPU6050 | ESP32 |
|----------|----------|
| VCC | 3.3V |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

### BMP280

| BMP280 | ESP32 |
|----------|----------|
| VCC | 3.3V |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

Both sensors communicate using the I2C bus.

---

## Software Requirements

Arduino IDE

Libraries:

- Wire
- WiFi
- WebServer
- MPU6050 (Jeff Rowberg)
- Adafruit BMP280

---

## System Architecture

```
Phone/Web UI
      │
      ▼
   ESP32
      │
 ┌────┴────┐
 ▼         ▼
MPU6050   BMP280
 │         │
 └────┬────┘
      ▼
  Control Logic
      ▼
 Motor Outputs
```

---

## Operating Modes

### Auto Mode

The controller automatically:

- Maintains level orientation
- Corrects roll and pitch disturbances
- Holds relative altitude

### Manual Mode

Control is provided through a web interface.

User can adjust:

- Throttle
- Roll
- Pitch

---

## Accessing the Controller

### Connect to WiFi

SSID:

```
DroneFC
```

Password:

```
12345678
```

### Open Browser

```
http://192.168.4.1
```

---

## Flight Control Logic

1. Read MPU6050 sensor data
2. Calculate Roll and Pitch
3. Read BMP280 altitude
4. Determine operating mode
5. Compute error values
6. Apply proportional control
7. Generate motor outputs
8. Repeat continuously

---

## Control Equations

### Roll

```
Roll = atan2(ay, az) × 180 / π
```

### Pitch

```
Pitch = atan2(-ax, √(ay² + az²)) × 180 / π
```

### Relative Altitude

```
Relative Altitude =
Current Altitude − Initial Altitude
```

### Motor Mixing

```
M1 = Throttle + Pitch + Roll
M2 = Throttle + Pitch - Roll
M3 = Throttle - Pitch + Roll
M4 = Throttle - Pitch - Roll
```

---

## Motor Layout

```
        FRONT

     M1       M2

     M3       M4

         BACK
```

Rotation directions:

```
M1 → CCW
M2 → CW
M3 → CW
M4 → CCW
```

---

## Testing

Current testing includes:

- Sensor validation
- Roll/Pitch verification
- Altitude measurement
- Auto mode stabilization
- Manual mode switching
- WiFi communication
- 3D visualization

No motors or propellers are required for software testing.

---

## Future Improvements

- Full PID controller
- Yaw control
- ESC integration
- GPS navigation
- Telemetry dashboard
- Autonomous waypoint flight
- Battery monitoring

---


## Author

Developed as an educational drone flight controller project using ESP32 and MEMS sensors.