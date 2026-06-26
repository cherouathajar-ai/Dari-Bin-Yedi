🏠 Dari Bin Yedi - Smart Home Hub

📖 Project Overview

"Dari Bin Yedi" is a comprehensive, real-time Smart Home and Security system built using an ESP32 microcontroller and Python. It utilizes MQTT protocol for instant, bidirectional communication between a physical hardware hub and a desktop graphical user interface (GUI).

The system features real-time environmental monitoring, an intruder alarm system, remote lighting control, and instant Telegram notifications.

✨ Key Features

Live Environmental Telemetry: Monitors temperature, humidity, and gas/smoke levels in real-time.

Smart Security System: Features "HOME" and "AWAY" modes. When armed (AWAY), PIR motion sensors and magnetic reed switches protect doors and windows, triggering local alarms and remote alerts.

Remote Control Dashboard: A custom Python Tkinter GUI that displays live sensor data and allows for remote control of room lighting and system arming/disarming.

Instant Notifications: Integrates with the Telegram API to send push notifications directly to a smartphone during security breaches or weather events (e.g., sudden rain).

Hardware Display: An onboard OLED screen provides instant, at-a-glance system status, temperature, humidity, and door/window states without needing a computer.

FreeRTOS Architecture: The ESP32 utilizes a Real-Time Operating System (FreeRTOS) to multitask efficiently, separating safety, security, telemetry, and display tasks into concurrent loops.

🛠️ Hardware Components

Microcontroller: ESP32 Development Board

Display: 0.96" I2C OLED Display (SSD1306)

Sensors:

DHT11 (Temperature & Humidity)

MQ-2 (Gas & Smoke Detector)

Analog Rain Drop Sensor

PIR Motion Sensor

Magnetic Reed Switches (Door/Window)

Actuators & Indicators:

Active Buzzer (Alarm)

LEDs / Relays (Room Lighting Simulation)

Access Control: RC522 RFID Reader (Hardware interface built; simulation via dashboard for presentation)

💻 Software Stack

Microcontroller Code: C++ (Arduino IDE) utilizing FreeRTOS.

Desktop Dashboard: Python 3 (Libraries: tkinter, paho-mqtt, urllib).

Communication Protocol: MQTT (Hosted on HiveMQ Public Broker).

Alerting: Telegram Bot API.

🚀 How It Works

The Brain (ESP32): The ESP32 continuously reads analog and digital data from the connected sensors. It processes this logic locally (e.g., sounding the buzzer if gas is high) and simultaneously publishes the state to specific MQTT topics.

The Broker (HiveMQ): Acts as the middleman in the cloud, instantly routing messages between the ESP32 and the Python Dashboard.

The Interface (Python): The desktop app subscribes to the sensor topics to update its UI in real-time. It also publishes control commands (like turning on a light or changing security modes), which the ESP32 instantly receives and executes.

📸 Project Showcase
<div align="center">
  <img src="https://github.com/user-attachments/assets/7c590fd2-26a1-41bc-852c-ebf85e5640ce" width="400">
  <p><i>Figure 1: Smart Home Dashboard</i></p>
</div>

<div align="center">
  <img src="https://github.com/user-attachments/assets/18cfb8ac-806b-4ca2-b48e-d5c0659ebbd4" width="400">
  <p><i>Figure 2: Hardware Prototype</i></p>
</div>
<div align="center">
  <img src="https://github.com/user-attachments/assets/44b42f99-2aae-4d04-8a5f-4fab7086fe9d" width="400">
  <p><i>Figure 3: Oled display</i></p>
</div>
<div align="center">
  <img src="https://github.com/user-attachments/assets/6a2eb29b-4cde-496f-a5e4-45cf58a63015" width="400">
  <p><i>Figure 4: Telegram Bot</i></p>
</div>
developed by Hajar Cherouat
