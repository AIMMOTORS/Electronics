//Bike IoT Circuit Code: It will request data from Battery IoT circuit via CAN and also send to the Tracker via CAN

#include "motorstructs.h"
#include "batterystructs.h"
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <espnow.h>
#include <mcp2515.h>

struct can_frame canMsg;

MCP2515 mcp2515(15);

// SoftwareSerial SerialBattery(D1, D2);  //BMS serial

bmsData90 mybms90;
bmsData91 mybms91;
bmsData92 mybms92;
bmsData93 mybms93;
bmsData95 mybms95;  //Defined Globally to avoid memory leakage
bmsData96 mybms96;

/*
following is the complete list in order of importance 
1. State of Charge 
2. Cycle Count
3. State of Health 
4. Total Voltage 
5.. Battery current
6. Battery Temperature 
7  Design_Capacity
8. Full_Capacity
9. Remaining_Capacity
*/

typedef struct struct_response {
  unsigned int voltage[2];
  unsigned int current[2];
  unsigned int soc[2];
  byte temperature;
  uint32_t remaining_capacity_mah = 0;
  uint32_t remaining_capacity_ah[2];
  int Battery_SOH;
  int16_t rated_capacity[2]; //design capacity
  uint16_t cellVoltages[20];
  byte batteryMac[6];
  unsigned int battery_id[5]; //To store "AIMBT-0001"
  byte BPI;
  byte slot_id;
  byte flags;
  unsigned long time;
} struct_response;

// Create a struct_message called DHTReadings to hold sensor readings
struct_response response;

unsigned int design_capacity = 28; //28 Ah 
unsigned int full_capacity = 30; //30 Ah When battery becomes fully charged
// unsigned int design_capacity = 36; //28 Ah 
// unsigned int full_capacity = 36; //30 Ah When battery becomes fully charged

// unsigned int design_capacity = 44; //28 Ah 
// unsigned int full_capacity = 44; //30 Ah When battery becomes fully charged

//unsigned int design_capacity = 48; //28 Ah 

uint8_t rxB[8];
uint8_t rxB1[8];

#define SS_FORMAT 0     //for maaster
uint8_t MAX_temp;
//uint8_t rxB1[8];
long unsigned int rxId;
uint8_t len = 0;
uint8_t DEBUGLVL = 1;

uint8_t SpeedH = 0;
uint8_t Pack_Voltage_H;
uint8_t Pack_Voltage_L;

void setup() {
  Serial.begin(9600);
  delay(1000);
  //Master_setup();
  setupUart(); //motor uart
  //batteryUart();



  SPI.begin();  //Begins SPI communication

  mcp2515.reset();
  mcp2515.setBitrate(CAN_250KBPS, MCP_8MHZ);  //Sets CAN at speed 500KBPS and Clock 8MHz
  mcp2515.setNormalMode();
}

void loop() {
  //UART Motor controller
   bikeDataStruct ud = getUartData(); //Motor UART
  //delay(1000);
  //getbmsData90(); //BMS UART
  //getbmsData92();
   Master_loop();
   //Serial.println("Hello: ");

  //delay(1000);
  // uint8_t track_txB1[8] = { rxB[0], rxB[1], rxB[4], rxB[5], rxB[6], rxB[7], SpeedH, Pack_Voltage_H };
//  uint8_t track_txB1[8] = {highByte(response.voltage), lowByte(response.voltage),response.temperature-40,  highByte(response.soc), lowByte(response.soc), SpeedH, response.Battery_SOH,0xFA };
//  // // Serial.println(response.Battery_SOH);
//  for (int i = 0; i < 8; i++) {
//    Serial.print(track_txB1[i], HEX);  //hex mn print krn
//  }
//   Serial.println();
//   Serial.println("  ====");
//   if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK)  // To receive data (Poll Read)
//   {

//     //    uint8_t x = canMsg.data[0];
//     //    uint8_t y = canMsg.data[1];
//     //     uint8_t a = canMsg.data[2]; tracker pr send krn // abhi idhr array check krrhai hain k data full hy k empty
//     //      uint8_t b = canMsg.data[3];
//     //       uint8_t c = canMsg.data[4];
//     //         uint8_t D = canMsg.data[5];

//     //  uint8_t x1 = canMsg.data[6];
//     //    uint8_t y1 = canMsg.data[7];


//     Serial.print("CAN ID: ");
//     Serial.println(canMsg.can_id, HEX);
//     if (canMsg.can_id == 0x63)  // getting  00000063 req from trakker
//     {
//       Serial.println("Received Request ID 63 from Tracker");
//       canMsg.can_id = 0x36;     //CAN id as 0x036
//       canMsg.can_dlc = 8;       //CAN data length as 8
//       canMsg.data[0] = response.voltage[0];  //Voltage [0] 
//       canMsg.data[1] = response.voltage[1];  //Voltage [1] 
//       canMsg.data[2] = response.temperature; //Temp  
//       canMsg.data[3] = response.soc[0];  //SoC [0]
//       canMsg.data[4] = response.soc[1];  //SoC [1]
//       canMsg.data[5] = response.current[0]; 
//       canMsg.data[6] = response.current[1]; 
//       canMsg.data[7] = SpeedH;  //Speed

//       //uint8_t canMsg.data[8] = {rxB[0], rxB[1], rxB[4], rxB[5], rxB[6], rxB[7], 0x00, 0xF0 };


//       mcp2515.sendMessage(&canMsg);  //Sends the CAN message
//       // for (int i = 0; i < 8; i++) {
//       //   Serial.print(canMsg.data[i], HEX);
//       // }
//       // Serial.println();
//       // Serial.println("===============");

//       // delay(1000);
//     }
//     //==========================================================================

//     if (canMsg.can_id == 0x73)  // getting  00000063 req from trakker
//     {
//       Serial.println("Received Request ID 73 from Tracker");
//       canMsg.can_id = 0x37;   //CAN id as 0x036
//       canMsg.can_dlc = 8;     //CAN data length as 8
//       for (int a = 0; a < 8; a++) { 
//          canMsg.data[a] = (response.cellVoltages[a]/100);
//       }

//       mcp2515.sendMessage(&canMsg);  //Sends the CAN message
//       for (int i = 0; i < 8; i++) {
//        Serial.print(canMsg.data[i], HEX);
    
//       }
//       Serial.println();
//       Serial.println("===============");
//       //delay(1000);
//     //}     can  req 73 loop ends 
  

// //  if (canMsg.can_id == 0x83)  // getting  00000063 req from trakker
// //      {
//       Serial.println("Received Request ID 83 from Tracker");
//       canMsg.can_id = 0x38;   //CAN id as 0x036
//       canMsg.can_dlc = 8;     //CAN data length as 8
//      for (int a = 0; a < 8; a++) { 
//       canMsg.data[a] = (response.cellVoltages[a+8]);
//       }
      
//       mcp2515.sendMessage(&canMsg);  //Sends the CAN message
//       for (int i = 0; i < 8; i++) {
//        Serial.print(canMsg.data[i], HEX);
    
//       }
//       Serial.println();
//       Serial.println("===============");
//       //delay(1000);
//    // }
  

// // if (canMsg.can_id == 0x93)  // getting  00000063 req from trakker
// //     {
//       Serial.println("Received Request ID 93 from Tracker");
//       canMsg.can_id = 0x39;   //CAN id as 0x036
//       canMsg.can_dlc = 8;     //CAN data length as 8
//     //  for (int a = 0; a <4 ; a++) { 
//     //   canMsg.data[a] = (response.cellVoltages[a+16]/100);
//     //   }
//           canMsg.data[0] = (response.cellVoltages[16]/100);
//           canMsg.data[1] = (response.cellVoltages[17]/100);
//           canMsg.data[2] = (response.cellVoltages[18]/100);
//           canMsg.data[3] = (response.cellVoltages[19]/100);
//           canMsg.data[4] = response.batteryMac[0];
//           canMsg.data[5] = response.batteryMac[1];
//           canMsg.data[6] = response.batteryMac[2];
//           canMsg.data[7] = response.batteryMac[3];
      
//       mcp2515.sendMessage(&canMsg);  //Sends the CAN message
//       for (int i = 0; i < 8; i++) {
//        Serial.print(canMsg.data[i], HEX);
    
//       }
//       Serial.println();
//       Serial.println("===============");
//       //delay(1000);
//     //}
      



//       Serial.println("Received Request ID 04 from Tracker");
//       canMsg.can_id = 0x40;   //CAN id as 0x036
//       canMsg.can_dlc = 8;     //CAN data length as 8
     
//       canMsg.data[0] = response.remaining_capacity_ah[0];   
//       canMsg.data[1] = response.remaining_capacity_ah[1];   
//       canMsg.data[2] = response.rated_capacity;  //design capacity
//       canMsg.data[3] = response.rated_capacity; //full_capacity;  
//       canMsg.data[4] = 0x00;  
//       canMsg.data[5] = 0x00; 
//       canMsg.data[6] = 0x00; 
//       canMsg.data[7] = 0x00;  
//       mcp2515.sendMessage(&canMsg);  //Sends the CAN message 
//       Serial.println();
//       Serial.println("===============");
      
//       }     //can  req 73 loop ends 
//   }
}
