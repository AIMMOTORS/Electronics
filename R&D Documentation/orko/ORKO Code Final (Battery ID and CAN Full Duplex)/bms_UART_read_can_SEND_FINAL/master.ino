
// #define SS_FORMAT 0  //Set to 1 = Serial data in Swap Station readable format, 0 = format Serial data in ASCII human readable format

//Slave 1=34:94:54:95:A0:7F
//Slave 2=34:94:54:8E:16:97
//Slave 3=CC:50:E3:6D:77:BA
//Slave 4=08:3A:8D:E3:D4:EB
//Slave 5=BC:FF:4D:55:D6:1F
//Slave 6=34:94:54:8E:2A:A7
//Slave 7=34:94:54:95:9E:E0
//Slave 8=34:94:54:8E:14:EA
//Slave 9=08:3A:8D:E3:CF:61

// REPLACE WITH THE MAC Address of your receiver
// uint8_t nodeAddress[1][6] = { { 0x08, 0xF9, 0xE0, 0x7E, 0x6D, 0xE4 },    //08:f9:e0:7e:6d:e4  slave
//                               };
// MAC address of bike 2 slave
 /*uint8_t nodeAddress[2][6] = { { 0xD8, 0xBF, 0xC0, 0xE3, 0x06, 0xF7 },    //d8:bf:c0:e3:06:f7 slave for 2nd bike
                               { 0x50, 0x02, 0x91, 0x75, 0x9D, 0xD6},    //50:02:91:75:9d:d6
                              };                             
*/
 uint8_t nodeAddress[2][6] = { { 0x08, 0xF9, 0xE0, 0x7E, 0x6D, 0xE4 },    //{ 0x08, 0xF9, 0xE0, 0x7E, 0x6D, 0xE4 }, //SLAVE FOR 2.8
                               { 0x08, 0x3A, 0x8D, 0xE3, 0xCB, 0xCE },
                              };                             

// Updates DHT readings every 10 seconds
byte command, datalength, s;
const long interval = 20000;
unsigned long previousMillis = 0;  // will store last time DHT was updated

typedef struct struct_request {
  bool d;
} struct_request;

// typedef struct struct_response {
//   unsigned int voltage;
//   unsigned int current;
//   unsigned int soc;
//   byte temperature;
//   byte batteryMac[6];
//   byte BPI;
//   byte slot_id;
//   byte flags;
//   unsigned long time;
// } struct_response;

// // Create a struct_message called DHTReadings to hold sensor readings
// struct_response response;

// Create a struct_message to hold incoming sensor readings
struct_request request;

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  if (!SS_FORMAT) {
    Serial.print("Packet Sent to ");
    printMACAddress(mac_addr);
    Serial.print(",Status: ");
    if (sendStatus == 0) {
      Serial.println("Delivery success");
    } else {
      Serial.println("Delivery fail");
    }
  }
}

// Callback when data is received
void OnDataRecv(uint8_t *mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&response, incomingData, sizeof(response));
  if (!SS_FORMAT) {
    Serial.print("Bytes received: ");
    Serial.print(len);
    Serial.print(" from Slave: ");
    printMACAddress(mac);
  }
  //sendtoSerial();
}

void sendtoSerial() {
  byte datatoSend[19];
  if (SS_FORMAT) {
    datatoSend[0] = 0x80 | 0x01;
    datatoSend[1] = 0x10;
    datatoSend[2] = response.voltage[0];
    datatoSend[3] = response.voltage[1];
    datatoSend[4] = response.current[0];
    datatoSend[5] = response.current[1];
    datatoSend[6] = response.soc[0];
    datatoSend[7] = response.soc[1];
    datatoSend[8] = response.temperature;
    for (int j = 0; j <= 5; j++) {
      datatoSend[9 + j] = response.batteryMac[j];
    }
    datatoSend[15] = response.BPI;
    datatoSend[16] = response.slot_id;
    datatoSend[17] = response.flags;

    //calculating XOR
    datatoSend[18] = datatoSend[0];
    for (int j = 1; j <= 17; j++) {
      datatoSend[18] = datatoSend[18] ^ datatoSend[j];
    }

    //Sending complete data
    for (int j = 0; j <= 18; j++) {
      Serial.write(datatoSend[j]);
    }
    // Serial.write(0x80 | 0x01);
    // Serial.write(0x10);
    // Serial.write(highByte(response.voltage));
    // Serial.write(lowByte(response.voltage));
    // Serial.write(highByte(response.current));
    // Serial.write(lowByte(response.current));
    // Serial.write(highByte(response.soc));
    // Serial.write(lowByte(response.soc));
    // Serial.write(response.temperature);
    // for (int j = 0; j < 6; j++) {
    //   Serial.write(response.batteryMac[j]);
    // }
    // Serial.write(response.BPI);
    // //Serial.write(s + 1);
    // Serial.write(response.slot_id);
    // Serial.write(response.flags);
    // Serial.write(0x81 ^ 0x10 ^ highByte(response.voltage) ^ lowByte(response.voltage) ^ highByte(response.current) ^ lowByte(response.current) ^ highByte(response.soc) ^ lowByte(response.soc) ^ response.temperature ^ response.batteryMac[0] ^ response.batteryMac[1] ^ response.batteryMac[2] ^ response.batteryMac[3] ^ response.batteryMac[4] ^ response.batteryMac[5] ^ response.BPI ^ response.slot_id ^ response.flags);
  } else {
    Serial.print(", Data response: Slot id=");
    Serial.print(response.slot_id);
    Serial.print(", time= ");
    Serial.print(response.time);
    Serial.print(", V= ");
    Serial.print(response.voltage[0]); Serial.print(response.voltage[1]);
    Serial.print(", I= ");
    Serial.print(response.current[0]); Serial.print(response.current[1]);
    Serial.print(", SOC= ");
    Serial.print(response.soc[0]); Serial.print(response.soc[1]);
    Serial.print(", Temp= ");
    Serial.print(response.temperature);
    Serial.print(", SOH= ");
    Serial.print(response.Battery_SOH);
    Serial.print(", Remaining capacity= ");
    Serial.print(response.remaining_capacity_mah);
    Serial.print(", Design capacity= ");
    Serial.print(response.rated_capacity[0]);Serial.print(response.rated_capacity[1]);
    Serial.print(", Full capacity= ");
    Serial.print(response.rated_capacity[0]);Serial.print(response.rated_capacity[1]);

    Serial.print(", Bat_Mac= ");
    Serial.print(response.batteryMac[0]);
    Serial.print(":");
    Serial.print(response.batteryMac[1]);
    Serial.print(":");
    Serial.print(response.batteryMac[2]);
    Serial.print(":");
    Serial.print(response.batteryMac[3]);
    Serial.print(":");
    Serial.print(response.batteryMac[4]);
    Serial.print(":");
    Serial.print(response.batteryMac[5]);
    Serial.print(":");
    Serial.print(response.batteryMac[6]);
    Serial.print(", BPI= ");
    Serial.print(response.BPI);
    Serial.print(", Flag= ");
    Serial.print(response.flags);
    Serial.println();
    for (int j = 0; j < 20; j++) {
      Serial.println(response.cellVoltages[j]); 
    }
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

void Master_setup() {
  // Init Serial Monitor
  // Serial.begin(9600);
  // delay(1000);
  if (!SS_FORMAT) {
    Serial.println();
    Serial.print("I am Master, my Mac address is: ");
    Serial.println(WiFi.macAddress());
  }
  // Init DHT sensor
  // dht.begin();

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Set ESP-NOW Role
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  for (int i = 0; i <= 2; i++) {
    esp_now_add_peer(nodeAddress[i], ESP_NOW_ROLE_COMBO, 1, NULL, 0);
    delay(100);
  }

  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
  request.d = 1;
}


void Master_loop() {

    canMsg.can_id = 0x32;
    canMsg.can_dlc = 8; 
    mcp2515.sendMessage(&canMsg);
    
    if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK)  // To receive data (Poll Read)
    {
       if (canMsg.can_id == 0x23)  
       {
          Serial.println("Received CAN ID: 0x23");
          response.voltage[0] = canMsg.data[0];
          response.voltage[1] = canMsg.data[1];
          response.temperature = canMsg.data[2];
          response.soc[0] = canMsg.data[3];
          response.soc[1] = canMsg.data[4];
          response.Battery_SOH = canMsg.data[6];
          //Serial.println("ID 0x23 received from Battery");
          Serial.print("Voltage: "); Serial.print(response.voltage[0],HEX); Serial.println(response.voltage[1],HEX);
          //Serial.print("Current: "); Serial.print(response.current[0],HEX); Serial.println(response.current[1],HEX);
          Serial.print("SoC: "); Serial.print(response.soc[0],HEX); Serial.println(response.soc[1],HEX);
          Serial.print("Temperature: "); Serial.println(response.temperature,HEX);
          Serial.print("Battery SoH: "); Serial.println(response.Battery_SOH,HEX);
       }

       if (canMsg.can_id == 0x43)  
       {
          Serial.println("Received CAN ID: 0x43");
          for (int a = 0; a < 8; a++) { 
            response.cellVoltages[a] = (canMsg.data[a]);
          }
          
          for (int i = 0; i < 8; i++) {
              Serial.print(canMsg.data[i], HEX);    
          }        
       }

       if (canMsg.can_id == 0x53)  
       {  
          Serial.println("Received CAN ID: 0x53");
          for (int a = 0; a < 8; a++) { 
            response.cellVoltages[a+8] = (canMsg.data[a]);
          }
          for (int i = 0; i < 8; i++) {
              Serial.print(canMsg.data[i], HEX);    
          }        
       }

       if (canMsg.can_id == 0x54)  
       { 
          Serial.println("Received CAN ID: 0x54");
          response.cellVoltages[16] = canMsg.data[0];
          response.cellVoltages[17] = canMsg.data[1];
          response.cellVoltages[18] = canMsg.data[2];
          response.cellVoltages[19] = canMsg.data[3];
          response.current[0] = canMsg.data[4];
          response.current[1] = canMsg.data[5];
          for (int i = 0; i < 6; i++) {
              Serial.print(canMsg.data[i], HEX);    
          }
          Serial.println(" ");
          // Serial.println("MAC: ");
          // for (int i = 4; i < 8; i++) {
          //     Serial.print(canMsg.data[i], HEX);    
          // }        
       }

       if (canMsg.can_id == 0x55)  
       { 
          Serial.println("Received CAN ID: 0x55");
          response.remaining_capacity_ah[0] = canMsg.data[0];
          response.remaining_capacity_ah[1] = canMsg.data[1];
          response.batteryMac[2] = canMsg.data[2]; 
          response.batteryMac[3] = canMsg.data[3]; 
          response.batteryMac[4] = canMsg.data[4]; 
          response.batteryMac[5] = canMsg.data[5];
          Serial.println(" ");
          Serial.println("MAC: ");
          for (int i = 2; i < 6; i++) {
              Serial.print(canMsg.data[i], HEX);    
          }

          Serial.print("Remaining Capacity: ");
          Serial.print(canMsg.data[0], HEX);  Serial.println(canMsg.data[1], HEX);     
                 
       }

       if (canMsg.can_id == 0x56)  
       { 
          Serial.println("Received CAN ID: 0x56");
          response.battery_id[0] = canMsg.data[0];
          response.battery_id[1] = canMsg.data[1];
          response.battery_id[2] = canMsg.data[2];
          response.battery_id[3] = canMsg.data[3];
          response.battery_id[4] = canMsg.data[4];
          Serial.println(" ");
          Serial.println("Batt ID: ");
          for (int i = 0; i < 5; i++) {
              Serial.print(canMsg.data[i], HEX);    
          }     
                 
       }




    //For Tracker
       Serial.print("CAN ID: ");
    Serial.println(canMsg.can_id, HEX);
    if (canMsg.can_id == 0x63)  // getting  00000063 req from trakker
    {
      Serial.println("Received Request ID 63 from Tracker");
      canMsg.can_id = 0x36;     //CAN id as 0x036
      canMsg.can_dlc = 8;       //CAN data length as 8
      canMsg.data[0] = response.voltage[0];  //Voltage [0] 
      canMsg.data[1] = response.voltage[1];  //Voltage [1] 
      canMsg.data[2] = response.temperature; //Temp  
      canMsg.data[3] = response.soc[0];  //SoC [0]
      canMsg.data[4] = response.soc[1];  //SoC [1]
      canMsg.data[5] = (SpeedH); //Speed  
      canMsg.data[6] = response.Battery_SOH; 
      canMsg.data[7] = 0x00;
      for (int i = 0; i < 8; i++) {
       Serial.print(canMsg.data[i], HEX);    
      }
      //Serial.println();
      Serial.println("===============");
      //uint8_t canMsg.data[8] = {rxB[0], rxB[1], rxB[4], rxB[5], rxB[6], rxB[7], 0x00, 0xF0 };


      mcp2515.sendMessage(&canMsg);  //Sends the CAN message
    }
    //==========================================================================

    if (canMsg.can_id == 0x73)  // getting  00000063 req from trakker
    {
      Serial.println("Received Request ID 73 from Tracker");
      canMsg.can_id = 0x37;   //CAN id as 0x036
      canMsg.can_dlc = 8;     //CAN data length as 8
      for (int a = 0; a < 8; a++) { 
         canMsg.data[a] = (response.cellVoltages[a]);
      }

      mcp2515.sendMessage(&canMsg);  //Sends the CAN message
      for (int i = 0; i < 8; i++) {
       Serial.print(canMsg.data[i], HEX);
    
      }
      Serial.println();
      Serial.println("===============");
      //delay(1000);
    //}     can  req 73 loop ends 
  

//  if (canMsg.can_id == 0x83)  // getting  00000063 req from trakker
//      {
      Serial.println("Received Request ID 83 from Tracker");
      canMsg.can_id = 0x38;   //CAN id as 0x036
      canMsg.can_dlc = 8;     //CAN data length as 8
     for (int a = 0; a < 8; a++) { 
      canMsg.data[a] = (response.cellVoltages[a+8]);
      }
      
      mcp2515.sendMessage(&canMsg);  //Sends the CAN message
      for (int i = 0; i < 8; i++) {
       Serial.print(canMsg.data[i], HEX);
    
      }
      Serial.println();
      Serial.println("===============");
      //delay(1000);
   // }
  

      Serial.println("Received Request ID 93 from Tracker");
      canMsg.can_id = 0x39;   //CAN id as 0x036
      canMsg.can_dlc = 8;     //CAN data length as 8

      canMsg.data[0] = (response.cellVoltages[16]);
      canMsg.data[1] = (response.cellVoltages[17]);
      canMsg.data[2] = (response.cellVoltages[18]);
      canMsg.data[3] = (response.cellVoltages[19]);
      canMsg.data[4] = response.current[0];
      canMsg.data[5] = response.current[1];
      canMsg.data[6] = 0x00;
      canMsg.data[7] = 0x00;
      
      mcp2515.sendMessage(&canMsg);  //Sends the CAN message
      for (int i = 0; i < 6; i++) {
       Serial.print(canMsg.data[i], HEX);    
      }
      Serial.println();
      Serial.println("===============");

      Serial.println("Received Request ID 04 from Tracker");
      canMsg.can_id = 0x40;   //CAN id as 0x036
      canMsg.can_dlc = 8;     //CAN data length as 8
     
      canMsg.data[0] = (response.remaining_capacity_ah[0],HEX);   
      canMsg.data[1] = response.remaining_capacity_ah[1];   
      canMsg.data[2] = design_capacity;
      canMsg.data[3] = full_capacity;  
      canMsg.data[4] = response.batteryMac[2];  
      canMsg.data[5] = response.batteryMac[3];
      canMsg.data[6] = response.batteryMac[4];
      canMsg.data[7] = response.batteryMac[5]; 
      mcp2515.sendMessage(&canMsg);  //Sends the CAN message 
      for (int i = 0; i < 8; i++) {
       Serial.print(canMsg.data[i], HEX);    
      }
      Serial.println();
      Serial.println("===============");

      Serial.println("Received Request ID 05 from Tracker");
      canMsg.can_id = 0x50;   //CAN id as 0x036
      canMsg.can_dlc = 8;     //CAN data length as 8
    
      canMsg.data[0] = response.battery_id[0];   
      canMsg.data[1] = response.battery_id[1];   
      canMsg.data[2] = response.battery_id[2]; //4
      canMsg.data[3] = response.battery_id[3]; //5 
      canMsg.data[4] = response.battery_id[4]; //9 
      canMsg.data[5] = 0x00; 
      canMsg.data[6] = 0x00;
      canMsg.data[7] = 0x00;
      mcp2515.sendMessage(&canMsg);  //Sends the CAN message 
      for (int i = 0; i < 5; i++) {
          Serial.print(canMsg.data[i], HEX);    
      }
      Serial.println();
      Serial.println("===============");
      
      }     //can  req 73 loop ends


      
    }
  
    
}
