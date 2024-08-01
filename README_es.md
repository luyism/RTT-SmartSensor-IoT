# RTT-SmartSensor-IoT

[中文](README.md)

The **RTT-SmartSensor-IoT** is a compact IoT project based on the StarFire One development board, aimed at achieving basic environmental monitoring and remote control functionalities. This project integrates multiple sensors, cloud connectivity, user interface, and remote control features to provide a practical platform for monitoring and managing environmental conditions.

**Features:**

1. **Environmental Data Collection:**
   - Utilizes the onboard AHT10 sensor for temperature and humidity measurement.
   - Employs the AP3216C sensor for proximity detection and ambient light intensity measurement.
2. **Cloud Connectivity:**
   - Implements MQTT protocol to connect to Alibaba Cloud for uploading sensor data and subscribing to control commands.
   - Provides real-time data transmission and command processing for remote control of onboard peripherals such as LED matrix, individual LEDs, and buzzer.
3. **Mobile App Remote Control:**
   - Develops a custom mobile application for real-time monitoring of sensor data and sending control commands.
   - Enables users to remotely change the color mode of the LED matrix, switch LED states, and toggle the buzzer from the mobile app.
4. **LVGL User Interface:**
   - Develops a graphical user interface using LVGL to display current sensor readings and control settings directly on the development board's display.

This project, though simple, demonstrates the fundamental concepts of combining hardware, software, and cloud services for environmental monitoring and control, providing a practical and easy-to-understand solution.

**Current Progress:**

1. **Basic Peripheral Hardware Testing:**
   - Achieved testing of basic peripheral hardware, including sensors, Wi-Fi, and LED lights.
2. **Alibaba Cloud Platform Integration:**
   - Completed device registration and connection testing with the Alibaba Cloud platform.
   - Conducted basic tests for data upload and message control command parsing.