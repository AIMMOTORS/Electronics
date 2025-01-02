/*
  Author: Muhammad Hashir Bin Khalid
  Date: 3-8-2023
  This file contains functions to read data DALY BMS via CAN interface.
*/


MCP_CAN CAN0(15);     // Set CS to digital pin D8

uint8_t zerotxB[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

//Setup the CAN configurations
void setup_CAN() {
  if (CAN0.begin(MCP_ANY, CAN_250KBPS, MCP_8MHZ) == CAN_OK) {
    Serial.println("MCP2515 Initialized Successfully!");
    canInitialized=1;
  }
  else {
    Serial.println("Error Initializing MCP2515...");
    canInitialized=0;
  }
  CAN0.setMode(MCP_NORMAL);   // Change to normal mode to allow messages to be transmitted
  delay(1000);
}

//Auxiliary Functions
//Make a CAN id from data ID
uint32_t makeCANID(uint8_t dataID, uint8_t BMS_addr, uint8_t PC_addr) {
  uint32_t CANid = 0x18;
  CANid = (CANid << 8) | dataID;
  CANid = (CANid << 8) | BMS_addr;    //0x01
  CANid = (CANid << 8) | PC_addr;     //0x40
  return CANid;
}

//Sends data request to BMS
uint8_t send_CAN(uint8_t dataId, uint8_t txB[8]) {
  uint32_t canid = makeCANID(dataId, 0x01, 0x40);
  uint8_t sndStat = CAN0.sendMsgBuf(canid, 1, 8, txB);
  delay(2000);
  return sndStat;
}

uint8_t rxB[8];
//Get data from BMS
byte get_CAN(uint8_t dataId) {
  long unsigned int rxId;
  uint8_t len = 0;
  byte recStat;
  //Read Response
  recStat = CAN0.readMsgBuf(&rxId, &len, rxB);      // Read data: len = data length, buf = data byte(s)
  if (DEBUGLVL > 0) {
    Serial.print("Error:");
    Serial.println(CAN0.checkError());
  }
  return recStat;
}


//Wrapper function to be called
//Gives Voltage, Current and SOC
bmsbasic get_Basic_BMS_Data() {
  bmsbasic mybms;
  send_CAN(0x90, zerotxB);
  byte recStat = get_CAN(0x90);
  mybms.Voltage = float(uint16_t((rxB[0] << 8) | rxB[1])) * 0.10;
  mybms.Current = float(uint16_t((rxB[4] << 8) | rxB[5]) - 30000) * 0.1;
  mybms.SOC = float(uint16_t((rxB[6] << 8) | rxB[7])) * 0.1;
  mybms.readSuccess = (recStat == CAN_OK);
  if (DEBUGLVL > 0) {
    if (mybms.readSuccess) {
      Serial.print("CAN ");
      Serial.print(0x90, HEX);
      Serial.print(": V=");
      Serial.print(mybms.Voltage);
      Serial.print("V, I=");
      Serial.print(mybms.Current);
      Serial.print("A, SOC=");
      Serial.println(mybms.SOC);
    }
    else {
      Serial.println("Error in CAN data 90");
    }
  }
  return mybms;
}

//Gives data from address 91 to 93
bmsData1 get_BMS_Data1() {
  bmsData1 s;
 /* send_CAN(0x91, zerotxB);
  byte recStat = get_CAN(0x91);
  s.max_cell_mv = uint16_t((rxB[0] << 8) | rxB[1]);
  s.no_max_cell_v = rxB[2];
  s.min_cell_mv = uint16_t((rxB[3] << 8) | rxB[4]);
  s.no_min_cell_v = rxB[5];
  s.readSuccess91 = (CAN0.getError() == 0);
  if (DEBUGLVL > 0) {
    if (s.readSuccess91) {
      Serial.print("CAN ");
      Serial.print(0x91, HEX);
      Serial.print(": max_cell_mv=");
      Serial.print(s.max_cell_mv);
      Serial.print("mV at Cell No.=");
      Serial.print(s.no_max_cell_v);
      Serial.print(", min_cell_mv=");
      Serial.print(s.min_cell_mv);
      Serial.print("mV at Cell No.=");
      Serial.println(s.no_min_cell_v);
    }
    else {
      Serial.println("Error in CAN data 91");
    }
  }

  delay(1000); */

  send_CAN(0x92, zerotxB);
  
  byte recStat = get_CAN(0x92);
  s.max_temp = rxB[0] - 40;
  s.max_temp_cell_no = rxB[1];
  s.min_temp = rxB[2] - 40;
  s.min_temp_cell_no = rxB[3];
  s.readSuccess92 = (CAN0.getError() == 0);
  if (DEBUGLVL > 0) {
    if (s.readSuccess92) {
      Serial.print("CAN ");
      Serial.print(0x92, HEX);
      Serial.print(": max_Temp=");
      Serial.println(s.max_temp);
      //Serial.print("C at Cell No.=");
     // Serial.print(s.max_temp_cell_no);
      //Serial.print(", min_Temp=");
     // Serial.print(s.min_temp);
      //Serial.print("C at Cell No.=");
      //Serial.println(s.min_temp_cell_no);
    }
    else {
      Serial.println("Error in CAN data 92");
    }
  }
  delay(1000);

  /*send_CAN(0x93, zerotxB);
  recStat = get_CAN(0x93);
  s.state = rxB[0];
  s.charge_mos_state = rxB[1];
  s.discharge_mos_state = rxB[2];
  s.BMS_life = rxB[3];
  s.remaining_capacity_mah = (rxB[4] << 24) | (rxB[5] << 16) | (rxB[6] << 8) | (rxB[7]);
  s.readSuccess93 = (CAN0.getError() == 0);
  if (DEBUGLVL > 0) {
    if (s.readSuccess93) {
      Serial.print("CAN ");
      Serial.print(0x93, HEX);
      Serial.print(": State=");
      Serial.print(s.state);
      Serial.print(", charging MOS state=");
      Serial.print(s.charge_mos_state);
      Serial.print(", Discharging MOS state=");
      Serial.print(s.discharge_mos_state);
      Serial.print(", BMS Life=");
      Serial.print(s.BMS_life);
      Serial.print(", Remaining mAh=");
      Serial.println(s.remaining_capacity_mah);
    }
    else {
      Serial.println("Error in CAN data 93");
    }
  } */

  return s;
}

//Gives data from address 94 to 98 excluding 95
//Debug serial printing not added
bmsData2 get_BMS_Data2() {
  bmsData2 s;
  byte recStat;
  send_CAN(0x94, zerotxB);
  recStat = get_CAN(0x94);
  s.no_battery_string = rxB[0];
  s.no_temperature = rxB[1];
  s.charger_status = rxB[2];
  s.load_status = rxB[3];
  s.DIO_state = rxB[4];

  send_CAN(0x96, zerotxB);
  recStat = get_CAN(0x96);
  s.cell_t_frame_no = rxB[0];
  s.cell_temp = rxB[1] << 48 | rxB[2] << 40 | rxB[3] << 32 | rxB[4] << 24 | rxB[5] << 16 | rxB[6] << 8 | rxB[7];
  delay(100);

  send_CAN(0x97, zerotxB);
  recStat = get_CAN(0x97);
  s.cell_bal_state = rxB[0] << 48 | rxB[1] << 40 | rxB[2] << 32 | rxB[3] << 24 | rxB[4] << 16 | rxB[5] << 8 | rxB[6];
  delay(100);

  send_CAN(0x98, zerotxB);
  recStat = get_CAN(0x98);
  s.bat_fail_status = rxB[0] << 54 | rxB[1] << 48 | rxB[2] << 40 | rxB[3] << 32 | rxB[4] << 24 | rxB[5] << 16 | rxB[6] << 8 | rxB[7];
  return s;
}

//Changes state of Charging MOSFET
void set_chg_FET_state(bool state) { //CHG=0 for discharging, CHG=1 f
  uint8_t txB[8] =  {0x01 & state, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  send_CAN(0xDA, txB);
  byte recStat = get_CAN(0xDA);
  if (DEBUGLVL > 0) {
    Serial.print("Charging MOSFET state switched to: ");
    Serial.println(rxB[0]);
  }
}

//Changes state of discharging MOSFET
void set_dischg_FET_state(bool state) { //CHG=0 for discharging, CHG=1 f
  uint8_t txB[8] =  {0x01 & state, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  send_CAN(0xD9, txB);
  byte recStat = get_CAN(0xD9);
  if (DEBUGLVL > 0) {
    Serial.print("Discharging MOSFET state switched to: ");
    Serial.println(rxB[0]);
  }
}

/*Untested

  uint8_t * get_Bat_id(){
  //uint8_t id[6];
  //WiFi.macAddress(id);
  send_CAN(0xD9, zerotxB);
  uint8_t *id = get_CAN(0x99);
  return id;
  }
*/
/*void setup() {
  setup_CAN();
  }

  bmsbasic t;
  bmsData1 s;
  bmsData2 u;
  void loop()
  {
  t = get_Basic_BMS_Data();
  s = get_BMS_Data1();
  u = get_BMS_Data2();
  delay(1000);
  set_chg_FET_state(1);
  set_dischg_FET_state(1);
  delay(1000);
  set_chg_FET_state(0);
  set_dischg_FET_state(0);
  uint8_t *bat_Id = get_Bat_id();  //Get Battery id
  }*/;
