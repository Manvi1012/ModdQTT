# MQTT-Modbus-RTOS

## Project Overview
This project implements a complete IoT ecosystem combining a local Modbus RTU network with cloud connectivity via MQTT. The system consists of three main devices communicating over serial interfaces, with one device acting as a gateway to an MQTT broker. The project demonstrates a custom Modbus stack implementation without relying on external Modbus libraries, alongside real-time task scheduling using FreeRTOS.

## System Architecture
The network is composed of three nodes:
* **Master (Gateway):** Acts as the Modbus Master and an MQTT client. It utilizes FreeRTOS to manage multiple tasks simultaneously. It polls the slave devices every 10 seconds to read temperature and relay states, displays the data on a local LCD, and publishes the information to an MQTT broker. It also subscribes to MQTT topics to receive remote commands and updates the local relays accordingly.
* **Controller (Slave ID: 2):** A Modbus Slave device connected to two relays. It processes incoming Modbus packets to either report the current state of the relays or change their state based on commands received from the Master.
* **Actuator (Slave ID: 1):** A Modbus Slave device connected to an LM75 temperature sensor via I2C. It utilizes an RTOS task to continuously read the temperature and transmits the data to the Master upon receiving a valid Modbus request.

## Implemented Modbus Functions
The custom Modbus stack features CRC16 validation and handles the following function codes:
* **01 (0x01) Read Coils:** Used by the Master to read the current states of the relays from the Controller.
* **04 (0x04) Read Input Registers:** Used by the Master to read the temperature data from the Actuator.
* **05 (0x05) Write Single Coil:** Implemented in the Master's software stack for relay state manipulation.
* **06 (0x06) Write Single Register:** Used by the Master to send control commands to the Controller to switch the relays on or off.

## Technologies and Dependencies
* **Microcontroller Environment:** ESP32 (or compatible boards with multiple hardware serial ports).
* **RTOS:** FreeRTOS (used for concurrent task management like LCD updating, temperature polling, and Modbus reading).
* **Protocols:** Modbus RTU (over Serial), MQTT (Secure/TLS), I2C.
* **Required Libraries:**
    * `WiFi.h` and `WiFiClientSecure.h` for network connectivity.
    * `PubSubClient.h` for MQTT communication.
    * `LiquidCrystal.h` for the 16x2 LCD display.
    * `Temperature_LM75_Derived.h` for the LM75 sensor.

## Project Structure
* `/master_ultimate`: Contains the source code for the Master/Gateway node.
* `/controller_without_lib`: Contains the source code for the Controller node.
* `/actuator_without_lib`: Contains the source code for the Actuator node.

## How to Run the Project

### 1. Hardware Setup
Connect the three devices using their hardware serial ports (Serial2):
* Connect the TX pin (GPIO 17) of the Master to the RX pin (GPIO 16) of the Slaves.
* Connect the RX pin (GPIO 16) of the Master to the TX pin (GPIO 17) of the Slaves.
* Ensure all connected devices share a common Ground (GND).
* **Actuator:** Connect the LM75 temperature sensor to the standard I2C pins.
* **Controller:** Connect the indicator LEDs or relays to GPIO 2 and GPIO 15.
* **Master:** Connect a 16x2 LCD using the defined pins (RS=5, EN=18, D4=19, D5=21, D6=22, D7=23).

### 2. Software Configuration
Before compiling and uploading the code, you must update the network credentials in the `master_ultimate` sketch:
* Change the `ssid` and `password` variables to match your local Wi-Fi network.
* If using a different MQTT broker, update `mqtt_server`, `mqtt_username`, `mqtt_password`, and `mqtt_port`. The current configuration connects securely to a HiveMQ Cloud cluster on port 8883.

### 3. Compilation and Flashing
* Open your preferred IDE.
* Install the required libraries mentioned in the dependencies section via your IDE's Library Manager.
* Open each sketch from their respective folders.
* Compile and upload the code to the Actuator and Controller boards first.
* Finally, compile and upload the Master code.

### 4. Operation and Monitoring
* Once powered, the Actuator and Controller will actively listen for Modbus queries.
* The Master will attempt to connect to the configured Wi-Fi network and the MQTT broker. You can monitor the connection status via the Serial Monitor (Baud Rate: 9600).
* The Master will poll the slaves every 10 seconds, update the LCD with the latest temperature and relay states, and publish a JSON payload to the `master/info` MQTT topic.
* To control the Controller's relays remotely, use an MQTT client to publish "COIL1ON", "COIL1OFF", "COIL2ON", or "COIL2OFF" to the `master/set` topic.

## Collaborators
This project was done in collaboration with:
* [AmirMohammad Talischian Tabrizi](https://github.com/Your-Link-Here)
* [Hamidreza Shafizadeh](https://github.com/Your-Link-Here)