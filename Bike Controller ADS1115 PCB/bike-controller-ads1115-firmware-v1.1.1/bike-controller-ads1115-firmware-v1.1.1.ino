//Bike Controller ADS PCB Code: It will fetch battery voltage from ADS1115 and then calculate SoC using linear interpolation based on the LuT for 72V battery
//upgraded by ENGG. MARIA SHOAIB

#include "motorstructs.h"
#include "batterystructs.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_ADS1015.h> 
#include <SoftwareSerial.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <LittleFS.h>
#include <espnow.h>
#include <SPI.h>
//#include <mcp2515.h>

#define EEPROM_SIZE 128
#define EEPROM_LAST_BATT_ID_ADDR 0
#define EEPROM_INIT_FLAG_ADDR 63
#define EEPROM_TRIP_FLAG_ADDR 62
#define EEPROM_TRIP_SOC_ADDR 80
#define EEPROM_LASTST_SOC_ADDR 90


Adafruit_ADS1115 ads(0x48); // I2C address of ADS1115

//ESP8266WebServer httpServer(80);

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
  unsigned int design_capacity; //28 Ah 
  unsigned int full_capacity; 
  byte BPI;
  byte slot_id;
  byte flags;
  unsigned long time;
} struct_response;

// Create a struct_message called DHTReadings to hold sensor readings
struct_response response;

uint8_t rxB[8];
uint8_t rxB1[8];

#define SS_FORMAT 0     //for maaster
uint8_t MAX_temp;
//uint8_t rxB1[8];
long unsigned int rxId;
uint8_t len = 0;
uint8_t DEBUGLVL = 1;

uint8_t SpeedH = 0;
char battID[20] = "";    // only keep if still needed elsewhere      //Prev: char battID[11];
float currentSoc = 0.0;  // keep same style as before (store as 0-1000 if needed)      //  Prev: int currentSoc;
uint8_t Pack_Voltage_H;
uint8_t Pack_Voltage_L;
float Radjust = 0.02941176470588235294117; //0.043421905; // Voltage divider ratio = R2 / (R1 + R2) ==> 10k/(330k + 10k) ==> 0.02941176

int calib = 0; //Prev: 7 // Calibration offset for ADC


void setup() {
  Serial.begin(9600);
  delay(1000);
  setupAP(); //Imp
  ads.begin();
  setupUart(); //motor uart
  LittleFS.begin();
  EEPROM.begin(EEPROM_SIZE);
  restoreSwapTripData();
}



void loop() {
   handlemyClient();
   bikeDataStruct ud = getUartData(); //Motor UART //UART Motor controller //for real time
   updateAverageSpeed(SpeedH);  // for realtime
   //getAverageSpeed();
   AvgSpeed();      // for realtime
   //simulateSpeedH();  //test
   //updateTripDistance(SpeedH);

   ads1115data(); //Newly added

   manageTrip(SpeedH, currentSoc, battID); //Important function, we'll keep it for ADS integration f/w
   detectShutdownStoreState(currentSoc);   //Important function, we'll keep it for ADS integration f/w
  //delay(1000);
}
 