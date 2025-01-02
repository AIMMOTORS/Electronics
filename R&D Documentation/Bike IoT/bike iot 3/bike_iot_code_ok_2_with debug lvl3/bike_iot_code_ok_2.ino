#include "batterystructs.h"
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <mcp_can.h>
#include <SoftwareSerial.h>
#include <ESP8266WebServer.h>

appdatastruct s;      //Global variable that contains all App's data
uint8_t DEBUGLVL;
bool motorUartEnable, canEnable, appEnable;
bool canInitialized;

void setup() {
  Serial.begin(9600);
  delay(1000);
  setup_CAN();
  setupUart();
  setupAP();
  setupEEPROM();
  loadEEPROMData();
  checkSwap();
  beginTrip();
  loadConfigs();
  Serial.print("DEBUG Level = ");
  Serial.println(DEBUGLVL);
  
  if (DEBUGLVL > 0) {Serial.println(WiFi.macAddress());}
  if ((Serial.available()) && (Serial.read() == 's'||'S')) {   //Enter Settings mode
    debugging();
  }
}

void loop() {
  handlemyClient();
  //CAN
 if(canEnable && canInitialized){
  bmsbasic t = get_Basic_BMS_Data();
  //bmsData1 q =  get_BMS_Data1();
  
  if ((t.SOC > (s.current_soc + 2)) || (t.SOC < (s.current_soc - 2))) {
    if (DEBUGLVL > 1) Serial.print("SOC Updated:"); Serial.println(t.SOC);
    s.current_soc = t.SOC;
    writeIntoEEPROM(CURRENT_SOC, uint32_t(s.current_soc * 10));
  } 
  
  
  // delay(1000);
  //  uint8_t *bat_Id = get_Bat_id();//Get Battery id: Untested
 }
  
  //UART Motor controller
 // if(motorUartEnable){
  bikeDataStruct ud = getUartData();
  bmsData1 q =  get_BMS_Data1();
  //if (ud.readSuccess) {
    // ud.bikeSpeed = 10;
    //if (ud.bikeSpeed > 0) {
      updateAtSpeedFetch(ud.bikeSpeed);
   // }
      printAppData(ud.bikeSpeed,q.max_temp);
     
  //} 
 // }

  //App data
//  if(appEnable){
  
  //delay(1000);
//  printAppData(ud.bikeSpeed);
  
}