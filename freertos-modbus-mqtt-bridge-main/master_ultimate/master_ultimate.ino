// Libraries
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <LiquidCrystal.h>

// Setting Serial2 RX and TX pins
#define RXD2 16
#define TXD2 17
#define BAUD_RATE 9600
#define TimeOut 1000  // ms

const int T_3_5 = 3.5*1000*10/BAUD_RATE+1; // Silent interval in milliseconds for BAUD_RATE (8N1 -> 10) (4ms for BAUD_RATE = 9600)
unsigned long lastReceiveTime = 0;
uint8_t buffer[256];  // as the buffer size of serial buffer is 64 byte and the maximun size of a modbus packet is 256 byte we need to save the incomming packet to avoid data lost
uint16_t temp;  // temperature value after reading from Input Register

// WiFi settings
const char* ssid = "I am nothing";
const char* password = "passwordofnothing";
// MQTT Broker settings
const char* mqtt_server = "9b2daa81f41940748fd1974985aa4ce2.s1.eu.hivemq.cloud"; // broker url
const char* mqtt_username = "ESP32";
const char* mqtt_password = "ESP32iot";
const int mqtt_port = 8883;
// SSL Certificate
static const char *root_ca PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

// For secure connection to the server (because HiveMQ doesn't work without this)
WiFiClientSecure espClient;
PubSubClient client(espClient);

// Coils Status
String COIL1_STATUS = "COIL1OFF";
String COIL2_STATUS = "COIL2OFF";

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 5, en = 18, d4 = 19, d5 = 21, d6 = 22, d7 = 23;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

uint16_t crc16(const uint8_t* data, size_t length){
  static const uint16_t wCRCTable[] = {
    0X0000, 0XC0C1, 0XC181, 0X0140, 0XC301, 0X03C0, 0X0280, 0XC241,
    0XC601, 0X06C0, 0X0780, 0XC741, 0X0500, 0XC5C1, 0XC481, 0X0440,
    0XCC01, 0X0CC0, 0X0D80, 0XCD41, 0X0F00, 0XCFC1, 0XCE81, 0X0E40,
    0X0A00, 0XCAC1, 0XCB81, 0X0B40, 0XC901, 0X09C0, 0X0880, 0XC841,
    0XD801, 0X18C0, 0X1980, 0XD941, 0X1B00, 0XDBC1, 0XDA81, 0X1A40,
    0X1E00, 0XDEC1, 0XDF81, 0X1F40, 0XDD01, 0X1DC0, 0X1C80, 0XDC41,
    0X1400, 0XD4C1, 0XD581, 0X1540, 0XD701, 0X17C0, 0X1680, 0XD641,
    0XD201, 0X12C0, 0X1380, 0XD341, 0X1100, 0XD1C1, 0XD081, 0X1040,
    0XF001, 0X30C0, 0X3180, 0XF141, 0X3300, 0XF3C1, 0XF281, 0X3240,
    0X3600, 0XF6C1, 0XF781, 0X3740, 0XF501, 0X35C0, 0X3480, 0XF441,
    0X3C00, 0XFCC1, 0XFD81, 0X3D40, 0XFF01, 0X3FC0, 0X3E80, 0XFE41,
    0XFA01, 0X3AC0, 0X3B80, 0XFB41, 0X3900, 0XF9C1, 0XF881, 0X3840,
    0X2800, 0XE8C1, 0XE981, 0X2940, 0XEB01, 0X2BC0, 0X2A80, 0XEA41,
    0XEE01, 0X2EC0, 0X2F80, 0XEF41, 0X2D00, 0XEDC1, 0XEC81, 0X2C40,
    0XE401, 0X24C0, 0X2580, 0XE541, 0X2700, 0XE7C1, 0XE681, 0X2640,
    0X2200, 0XE2C1, 0XE381, 0X2340, 0XE101, 0X21C0, 0X2080, 0XE041,
    0XA001, 0X60C0, 0X6180, 0XA141, 0X6300, 0XA3C1, 0XA281, 0X6240,
    0X6600, 0XA6C1, 0XA781, 0X6740, 0XA501, 0X65C0, 0X6480, 0XA441,
    0X6C00, 0XACC1, 0XAD81, 0X6D40, 0XAF01, 0X6FC0, 0X6E80, 0XAE41,
    0XAA01, 0X6AC0, 0X6B80, 0XAB41, 0X6900, 0XA9C1, 0XA881, 0X6840,
    0X7800, 0XB8C1, 0XB981, 0X7940, 0XBB01, 0X7BC0, 0X7A80, 0XBA41,
    0XBE01, 0X7EC0, 0X7F80, 0XBF41, 0X7D00, 0XBDC1, 0XBC81, 0X7C40,
    0XB401, 0X74C0, 0X7580, 0XB541, 0X7700, 0XB7C1, 0XB681, 0X7640,
    0X7200, 0XB2C1, 0XB381, 0X7340, 0XB101, 0X71C0, 0X7080, 0XB041,
    0X5000, 0X90C1, 0X9181, 0X5140, 0X9301, 0X53C0, 0X5280, 0X9241,
    0X9601, 0X56C0, 0X5780, 0X9741, 0X5500, 0X95C1, 0X9481, 0X5440,
    0X9C01, 0X5CC0, 0X5D80, 0X9D41, 0X5F00, 0X9FC1, 0X9E81, 0X5E40,
    0X5A00, 0X9AC1, 0X9B81, 0X5B40, 0X9901, 0X59C0, 0X5880, 0X9841,
    0X8801, 0X48C0, 0X4980, 0X8941, 0X4B00, 0X8BC1, 0X8A81, 0X4A40,
    0X4E00, 0X8EC1, 0X8F81, 0X4F40, 0X8D01, 0X4DC0, 0X4C80, 0X8C41,
    0X4400, 0X84C1, 0X8581, 0X4540, 0X8701, 0X47C0, 0X4680, 0X8641,
    0X8201, 0X42C0, 0X4380, 0X8341, 0X4100, 0X81C1, 0X8081, 0X4040 };

  uint8_t nTemp;
  uint16_t wCRCWord = 0xFFFF;

  while (length--)
  {
    nTemp = *data++ ^ wCRCWord;
    wCRCWord >>= 8;
    wCRCWord  ^= wCRCTable[nTemp];
  }
  return wCRCWord;
} // End: CRC16

void Send_Modbus_packet(uint8_t Slave_Address, uint8_t Function_Code, uint16_t Starting_Address, uint16_t Data){
  while(millis() - lastReceiveTime <= T_3_5);
  uint8_t Starting_Address_LO = Starting_Address;
  uint8_t Starting_Address_HI = Starting_Address >> 8;
  uint8_t Data_LO = Data;
  uint8_t Data_HI = Data >> 8;

  uint8_t packet[] = {Slave_Address, Function_Code, Starting_Address_HI, Starting_Address_LO, Data_HI, Data_LO, 0, 0};
  uint16_t CRC = crc16(packet, 6);
  uint8_t CRC_LO = CRC;
  uint8_t CRC_HI = CRC >> 8;
  packet[7] = CRC_HI;
  packet[6] = CRC_LO; // {CRC_HI, CRC_LO} is correct not {CRC_LO, CRC_HI}
  while(Serial2.available())
    Serial2.read();   // clearing buffer before starting transmition
  Serial2.write(packet, 8);
  delay(T_3_5);
  lastReceiveTime = millis();
}

int Read_Coils(uint8_t Slave_Address, uint16_t Starting_Address, uint16_t Quantity, uint8_t* Data){
  Send_Modbus_packet(Slave_Address, 1, Starting_Address, Quantity);
  
  uint8_t ind = 0;
  while(millis() - lastReceiveTime < TimeOut && !Serial2.available());
  while(Serial2.available()){
    Serial2.readBytes(&buffer[ind++], 1); // any Serial2.read try to read buffer but if it cant do that after this timeout(1 second by default), it returns 0 (means number of readed bytes is zero)
    if(!Serial2.available())
      delay(T_3_5); // delay to ensure that a complete Moodbus packet has recieved
  }

  Serial.print("Recieved packet: ");
  for (uint8_t i = 0; i < ind; i++){
    Serial.print(buffer[i], HEX);
    Serial.print(' ');
  }
  Serial.print('\n');

  if(ind < 6)
    return 1; // Timeout Error (or Invalid packet structure)
  uint8_t Recieved_Slave_Address = buffer[0];
  if(Recieved_Slave_Address != Slave_Address)
    return 2; // Error: Recieved_Slave_Address does not match with Slave_Address
  uint8_t Recieved_Function_Code = buffer[1];
  if(Recieved_Function_Code >= 128)
    return 3; // Error: Modbus Exception Code
  if(Recieved_Function_Code != 1)
    return 4; // Error: Invalid Function Code
  uint8_t Bytes_Count = buffer[2];
  if(Bytes_Count*8-Quantity >= 8)
    return 5; // Error: Invalid Bytes Count

  uint16_t Recieved_CRC16 = (buffer[Bytes_Count+4] << 8) | buffer[Bytes_Count+3];
  if(Recieved_CRC16 != crc16(buffer, Bytes_Count+3))
    return 6; // CRC Erorr
  for(uint8_t i = 0;i < Bytes_Count;i++)
    Data[i] = buffer[3+i];
  return 0; // Success
}

int Read_Input_Registers(uint8_t Slave_Address, uint16_t Starting_Address, uint16_t Quantity, uint16_t* Data){
  Send_Modbus_packet(Slave_Address, 4, Starting_Address, Quantity);
  
  uint8_t ind = 0;
  while(millis() - lastReceiveTime < TimeOut && !Serial2.available());
  while(Serial2.available()){
    Serial2.readBytes(&buffer[ind++], 1); // any Serial2.read try to read buffer but if it cant do that after this timeout(1 second by default), it returns 0 (means number of readed bytes is zero)
    if(!Serial2.available())
      delay(T_3_5); // delay to ensure that a complete Moodbus packet has recieved
  }
  Serial.print("Recieved packet: ");
  for (uint8_t i = 0; i < ind; i++){
    Serial.print(buffer[i], HEX);
    Serial.print(' ');
  }
  Serial.print('\n');

  if(ind < 6)
    return 1; // Timeout Error (or Invalid packet structure)
  uint8_t Recieved_Slave_Address = buffer[0];
  if(Recieved_Slave_Address != Slave_Address)
    return 2; // Error: Recieved_Slave_Address does not match with Slave_Address
  uint8_t Recieved_Function_Code = buffer[1];
  if(Recieved_Function_Code >= 128)
    return 3; // Error: Modbus Exception Code
  if(Recieved_Function_Code != 4)
    return 4; // Error: Invalid Function Code
  uint8_t Bytes_Count = buffer[2];
  if(Bytes_Count != 2*Quantity)
    return 5; // Error: Invalid Bytes Count

  uint16_t Recieved_CRC16 = (buffer[Bytes_Count+4] << 8) | buffer[Bytes_Count+3];
  if(Recieved_CRC16 != crc16(buffer, Bytes_Count+3))
    return 6; // CRC Erorr
  for(uint8_t i = 0;i < Bytes_Count;i+=2)
    Data[i] = (buffer[i+3] << 8) | buffer[i+4];
  return 0; // Success
}

int Handle_Write_Operations(uint8_t Slave_Address, uint8_t FunctionCode, uint16_t Starting_Address, uint16_t Data){
  // any Serial2.read try to read buffer but if it cant do that after this timeout(1 second by default), it returns 0 (means number of readed bytes is zero)
  
  uint8_t ind = 0;
  while(millis() - lastReceiveTime < TimeOut && !Serial2.available());
  while(Serial2.available()){
    Serial2.readBytes(&buffer[ind++], 1); // any Serial2.read try to read buffer but if it cant do that after this timeout(1 second by default), it returns 0 (means number of readed bytes is zero)
    if(!Serial2.available())
      delay(T_3_5); // delay to ensure that a complete Moodbus packet has recieved
  }

  Serial.print("Recieved packet: ");
  for (uint8_t i = 0; i < ind; i++){
    Serial.print(buffer[i], HEX);
    Serial.print(' ');
  }
  Serial.print('\n');

  if(ind < 8)
    return 1; // Timeout Error (or Invalid packet structure)
  uint8_t Recieved_Slave_Address = buffer[0];
  if(Recieved_Slave_Address != Slave_Address)
    return 2; // Error: Recieved_Slave_Address does not match with Slave_Address
  uint8_t Recieved_Function_Code = buffer[1];
  if(Recieved_Function_Code >= 128)
    return 3; // Error: Modbus Exception Code
  if(Recieved_Function_Code != FunctionCode)
    return 4; // Error: Invalid Function Code
  uint16_t Recieved_CRC16 = (buffer[7] << 8) | buffer[6];
  if(Recieved_CRC16 != crc16(buffer, 6))
    return 6; // CRC Erorr
  if(((buffer[2] << 8) | buffer[3]) != Starting_Address || ((buffer[4] << 8) | buffer[5]) != Data)
    return 7; // Request and Response are not same
  return 0; // Success
}

int Write_Single_Register(uint8_t Slave_Address, uint16_t Starting_Address, uint16_t Data){
  Send_Modbus_packet(Slave_Address, 6, Starting_Address, Data);
  return Handle_Write_Operations(Slave_Address, 6, Starting_Address, Data);
}

int Write_Single_Coil(uint8_t Slave_Address, uint16_t Starting_Address, uint16_t Data){
  Send_Modbus_packet(Slave_Address, 5, Starting_Address, Data);
  return Handle_Write_Operations(Slave_Address, 5, Starting_Address, Data);
}

void reconnect() {
  // Loop until we’re reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    String clientId = "ESP32Client-"; // Create a random client ID
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("connected");

      client.subscribe("master/set");   // subscribe the topics here
    }
    else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");   // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// if a message comes from dashboard this function handles it
void callback(char* topic, byte* payload, unsigned int length) {
  String incommingMessage = "";
  for (int i = 0; i < length; i++)
    incommingMessage += (char) payload[i];
  Serial.println("Message arrived topic: " + String(topic) + ", message: " + incommingMessage);
  if(incommingMessage == "COIL1ON" || incommingMessage == "COIL1OFF")
    COIL1_STATUS = incommingMessage;

  else if(incommingMessage == "COIL2ON" || incommingMessage ==    "COIL2OFF")
    COIL2_STATUS = incommingMessage;
  
  while(Write_Single_Register(2, 1, (COIL2_STATUS == "COIL2ON")*2+(COIL1_STATUS == "COIL1ON")));
  update_information();
}

// function to update dashboard information
void update_information(){
  // sending to MQTT Broker
  String msg = "{\"temperature\": " + String(temp) + ", \"coil1\": \"" + COIL1_STATUS + "\", \"coil2\": \"" + COIL2_STATUS + "\"}";
  if(client.publish("master/info", msg.c_str())){
    Serial.println("informations updated successfully");
  }
  else
    Serial.println("failed to publish informations");
  lcd.clear();
}

// RTOS task function to monitor data in LCD
void LCD_Task( void* pvParameters){
  for(;;){
    lcd.setCursor(0, 0);
    lcd.print("Temp: "+String(temp));
    lcd.setCursor(0, 1);
    lcd.print("C1: " + COIL1_STATUS.substring(5) + " C2: " + COIL2_STATUS.substring(5));
    delay(70);
  }
}

void Read_Modbus_Task( void* pvParameters){
  // infinity loop of this RTOS Task
  for(;;){
    uint8_t coils;
    int status = Read_Coils(2, 1, 2, &coils);
    Serial.print("Read coil Status Code: ");
    Serial.println(status);
    Serial.print("coils: ");
    Serial.println(coils);
    delay(10);
    status = Read_Input_Registers(1, 1, 1, &temp);
    Serial.print("Read Input Registers Status Code: ");
    Serial.println(status);
    Serial.print("Temp: ");
    Serial.println(temp);

    if(coils & 1)
      COIL1_STATUS = "COIL1ON";
    else
      COIL1_STATUS = "COIL1OFF";
    if(coils & 2)
      COIL2_STATUS = "COIL2ON";
    else
      COIL2_STATUS = "COIL2OFF";
    
    update_information();
    delay(10000); // every 10 second

    // Passing control to other tasks when called. Ideally yield() should be used in functions that will take awhile to complete.
    yield();
  }
}

void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  Serial.begin(9600);
  Serial2.begin(BAUD_RATE, SERIAL_8N1, RXD2, TXD2);
  
  // Creating RTOS task 
  xTaskCreatePinnedToCore(
    Read_Modbus_Task,         // function name
    "Read_Modbus_Task",       // a name just for humans
    4096,                     // Stack size
    NULL,                     // input parameters
    2,                        // priority (0 to 3) higher has more priority
    NULL,                     // handler
    xPortGetCoreID()          // core name xPortGetCoreID() returns current cpu core id (can be 0 or 1 for ESP32)
  );

  // Creating RTOS task for LCD
  xTaskCreatePinnedToCore(
    LCD_Task,                 // function name
    "LCD_Task",               // a name just for humans
    2048,                     // Stack size
    NULL,                     // input parameters
    2,                        // priority (0 to 3) higher has more priority
    NULL,                     // handler
    xPortGetCoreID()          // core name xPortGetCoreID() returns current cpu core id (can be 0 or 1 for ESP32)
  );

  // Connecting to Wi-Fi
  Serial.print("\nConnecting to ");
  Serial.print(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Waiting for Connection
  while (WiFi.status() != WL_CONNECTED) {
    lcd.print("WiFi Connecting");
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nWiFi connected\nIP address: ");
  Serial.println(WiFi.localIP());

  // random seed
  randomSeed(micros());
  espClient.setCACert(root_ca);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

}

void loop() {
  // MQTT Connection
  if (!client.connected())
    reconnect();
  client.loop();

  delay(50);
  yield();
}
