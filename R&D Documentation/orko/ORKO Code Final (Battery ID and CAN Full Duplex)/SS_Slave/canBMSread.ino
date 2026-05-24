// /*
//   Author: Muhammad Hashir Bin Khalid
//   Date: 3-8-2023
//   This file contains functions to read data DALY BMS via CAN interface.
// */


// MCP_CAN CAN0(15);  // Set CS to digital pin D8

// uint8_t zerotxB[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// //Setup the CAN configurations
// bool setup_CAN() {
//   if (CAN0.begin(MCP_ANY, CAN_250KBPS, MCP_8MHZ) == CAN_OK) {
//     Serial.println("MCP2515 Initialized Successfully!");
//     canInitialized = 1;
//   } else {
//     Serial.println("Error Initializing MCP2515...");
//     canInitialized = 0;
//   }
//   CAN0.setMode(MCP_NORMAL);  // Change to normal mode to allow messages to be transmitted
//   delay(1000);
//   return canInitialized;
// }

// //Auxiliary Functions
// //Make a CAN id from data ID
// uint32_t makeCANID(uint8_t dataID, uint8_t BMS_addr, uint8_t PC_addr) {
//   uint32_t CANid = 0x18;
//   CANid = (CANid << 8) | dataID;
//   CANid = (CANid << 8) | BMS_addr;  //0x01
//   CANid = (CANid << 8) | PC_addr;   //0x40
//   return CANid;
// }

// //Sends data request to BMS
// uint8_t send_CAN(uint8_t dataId, uint8_t txB[8]) {
//   uint32_t canid = makeCANID(dataId, 0x01, 0x40);
//   uint8_t sndStat = CAN0.sendMsgBuf(canid, 1, 8, txB);
//   delay(2000);
//   return sndStat;
// }

// uint8_t rxB[8];
// //Get data from BMS
// byte get_CAN(uint8_t dataId) {
//   long unsigned int rxId;
//   uint8_t len = 0;
//   byte recStat;
//   //Read Response
//   recStat = CAN0.readMsgBuf(&rxId, &len, rxB);  // Read data: len = data length, buf = data byte(s)

//   Serial.print("Error:");
//   Serial.println(CAN0.checkError());
//   if(CAN0.checkError()==5){
//     if(setup_CAN()){Serial.println("can re init passed");
//       recStat = CAN0.readMsgBuf(&rxId, &len, rxB);  // Read data: len = data length, buf = data byte(s)
// }
//     Serial.println("Got Error");
//   }

//   return recStat;
// }


// //Wrapper function to be called
// //Gives Voltage, Current and SOC
// void update_Basic_BMS_Data() {
//   send_CAN(0x90, zerotxB);
//   byte recStat = get_CAN(0x90);
//   if (recStat == CAN_OK && CAN0.checkError()!=5) {
//     Serial.print("CAN ");
//     Serial.print(0x90, HEX);
//     Serial.print(": V=");
//     Serial.print(response.voltage);
//     Serial.print("V, I=");
//     Serial.print(response.current);
//     Serial.print("A, SOC=");
//     Serial.println(response.soc);
//     response.voltage = uint16_t((rxB[0] << 8) | rxB[1]);
//     response.current = uint16_t((rxB[4] << 8) | rxB[5]);
//     response.soc = uint16_t((rxB[6] << 8) | rxB[7]);
//     response.flags |= 0x07;
//   } else {
//     response.flags &= 0xF8;
//     response.flags |= 0x01;
//     Serial.println("Error in CAN data 90");
//   }

//   delay(1000);

//   send_CAN(0x92, zerotxB);
//   recStat = get_CAN(0x92);
//   if (recStat == CAN_OK && CAN0.checkError()!=5) {
//     response.temperature = rxB[0];  // - 40;   //Max Temperature
//   }
// }

// void updateBatteryMacAndBPI() {
//   send_CAN(BATTERY_MAC_ADDRESS, zerotxB);
//   byte recStat = get_CAN(BATTERY_MAC_ADDRESS);
//   if (recStat == CAN_OK && CAN0.checkError()!=5) {
//     response.batteryMac[0] = rxB[0];
//     response.batteryMac[1] = rxB[1];
//     response.batteryMac[2] = rxB[2];
//     response.batteryMac[3] = rxB[3];
//     response.batteryMac[4] = rxB[4];
//     response.batteryMac[5] = rxB[5];
//     response.BPI = rxB[6];
//     response.flags &= 0b11110111;
//     Serial.print("Mac Address fetched:");
//     printMACAddress(response.batteryMac);
//     Serial.print(", BPI:");
//     Serial.println(response.BPI);
//   } else {
//     Serial.println("Mac Address fetch failed");
//     response.flags |= 0b00001000;
//   }
// }
