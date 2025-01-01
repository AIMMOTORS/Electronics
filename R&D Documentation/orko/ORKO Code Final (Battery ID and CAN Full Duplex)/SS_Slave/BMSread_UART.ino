byte receive_data[200];
uint8_t zerotxB[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

int16_t rated_capacity = 2950;  //design capacity 28800 29.5Ah
//int16_t rated_capacity = 440;  //design capacity 28800
//int16_t rated_capacity = 400;  //design capacity 28800
int16_t Rem_capacity;
int16_t Rem_capacity_Ah;
int Battery_SOH;


bool CMOS_readstate;
bool DMOS_readstate;

void setupUart() {
  Serialx.begin(9600);
}

/**
 * @brief Creating ID
 * @param dataID: data IDs from 0x90-0x98 
 * @param BMS address: 0x08
 * @param PC address: 0x40
 */
uint32_t makeCommID(uint8_t dataID, uint8_t BMS_addr, uint8_t PC_addr) {
  uint32_t Commid = 0xA5;
  Commid = (Commid << 8) | PC_addr;     //0x40
  Commid = (Commid << 8) | dataID;
  Commid = (Commid << 8) | BMS_addr;    //0x08
  return Commid;
}

uint8_t rxB[8];
uint8_t rxAddress[4];

/**
 * @brief Receives data via UART
 */
int getUartData(uint8_t dataId) {
  size_t x;
  int dataSize = 13;
  if (dataId == 0x95) {
    dataSize = 91;
  }
  else if (dataId == 0x96) {
    dataSize = 26;
  }
  if (Serialx.available())
  {
    //x = Serialx.readBytesUntil('\n', receive_data, 13);
    x = Serialx.readBytes(receive_data, dataSize);
    Serial.print("received bytes: ");
    Serial.print(x);
    Serial.print(" for address =");
    Serial.print(dataId,HEX);
    Serial.print(" : ");
    if (Serialx.available()) {
      Serial.print("Regular data:");
      for (int i = 0; i < dataSize; i++) {
        Serial.print(receive_data[i], HEX);
        Serial.print(",");
      }
      //  Serial.print(",Extra data:");
      //   while (Serialx.available()) {Serial.print(Serialx.read(),HEX); Serial.print(",");}
      //   Serial.println();
    }
    if (x == 13) {
      for (int i = 0; i < 4; i++) {
        rxAddress[i] = receive_data[i];
      }
      for (int i = 0; i < 8; i++) {
        rxB[i] = receive_data[i + 4];
      }
    }
    if (x == dataSize) {
      if (receive_data[2] == dataId) {
        return 1; //there is some error while executing the program, and it is not performing what it was intended to do.
      }
      else {
        return -2;
      }
    }
    while (Serialx.available()) {
      Serialx.read();
    }
    return 0;
  }
  else {
    Serial.println("No Data Found:");
    Serial.println(dataId, HEX);
    return -1;
  }

}

/**
 * @brief Sends IDs to request data from DALY BMS via UART
 */
void sendUartData(uint8_t dataId, uint8_t txB[8]) {
  uint8_t bufferbyte;
  uint8_t CRC=0;
  //delay(500);
   while (Serialx.available()) {
    Serialx.read();
  }
  uint32_t address = makeCommID(dataId, 0x08, 0x40);
  for (int i = 3; i >= 0; i--)
  {
    bufferbyte = (address >> (i * 8)) & 0xFF;
    CRC+=bufferbyte;
    Serialx.write(bufferbyte);
  }
  for (uint8_t i = 0; i < 8; i++)
  {
    bufferbyte = txB[i];
    CRC+=bufferbyte;
    Serialx.write(bufferbyte);
  }
  Serialx.write(CRC);
  delay(100);
}

//Wrapper function to be called
//Gives Voltage, Current and SOC
void printDebug(String message) {
  if (DEBUGLVL > 0) {
    Serial.println( message);
  }
}

/**
 * @brief This function is used for getting and decoding data from id 0x90
 * Returned data: Voltage, Current and SoC
 */
void getbmsData90() {
  //bmsData90 mybms;
  sendUartData(0x90, zerotxB); // Sending command to request data
  int recStat = getUartData(0x90); // Getting data response
  mybms90.readSuccess90 = (recStat == 1); // Checking if data reception was successful

  if (mybms90.readSuccess90) {
    // Extract data
    mybms90.Voltage = float(uint16_t((rxB[0] << 8) | rxB[1]))* 0.10;     //* 0.10;
    response.voltage[0] = highByte(mybms90.Voltage);
    response.voltage[1] = lowByte(mybms90.Voltage);
    mybms90.Current = float(uint16_t((rxB[4] << 8) | rxB[5])); //- 30000) // * 0.1;
    response.current[0] = highByte(mybms90.Current);
    response.current[1] = lowByte(mybms90.Current);
    mybms90.SOC = float(uint16_t((rxB[6] << 8) | rxB[7])); //* 0.1;
    response.soc[0] = highByte(mybms90.SOC);
    response.soc[1] = lowByte(mybms90.SOC);
    Serial.print(" Voltage from BMS: "); Serial.print(response.voltage[0],HEX); Serial.println(response.voltage[1],HEX); //Serial.print(mybms90.Voltage);
    Serial.print(" Current: "); Serial.print(mybms90.Current);
    Serial.print(" SoC:");  Serial.println(mybms90.SOC);
    printDebug("CAN 90: V=" + String(mybms90.Voltage) + "V, I=" + String(mybms90.Current) +
               "A, SOC=" + String(mybms90.SOC));
  } else {
    printDebug("Error in CAN data 90");
  }
}

/**
 * @brief This function is used for getting and decoding data from id 0x91.
 * Returned data: max cell voltage, min cell voltage, no. of cell with max voltage and no. of cell with min voltage
 */
void getbmsData91() {
  sendUartData(0x91, zerotxB); // Sending command to request data
  int recStat = getUartData(0x91); // Getting data response

  mybms91.readSuccess91 = (recStat == 1); // Checking if data reception was successful

  if (mybms91.readSuccess91) {
    mybms91.max_cell_mv = uint16_t((rxB[0] << 8) | rxB[1]);
    mybms91.no_max_cell_v = rxB[2];
    mybms91.min_cell_mv = uint16_t((rxB[3] << 8) | rxB[4]);
    mybms91.no_min_cell_v = rxB[5];
    printDebug("CAN 91: Max Cell MV=" + String(mybms91.max_cell_mv) +
               ", No Max Cell V=" + String(mybms91.no_max_cell_v) +
               ", Min Cell MV=" + String(mybms91.min_cell_mv) +
               ", No Min Cell V=" + String(mybms91.no_min_cell_v));
  } else {
    printDebug("Error in CAN data 91");
  }
}

/**
 * @brief This function is used for getting and decoding data from id 0x92.
 * Returned data: max temp, min temp, cell no. with max temp and cell no. with min temp
 */
void getbmsData92() {
  sendUartData(0x92, zerotxB); // Sending command to request data
  int recStat = getUartData(0x92); // Getting data response

  mybms92.readSuccess92 = (recStat == 1); // Checking if data reception was successful

  if (mybms92.readSuccess92) {
    // Extract data
    response.temperature = rxB[0] - 40;
    mybms92.max_temp_cell_no = rxB[1];
    mybms92.min_temp = rxB[2] - 40;
    mybms92.min_temp_cell_no = rxB[3];
    printDebug("CAN 92: Max Temp=" + String(mybms92.max_temp) + "°C, Max Temp Cell No=" +
               String(mybms92.max_temp_cell_no) + ", Min Temp=" + String(mybms92.min_temp) +
               "°C, Min Temp Cell No=" + String(mybms92.min_temp_cell_no));
    Serial.print("Temp: "); Serial.println(response.temperature);
  }  
   else {
    printDebug("Error in CAN data 92");
  }
}

/**
 * @brief This function is used for getting and decoding data from id 0x93.
 * Returned data: Charge MOS State, Discharge MOS State, BMS Life and Remaining Capacity 
 */
void getbmsData93() {
  sendUartData(0x93, zerotxB); // Sending command to request data
  int recStat = getUartData(0x93); // Getting data response

  mybms93.readSuccess93 = (recStat == 1); // Checking if data reception was successful

  if (mybms93.readSuccess93) {
    // Extract data
    mybms93.state = rxB[0];
    mybms93.charge_mos_state = rxB[1];
    mybms93.discharge_mos_state = rxB[2];
    mybms93.BMS_life = rxB[3];
    mybms93.remaining_capacity_mah = (rxB[4] << 24) | (rxB[5] << 16) | (rxB[6] << 8) | (rxB[7]);
    printDebug("CAN 93: State=" + String(mybms93.state) +
               ", Charge MOS State=" + String(mybms93.charge_mos_state) +
               ", Discharge MOS State=" + String(mybms93.discharge_mos_state) +
               ", BMS Life=" + String(mybms93.BMS_life) +
               ", Remaining Capacity=" + String(mybms93.remaining_capacity_mah));
                 response.remaining_capacity_mah = (rxB[4] << 24) | (rxB[5] << 16) | (rxB[6] << 8) | (rxB[7]);
                 //response.remaining_capacity_ah = (response.remaining_capacity_mah, HEX); //in Ah
                 response.remaining_capacity_ah[0] = highByte(response.remaining_capacity_mah);
                 response.remaining_capacity_ah[1] = lowByte(response.remaining_capacity_mah);
                  Serial.println(response.remaining_capacity_mah);
                  //Serial.print("Battery Capacity in Ah: "); Serial.println(response.remaining_capacity_ah);
                  Rem_capacity = response.remaining_capacity_mah;
                  //  response.Battery_SOH = ((R_capacity/rated_capacity));
                  response.Battery_SOH = Rem_capacity / rated_capacity;
                  response.rated_capacity[0] = highByte(rated_capacity);     // design cap
                  response.rated_capacity[1] = lowByte(rated_capacity);
                  Serial.println(response.Battery_SOH);
                  Serial.println(Rem_capacity);
                  Serial.print("Rated Capacity: "); Serial.print(response.rated_capacity[0], HEX); Serial.println(response.rated_capacity[1], HEX);
                  Serial.print("Remaining Capacity: "); Serial.print(response.remaining_capacity_ah[0], HEX); Serial.println(response.remaining_capacity_ah[1], HEX);

  } else {
    printDebug("Error in CAN data 93");
  }
}

/**
 * @brief This function is used for getting and decoding data from id 0x94
 * Returned data: battery string, charger status, load status and DIO state
 */
bmsData94 getbmsData94() {
  bmsData94 mybms;
  sendUartData(0x94, zerotxB); // Sending command to request data
  int recStat = getUartData(0x94); // Getting data response

  mybms.readSuccess94 = (recStat == 1); // Checking if data reception was successful

  if (mybms.readSuccess94) {
    // Extract data
    mybms.no_battery_string = rxB[0];
    mybms.no_temperature = rxB[1];
    mybms.charger_status = rxB[2];
    mybms.load_status = rxB[3];
    mybms.DIO_state = rxB[4];
    printDebug("CAN 94: No Battery String=" + String(mybms.no_battery_string) +
               ", No Temperature=" + String(mybms.no_temperature) +
               ", Charger Status=" + String(mybms.charger_status) +
               ", Load Status=" + String(mybms.load_status) +
               ", DIO State=" + String(mybms.DIO_state));
  } else {
    printDebug("Error in CAN data 94");
  }
  return mybms;
}

/**
 * @brief This function is used for getting and decoding data from id 0x95
 * Returned data: individual cell voltages
 */
void getbmsData95() {
  sendUartData(0x95, zerotxB); // Sending command to request data
  int recStat = getUartData(0x95); // Getting data response

  mybms95.readSuccess95 = (recStat == 1); // Checking if data reception was successful
  printDebug("Read Success 95=" + String(mybms95.readSuccess95));
  if (mybms95.readSuccess95) {
    for (int j = 0; j < 7; j++) {
      response.cellVoltages[j * 3] = receive_data[j * 13 + 5] << 8 | receive_data[j * 13 + 6];
      response.cellVoltages[j * 3 + 1] = receive_data[j * 13 + 7] << 8 | receive_data[j * 13 + 8];
      if (j != 6)
        response.cellVoltages[j * 3 + 2] = receive_data[j * 13 + 9] << 8 | receive_data[j * 13 + 10];

      printDebug("CAN 95: Cell" + String(j * 3 + 1) + " V = " + String(response.cellVoltages[j * 3]) +
                 ", Cell " + String(j * 3 + 2) + " V = " + String(response.cellVoltages[j * 3 + 1]) +
                 (j == 6) ? "" : ", Cell " + String(j * 3 + 3) + " V = " + String(response.cellVoltages[j * 3] + 2) + "\n");
    }
    for (int j = 0; j < 20; j++) {
      Serial.println(response.cellVoltages[j]); 
    }
  }
  else {
    for (int j = 0; j < 20; j++) {
      response.cellVoltages[j] = 0;
    }
    printDebug("Error in CAN data 95:" + String(recStat));
  }
  // return mybms;
 
}

/**
 * @brief This function is used for getting and decoding data from id 0x96
 * Returned data: cell temperature
 */
void getbmsData96() {
  delay(500);
  sendUartData(0x96, zerotxB); // Sending command to request data
  delay(500);
  int recStat = getUartData(0x96); // Getting data response

  mybms96.readSuccess96 = (recStat == 1); // Checking if data reception was successful

  if (mybms96.readSuccess96) {
    // Extract data
    mybms96.cell_t_frame_no = receive_data[4];
    mybms96.cell_temp = receive_data[5]-40;
    printDebug("CAN 96: Cell T Frame No=" + String(mybms96.cell_t_frame_no) +
               ", Cell Temp =" + String(mybms96.cell_temp));
  } else {
    printDebug("Error in CAN data 96");
  }
  //return mybms;
}

bmsData97 getbmsData97() {
  bmsData97 mybms;
  sendUartData(0x97, zerotxB); // Sending command to request data
  int recStat = getUartData(0x97); // Getting data response

  mybms.readSuccess97 = (recStat == 1); // Checking if data reception was successful

  if (mybms.readSuccess97) {
    // Extract data
    mybms.cell_bal_state = rxB[0] << 48 | rxB[1] << 40 | rxB[2] << 32 | rxB[3] << 24 | rxB[4] << 16 | rxB[5] << 8 | rxB[6];
    printDebug("CAN 97: Cell Bal State=" + String(uint32_t(mybms.cell_bal_state >> 32)) + String(uint32_t(mybms.cell_bal_state & 0xFFFFFFFF)));
//Serial.print("CAN 97 data");
//for(int i=0;i<8;i++){
//Serial.print(rxB[i],BIN);
//Serial.print(",");}
//Serial.println();
  } else {
    printDebug("Error in CAN data 97");
  }

  return mybms;
}

bmsData98 getbmsData98() {
  bmsData98 mybms;
  sendUartData(0x98, zerotxB); // Sending command to request data
  int recStat = getUartData(0x98); // Getting data response

  mybms.readSuccess98 = (recStat == 1); // Checking if data reception was successful

  if (mybms.readSuccess98) {
    // Extract data
    mybms.bat_fail_status = rxB[0] << 54 | rxB[1] << 48 | rxB[2] << 40 | rxB[3] << 32 | rxB[4] << 24 | rxB[5] << 16 | rxB[6] << 8 | rxB[7];
    printDebug("CAN 98: Bat Fail Status=" + String(uint32_t(mybms.bat_fail_status >> 32)) + String(uint32_t(mybms.bat_fail_status & 0xFFFFFFFF)));
  } else {
    printDebug("Error in CAN data 98");
  }

  return mybms;
}

void CMOS_state(bool mstate){
  byte CMOStxB[] = {0x01&mstate, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  sendUartData(0xDA, CMOStxB); // Sending command to request data
  int recStat = getUartData(0xDA); // Getting data response
  Serial.print("CMOS Data:");
   for (int i = 0; i < 13; i++) {
        Serial.print(receive_data[i], HEX);
        Serial.print(",");
      }
      Serial.println();
  }

void DMOS_state(bool mstate){
  byte DMOStxB[] =  {0x01&mstate, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  sendUartData(0xD9,DMOStxB); // Sending command to request data
  int recStat = getUartData(0xD9); // Getting data response
  Serial.print("DMOS Data:");
   for (int i = 0; i < 13; i++) {
        Serial.print(receive_data[i], HEX);
        Serial.print(",");
      }
      Serial.println();
  }
