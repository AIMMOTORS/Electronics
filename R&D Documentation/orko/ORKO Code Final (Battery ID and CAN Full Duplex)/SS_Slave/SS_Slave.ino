//Battery Circuit (BMS Datalogger) Code: Will fetch data from battery BMS via UART and send to bike IoT circuit via CAN upon its request
/*
  Author: Muhammad Hashir Bin Khalid
  Email: mohammadhashirbinkhalid@gmail.com
  Date: 23-11-2023
*/
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <EEPROM.h>
#include "batterystructs.h"
//#include <mcp_can.h>
#include <SoftwareSerial.h>

#include <mcp2515.h>

struct can_frame canMsg;

MCP2515 mcp2515(15);

//bool  canEnable, appEnable;
//bool canInitialized;

//bmsbasic mybms90;

#define SLOT_ID 1
#define buttonPin D4
SoftwareSerial Serialx(D1, D2);

/**
 * @brief Struct variables have been created for different DALY BMS IDs
 */
bmsData90 mybms90; //volt, current, soc
bmsData91 mybms91;
bmsData92 mybms92; //temp
bmsData93 mybms93;
bmsData95 mybms95;  //Defined Globally to avoid memory leakage
bmsData96 mybms96; 

// REPLACE WITH THE MAC Address of your sender/master       0x80, 0x7D, 0x3A, 0x12, 0x24, 0xA5 },       //80:7D:3A:12:24:A5   rehman sir
//uint8_t masterAddress[] = { 0x08, 0x3A, 0x8D, 0xE3, 0xC6, 0xD8 };   //08:3a:8d:e3:c6:d8   master 

//uint8_t masterAddress[] = { 0x08, 0x3A, 0x8D, 0xE3, 0xC6, 0xD8 };   // 08:3a:8d:e3:c6:d8  nayel2.8
uint8_t masterAddress[] = { 0x3C, 0x71, 0xBF, 0x3C, 0x3E, 0x04 };                              //3c:71:bf:3c:3e:04 SLAVE FOR TESTING
//uint8_t masterAddress[] = { 0xEC, 0x64, 0xC9, 0xCD, 0xBE, 0xD4 };    // ec:64:c9:cd:be:d4   MASTER FOR 2ND BIKE
//uint8_t masterAddress[] = { 0x80, 0x7D, 0x3A, 0x12, 0x24, 0xA5 };      //80:7D:3A:12:24:A5   rehman sir  master
uint8_t DEBUGLVL;

// Define variables to store DHT readings to be sent
float temperature;
float humidity;

// Define variables to store incoming readings
float incomingTemp;
float incomingHum;

// Updates DHT readings every 5 seconds
const long interval = 5000;
unsigned long previousMillis = 0;  // will store last time DHT was updated
bool dataSend=1;

// Variable to store if sending data was successful
String success;

//Structure example to send data
//Must match the receiver structure
typedef struct struct_request {
  bool d;
} struct_request;

typedef struct struct_response {
  unsigned int voltage[2];
  unsigned int current[2];
  unsigned int soc[2];
  byte temperature;
  uint32_t remaining_capacity_mah = 0;
  uint32_t remaining_capacity_ah[2];
  int Battery_SOH;
  int16_t rated_capacity[2]; 
  uint16_t cellVoltages[20];
  byte batteryMac[6];
  unsigned int battery_id[5];
  byte BPI;
  byte slot_id;
  byte flags;
  unsigned long time;
} struct_response;

// Create a struct_message called DHTReadings to hold sensor readings
struct_response response;

// Create a struct_message to hold incoming sensor readings
struct_request request;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Packet Sent to ");
  printMACAddress(mac_addr);
  Serial.print(",Status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");
  } else {
    Serial.println("Delivery fail");
  }
}

void printMACAddress(uint8_t *mac) {
  for (int i = 0; i < 6; ++i) {
    Serial.print(mac[i], HEX);
    if (i < 5) {
      Serial.print(':');
    }
  }
}

// Callback when data is received
void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&request, incomingData, sizeof(request));
  Serial.print("Bytes received: ");
  Serial.print(len);
  Serial.print(" from Master:");
  printMACAddress(mac);
  Serial.println();
  Serial.print("Data request:");
  Serial.println(request.d);
  response.time = millis();
  dataSend=1;
  // get_Basic_BMS_Data();
 // esp_now_send(masterAddress, (uint8_t *)&response, sizeof(response));
}

void setup() {
  // Init Serial Monitor
  Serial.begin(9600);
  delay(1000);
  setupUart();
  mcp2515.reset();
  mcp2515.setBitrate(CAN_250KBPS, MCP_8MHZ);  //Sets CAN at speed 500KBPS and Clock 8MHz
  mcp2515.setNormalMode();
  //setup_CAN();
  EEPROM.begin(512);
  delay(1000);
  pinMode(buttonPin, INPUT_PULLUP);
  Serial.println();
  WiFi.macAddress(response.batteryMac);
  Serial.print("I am a Node, my Mac address is: ");
  for(int i=0; i<6; i++){
     Serial.print(response.batteryMac[i],HEX);
  }
  Serial.println(" "); //000459
  for(int i = 0; i < 2; i++){
      response.battery_id[i] = 0;
  }
  response.battery_id[2] = 4; //battery id
  response.battery_id[3] = 5; //battery id
  response.battery_id[4] = 9; //battery id
  // response.current = 30000 + SLOT_ID;
  // response.voltage = 720 + SLOT_ID;
  // response.soc = 900 + SLOT_ID;
  // response.batteryMac[0] = SLOT_ID;
  // response.batteryMac[1] = SLOT_ID;
  // response.batteryMac[2] = SLOT_ID;
  // response.batteryMac[3] = SLOT_ID;
  // response.batteryMac[4] = SLOT_ID;
  // response.batteryMac[5] = SLOT_ID;
  response.BPI = SLOT_ID * 10;
  response.slot_id = SLOT_ID;
  response.temperature = 0;  //30 + SLOT_ID;  //Disabled feature

  // Set device as a Wi-Fi Station
//  WiFi.mode(WIFI_STA);
//  WiFi.disconnect();
//
//  // Init ESP-NOW
//  if (esp_now_init() != 0) {
//    Serial.println("Error initializing ESP-NOW");
//    return;
//  }
//
//  // Set ESP-NOW Role
//  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
//
//  // Once ESPNow is successfully Init, we will register for Send CB to
//  // get the status of Trasnmitted packet
//  esp_now_register_send_cb(OnDataSent);
//
//  // Register peer
//  esp_now_add_peer(masterAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
//
//  // Register for a callback function that will be called when data is received
//  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  // if (digitalRead(buttonPin) == LOW) {
  //   response.flags = 0x01;
  // } else {
  //   response.flags = 0x07;
  // }

  //if (millis() - previousMillis > interval) {
   // previousMillis = millis();
//    if(dataSend)
//    {
  // //dataSend=0;
  getbmsData93();
  getbmsData90();
  //getbmsData91();
  getbmsData92();
  getbmsData95();
//response.current[0]; 
//response.current[1];
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK)  // To receive data (Poll Read)
  {
    Serial.print("CAN ID: ");
    Serial.println(canMsg.can_id, HEX);
    if (canMsg.can_id == 0x32)  // getting  00000063 req from trakker
    {
      Serial.println("Received Request ID 32 from Bike IoT");
      canMsg.can_id = 0x23;     //CAN id as 0x036
      canMsg.can_dlc = 8;       //CAN data length as 8
      canMsg.data[0] = response.voltage[0];  //Voltage [0] 
      canMsg.data[1] = response.voltage[1];  //Voltage [1] 
      canMsg.data[2] = response.temperature; //Temp
      canMsg.data[3] = response.soc[0];  //SoC [0]
      canMsg.data[4] = response.soc[1];  //SoC [0]
      canMsg.data[5] = 0x00; //For speed
      canMsg.data[6] = response.Battery_SOH;
      canMsg.data[7] = 0x00;
      mcp2515.sendMessage(&canMsg);  //Sends the CAN message
      Serial.print("Voltage: "); Serial.print(canMsg.data[0],HEX); Serial.println(canMsg.data[1],HEX);

      canMsg.can_id = 0x43;     //CAN id as 0x036
      canMsg.can_dlc = 8;       //CAN data length as 8
      for (int a = 0; a < 8; a++) { 
          canMsg.data[a] = (response.cellVoltages[a]/100);
      }  
         
      mcp2515.sendMessage(&canMsg);
      for (int i = 0; i < 8; i++) {
          Serial.print(canMsg.data[i], HEX);    
      }
      Serial.println();
      Serial.println("===============");

      canMsg.can_id = 0x53;     //CAN id as 0x036
      canMsg.can_dlc = 8;       //CAN data length as 8
      for (int a = 0; a < 8; a++) { 
          canMsg.data[a] = (response.cellVoltages[a+8]/100);
      }

      mcp2515.sendMessage(&canMsg);
      for (int i = 0; i < 8; i++) {
          Serial.print(canMsg.data[i], HEX);
      }
      Serial.println();
      Serial.println("===============");

      canMsg.can_id = 0x54;     //CAN id as 0x036
      canMsg.can_dlc = 8;       //CAN data length as 8
      canMsg.data[0] = (response.cellVoltages[16]/100);
      canMsg.data[1] = (response.cellVoltages[17]/100);
      canMsg.data[2] = (response.cellVoltages[18]/100);
      canMsg.data[3] = (response.cellVoltages[19]/100);
      canMsg.data[4] = response.current[0]; 
      canMsg.data[5] = response.current[1];  
      canMsg.data[6] = 0x00; 
      canMsg.data[7] = 0x00; 
      // canMsg.data[4] = response.batteryMac[2]; //Last 4 bytes of MAC address
      // canMsg.data[5] = response.batteryMac[3]; //Last 4 bytes of MAC address
      // canMsg.data[6] = response.batteryMac[4]; //Last 4 bytes of MAC address
      // canMsg.data[7] = response.batteryMac[5]; //Last 4 bytes of MAC address

      mcp2515.sendMessage(&canMsg);
      for (int i = 0; i < 8; i++) {
          Serial.print(canMsg.data[i], HEX);
      }
      // Serial.print("MAC: ");
      // for (int i = 4; i < 8; i++) {
      //     Serial.print(canMsg.data[i], HEX);
      // }
      Serial.println();
      Serial.println("===============");


      canMsg.can_id = 0x55;     //CAN id as 0x036
      canMsg.can_dlc = 8;       //CAN data length as 8
      canMsg.data[0] = response.remaining_capacity_ah[0];
      canMsg.data[1] = response.remaining_capacity_ah[1];
      canMsg.data[2] = response.batteryMac[2]; //Last 4 bytes of MAC address
      canMsg.data[3] = response.batteryMac[3]; //Last 4 bytes of MAC address
      canMsg.data[4] = response.batteryMac[4]; //Last 4 bytes of MAC address
      canMsg.data[5] = response.batteryMac[5]; //Last 4 bytes of MAC address
      canMsg.data[6] = 0x00;
      canMsg.data[7] = 0x00;

      mcp2515.sendMessage(&canMsg);
      for (int i = 0; i < 8; i++) {
          Serial.print(canMsg.data[i], HEX);
      }
      Serial.print("MAC: ");
      for (int i = 2; i < 6; i++) {
          Serial.print(canMsg.data[i], HEX);
      }
      Serial.println();
      Serial.println("===============");

      canMsg.can_id = 0x56;     //CAN id as 0x036
      canMsg.can_dlc = 8;       //CAN data length as 8
      canMsg.data[0] = response.battery_id[0];
      canMsg.data[1] = response.battery_id[1];
      canMsg.data[2] = response.battery_id[2];
      canMsg.data[3] = response.battery_id[3];
      canMsg.data[4] = response.battery_id[4];
      canMsg.data[5] = 0x00;
      canMsg.data[6] = 0x00;
      canMsg.data[7] = 0x00; //00001

      mcp2515.sendMessage(&canMsg);
      Serial.print("Battery ID: ");
      for (int i = 0; i < 5; i++) {
          Serial.print(canMsg.data[i], HEX); 
      }
      Serial.println();
      Serial.println("===============");
         
    }
  } 
  
  //delay(100);

      //update_Basic_BMS_Data();
      // updateBatteryMacAndBPI();
      //esp_now_send(masterAddress, (uint8_t *)&response, sizeof(response));
  //} //datasend
}
