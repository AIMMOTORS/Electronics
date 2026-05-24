void setupEEPROM()
{
  EEPROM.begin(512);
  delay(1000);
}


void writeIntoEEPROM(int address, uint32_t number)
{
  EEPROM.write(address, (number >> 24) & 0xFF);
  EEPROM.write(address + 1, (number >> 16) & 0xFF);
  EEPROM.write(address + 2, (number >> 8) & 0xFF);
  EEPROM.write(address + 3, number & 0xFF);
  EEPROM.commit();
}

uint32_t readFromEEPROM(int address)
{
  return (EEPROM.read(address) << 24) + (EEPROM.read(address + 1) << 16) + (EEPROM.read(address + 2) << 8) + EEPROM.read(address + 3);
}

void resetEEPROMData() {
  EEPROM.write(DEBUG_ADDRESS, 1);
  EEPROM.write(MOTOR_UART_ENABLE, 1);
  EEPROM.write(CAN_ENABLE, 1);
  EEPROM.write(APP_ENABLE, 1);
  EEPROM.commit();
  writeIntoEEPROM(NUM_TRIPS, 0);
  writeIntoEEPROM(EEPROM_DISTANCE, 0);
  writeIntoEEPROM(AVG_SPEED_THIS_T, 0);
 // writeIntoEEPROM(NEW_SPEED_T, 0);        //
  writeIntoEEPROM(CURRENT_SOC, 0);
   writeIntoEEPROM(VOLTAGE, 0); 
   writeIntoEEPROM(MAX_TEMP, 0);
  writeIntoEEPROM(THIS_ST_START_SOC, 0);
  writeIntoEEPROM(LAST_ST_START_SOC, 0);
  writeIntoEEPROM(LAST_ST_FINAL_SOC, 0);
  writeIntoEEPROM(DIST_START_ST, 0);
 // writeIntoEEPROM(KMS_THIS_ST, 0);
  writeIntoEEPROM(KMS_LAST_ST, 0);
  writeIntoEEPROM(NUM_SWAP_TRIPS, 0);
  delay(1000);
}


void loadEEPROMData() {
  s.num_trips = readFromEEPROM(NUM_TRIPS);
  s.eeprom_distance = readFromEEPROM(EEPROM_DISTANCE);
  s.distance = s.eeprom_distance;
  s.avg_speed_this_t = readFromEEPROM(AVG_SPEED_THIS_T);
  //s.new_speed_t = readFromEEPROM(NEW_SPEED_T);
  s.Voltage = readFromEEPROM(VOLTAGE);
  s.max_temp = readFromEEPROM(MAX_TEMP);
  //s.avg_speed = readFromEEPROM(AVG_SPEED);
  s.num_swap_trips = readFromEEPROM(NUM_SWAP_TRIPS);
  s.current_soc = readFromEEPROM(CURRENT_SOC) / 10;
  s.this_st_start_soc = readFromEEPROM(THIS_ST_START_SOC);
  s.last_st_start_soc = readFromEEPROM(LAST_ST_START_SOC);
  s.last_st_final_soc = readFromEEPROM(LAST_ST_FINAL_SOC);
  s.dist_start_st = readFromEEPROM(DIST_START_ST);
  s.kms_this_st = s.eeprom_distance - s.dist_start_st;
  s.kms_last_st = readFromEEPROM(KMS_LAST_ST);
  delay(1000);
}

void beginTrip() {
  s.num_trips += 1;
  writeIntoEEPROM(NUM_TRIPS, s.num_trips);
  s.num_speed_in_trip = 0;
  s.avg_speed_this_t = 0;
}

unsigned long int distUpdateMillis = 0;

void updateAtSpeedFetch(int bikeSpeed) {
  if (millis() > distUpdateMillis + 1000) {
   // s.avg_speed_this_t = (s.avg_speed_this_t*s.num_speed_in_trip + bikeSpeed) / (s.num_speed_in_trip + 1);      //avg speed  
     s.avg_speed_this_t = (s.avg_speed_this_t*s.num_speed_in_trip + bikeSpeed) / (s.num_speed_in_trip + 1); 
    s.num_speed_in_trip += 1;
    //s.new_speed_t =bikeSpeed;
    s.distance += uint32_t(bikeSpeed * ((millis() - distUpdateMillis) / (1000 * 60 * 60)));
    distUpdateMillis = millis();
    if (s.distance >= s.eeprom_distance + 1) {
      s.eeprom_distance = s.distance;
      s.kms_this_st = s.eeprom_distance - s.dist_start_st;
      writeIntoEEPROM(EEPROM_DISTANCE, s.eeprom_distance);
    }
  }
}

void checkSwap() {
  bmsbasic t = get_Basic_BMS_Data();
 // bmsData1 q =  get_BMS_Data1();
  if (t.SOC > s.current_soc + 10) {   //Swap done
    if (DEBUGLVL > 0) Serial.println("Swap Detected");
    s.num_swap_trips = s.num_swap_trips + 1;
    writeIntoEEPROM(NUM_SWAP_TRIPS, s.num_swap_trips);
    s.last_st_start_soc = s.this_st_start_soc;
    s.last_st_final_soc = s.current_soc;
    s.Voltage = t.Voltage;
   // s.max_temp = q.max_temp;
    writeIntoEEPROM(LAST_ST_START_SOC, s.last_st_start_soc);
    writeIntoEEPROM(LAST_ST_FINAL_SOC, s.last_st_final_soc);
    writeIntoEEPROM(VOLTAGE, s.Voltage);
  writeIntoEEPROM(MAX_TEMP, s.max_temp);
    s.this_st_start_soc = t.SOC;
    writeIntoEEPROM(THIS_ST_START_SOC, s.this_st_start_soc);
    s.current_soc = t.SOC;
    writeIntoEEPROM(CURRENT_SOC, s.current_soc);
    s.kms_last_st = s.kms_this_st;
    writeIntoEEPROM(KMS_LAST_ST, s.kms_last_st);
    s.kms_this_st = 0;
    s.dist_start_st = s.eeprom_distance;
    writeIntoEEPROM(DIST_START_ST, s.dist_start_st);
    //loadEEPROMData();
  }
}


void printAppData(int bikeSpeed,float max_temp){
  if (DEBUGLVL == 2) {
    Serial.println();
    Serial.print("App Data:\n  No. of Trips= ");
    Serial.println(s.num_trips);
    Serial.print("  No. of Speeds in Trip= ");
    Serial.println(s.num_speed_in_trip);
    Serial.print("  EEPROM Distance= ");
    Serial.println(s.eeprom_distance);
    Serial.print("  Distance= ");
    Serial.println(s.distance);
    Serial.print("  Avg. speed of current trip= ");
    Serial.println(s.avg_speed_this_t);
 ///Serial.print("  Avg. speed = ");
//    Serial.println(s.avg_speed);
   // Serial.print("Speed");
    //Serial.println(s.new_speed_t);
    Serial.print("  Current SOC = ");
    Serial.print(s.current_soc);
    Serial.println();
  }

  if (DEBUGLVL == 1) {
    Serial.print("App Data: trip=");
    Serial.print(s.num_trips);
    Serial.print(", num_speed=");
    Serial.print(s.num_speed_in_trip);
    Serial.print(", eeprom_distance=");
    Serial.print(s.eeprom_distance);
    Serial.print(", Distance=");
    Serial.print(s.distance);
    Serial.print(", avg_v_this_t=");
    Serial.print(s.avg_speed_this_t);
//    Serial.print(", avg_v=");
//    Serial.print(s.avg_speed);
    Serial.print(", current_soc=");
    Serial.print(s.current_soc);
    Serial.print(", 1st_soc=");
    Serial.print(s.this_st_start_soc);
    Serial.print(", 2st_soc=");
    Serial.print(s.last_st_start_soc);
    Serial.print(", 2end_soc=");
    Serial.print(s.last_st_final_soc);
    Serial.print(", dist_start=");
    Serial.print(s.dist_start_st);
    Serial.print(", kms1=");
    Serial.print(s.kms_this_st);
    Serial.print(", kms2_st=");
    Serial.println(s.kms_last_st);
  }

 if (DEBUGLVL == 3) {
 // bmsbasic mybms =get_Basic_BMS_Data();
 // bmsData1 q =  get_BMS_Data1();
 // bikeDataStruct bikeSt =getUartData();
  Serial.print("App Data: trip=");
  Serial.print(s.num_trips);
  Serial.print(", voltage=");
  Serial.print(s.Voltage);
  Serial.print(", current_soc=");
  Serial.print(s.current_soc);
  Serial.print(", 1st_soc=");
  Serial.print(s.this_st_start_soc);
  Serial.print(", 2st_soc=");
  Serial.print(s.last_st_start_soc);
  Serial.print(", 2end_soc=");
  Serial.print(s.last_st_final_soc);
  Serial.print(": max_Temp=");
  Serial.print(max_temp);
  //Serial.print("C at Cell No.=");
  //Serial.print(q.max_temp_cell_no);
  //Serial.print(", min_Temp=");
  //Serial.print(q.min_temp);
  //Serial.print("C at Cell No.=");
 // Serial.print(q.min_temp_cell_no);
  //Serial.print(" Gear=");
  //Serial.print(bikeSt.gear);
    Serial.print(" Speed =");
    //Serial.println(s.new_speed_t);
    Serial.println(bikeSpeed);
    
 }

}
