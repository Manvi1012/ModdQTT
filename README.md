# FreeRTOS Industrial IoT Gateway (Modbus RTU to MQTT)

A complete end-to-end Industrial IoT ecosystem featuring a local **Modbus RTU** serial network integrated with cloud connectivity via **MQTT (TLS/HiveMQ Cloud)**. Built on **ESP32** controllers using **FreeRTOS** for concurrent task scheduling, this project implements a custom, bare-metal Modbus RTU protocol stack with **CRC16 validation** without relying on third-party Modbus libraries.

---

 📌 System Architecture

The network consists of three distinct nodes communicating over hardware serial interfaces:

1. **Master Gateway (`/master_ultimate`)**
   * Acts as the **Modbus Master** and **MQTT Client**.
   * Uses **FreeRTOS** multi-tasking to poll slaves every 10 seconds, update a local 16x2 LCD display, and handle network traffic.
   * Publishes sensor data telemetry to HiveMQ Cloud over secure TLS.
   * Subscribes to MQTT command topics for remote bi-directional control.

2. **Controller Slave (ID: 2) (`/controller_without_lib`)**
   * Modbus RTU Slave connected to a dual-relay module.
   * Processes Function Codes `01`, `05`, and `06` to read coil statuses and toggle relay outputs.

3. **Actuator Slave (ID: 1) (`/actuator_without_lib`)**
   * Modbus RTU Slave connected to an **LM75 temperature sensor** over I2C.
   * Continuously samples ambient temperature via an RTOS task and returns data to the Master upon receiving Function Code `04` requests.

```
       +------------------------------------+
       |          HiveMQ Cloud              |
       +------------------------------------+
                         ^
                    MQTT | (TLS / Wi-Fi)
                         v
       +------------------------------------+
       |     ESP32 Master Gateway           |
       |  - FreeRTOS Task Scheduler         |
       |  - Custom Modbus Stack             |
       |  - 16x2 LCD Display                |
       +------------------------------------+
                 /                \
   Serial (UART) /                  \ Serial (UART)
   Modbus RTU   /                    \ Modbus RTU
               v                      v
+-----------------------+   +-----------------------+
|  Actuator (Slave 1)   |   |  Controller (Slave 2) |
|  - LM75 Temp (I2C)    |   |  - Dual Relay Output  |
+-----------------------+   +-----------------------+
```

---

## 🛠 Implemented Modbus Functions

The bare-metal Modbus stack implements **CRC16** frame validation and supports the following standard function codes:

| Function Code | Hex | Description | Usage |
| :--- | :--- | :--- | :--- |
| **01** | `0x01` | Read Coils | Queries current relay states from Controller |
| **04** | `0x04` | Read Input Registers | Reads temperature measurements from Actuator |
| **05** | `0x05` | Write Single Coil | Software stack handling for coil manipulation |
| **06** | `0x06` | Write Single Register | Sends relay switching control commands to Controller |

---

## 💻 Tech Stack & Dependencies

* **Hardware:** ESP32 Microcontrollers, LM75 I2C Sensor, Dual Relay Module, 16x2 LCD Display.
* **Core Operating System:** FreeRTOS.
* **Protocols:** Modbus RTU, MQTT over TLS/SSL, I2C, Hardware Serial (UART2).
* **Libraries:**
  * `WiFi.h` & `WiFiClientSecure.h` (Network & TLS)
  * `PubSubClient.h` (MQTT messaging)
  * `LiquidCrystal.h` (16x2 LCD interface)
  * `Temperature_LM75_Derived.h` (I2C sensor reading)

---

## 📁 Repository Structure

```
├── master_ultimate/          # Master Gateway firmware (FreeRTOS, MQTT, LCD, Modbus Master)
├── controller_without_lib/   # Controller Slave firmware (Modbus Slave ID 2, Relays)
└── actuator_without_lib/     # Actuator Slave firmware (Modbus Slave ID 1, LM75 Sensor)
```

---

## 🚀 How to Run

### 1. Hardware Connections
Connect all three ESP32 boards over shared Hardware Serial (`Serial2`):
* Connect Master **TX (GPIO 17)** to Slaves **RX (GPIO 16)**.
* Connect Master **RX (GPIO 16)** to Slaves **TX (GPIO 17)**.
* **Common Ground:** Ensure all boards share a common `GND`.
* **Actuator Pinout:** Connect LM75 to default I2C pins (`SDA`: GPIO 21, `SCL`: GPIO 22).
* **Controller Pinout:** Connect Relay Module to `GPIO 2` and `GPIO 15`.
* **Master Pinout:** Connect 16x2 LCD (`RS`: 5, `EN`: 18, `D4`: 19, `D5`: 21, `D6`: 22, `D7`: 23).

### 2. Software Setup & Flashing
1. Update Wi-Fi and HiveMQ credentials in `/master_ultimate/master_ultimate.ino`:
   ```cpp
   const char* ssid = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
   const char* mqtt_server = "YOUR_HIVEMQ_CLUSTER_URL";
   ```
2. Flash `actuator_without_lib` to Slave 1.
3. Flash `controller_without_lib` to Slave 2.
4. Flash `master_ultimate` to the Master Gateway board.

### 3. MQTT Payload & Remote Control
* **Telemetry Payload (`master/info` topic):**
  ```json
  {
    "temperature": 26.5,
    "relay1": "OFF",
    "relay2": "ON"
  }
  ```
* **Remote Commands (`master/set` topic):**
  Publish strings `COIL1ON`, `COIL1OFF`, `COIL2ON`, or `COIL2OFF` to manipulate relay channels remotely.

---
