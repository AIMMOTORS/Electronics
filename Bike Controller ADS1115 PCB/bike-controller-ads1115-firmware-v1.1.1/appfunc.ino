float totalSpeed = 0;
unsigned long speedSamples = 0;
float avgSpeed;
uint8_t avgSpeedHex;

uint8_t testSpeeds[] = {20, 22, 25, 27, 30, 28, 0, 15, 18, 0, 20, 35, 55, 34, 23, 45, 32, 54, 56, 67, 34, 0}; // Test values
int speedIndex = 0;
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 1000;  // 1 second

unsigned long lastSpeedTime = 0;
unsigned long speedZeroSince = 0;
float distance = 0.0; 
float tripDistance = 0.0;
float lastTripDistance = 0;
float currentSwapTripDistance = 0;
float lastSwapTripDistance = 0;

int startTripSoc = 0;
int endTripSoc = 0;
int lastTripSocStart = 0;
int lastTripSocEnd = 0;
int swapStartSOC = 0;
int swapEndSOC = 0;
int lastSwapStartSOC = 0;
int lastSwapEndSOC = 0;
bool startSocCaptured = false;

String lastBatteryID = "";
String currentBatteryID = "";

bool tripActive = false;
const unsigned long tripEndThreshold = 120000; // 120 seconds
unsigned int tripCounter = 0; 
int swapTripCount = 0;
int lastSwapTripCount = 0;

bool chargeCycleDetected = false;
const int chargeSOCThreshold = 20; // min delta SOC to detect charge (tune as needed)
int currentChargeTripStartSOC = 0;
int currentChargeTripEndSOC = 0;
float currentChargeTripDistance = 0.0;

int lastChargeTripStartSOC = 0;
int lastChargeTripEndSOC = 0;
float lastChargeTripDistance = 0.0;

// int voltage;
// //uint8_t currentSoc=0;
// int temperature;
// int soh;
// int current;
// int remainingCapacity;
// //char battID[11];
// uint8_t individualCellVoltages[20];

//For ADS1115
float voltage = 0;     // ADS1115 scaled voltage
float vbat = 0.0;      // actual battery voltage after divider correction
//float currentSoc = 0.0;   // keep same style as before (store as 0-1000 if needed)
//char battID[20] = "";      // only keep if still needed elsewhere

typedef struct {
    float voltage;
    uint8_t soc;
} soc_table_t;

//Look up Table for 72V battery pack
soc_table_t soc_table[] = {
    {84.0, 100},
    {82.0, 90},
    {80.0, 80},
    {78.0, 70},
    {76.0, 60},
    {74.0, 50},
    {72.0, 40},
    {70.0, 30},
    {68.0, 20},
    {66.0, 10}
};


void updateAverageSpeed(uint8_t SpeedH) {
  if (SpeedH > 0) {
    totalSpeed += SpeedH;
    speedSamples++;
  }
}

float getAverageSpeed() {
  if (speedSamples == 0) return 0;
  return totalSpeed / speedSamples;
 
}

void AvgSpeed(){
  avgSpeed = getAverageSpeed();                    // Decimal average speed
  avgSpeedHex = round(avgSpeed);                 // // For HEX representation (rounded)
    Serial.print("SpeedH: ");
    Serial.print(SpeedH);
    Serial.print(" | Avg Speed: ");
    Serial.print(avgSpeed, 2);
    Serial.print(" km/h | Avg Speed HEX: ");
    Serial.println(avgSpeedHex, HEX);
}

// --- SpeedH Simulation Function ---
void simulateSpeedH() {
  
  if (millis() - lastUpdate >= updateInterval && speedIndex < sizeof(testSpeeds)) {
    Serial.println();
    Serial.println("Starting SpeedH Simulation...");
    lastUpdate = millis();

    SpeedH = testSpeeds[speedIndex++];
    updateAverageSpeed(SpeedH);

    Serial.print("SpeedH: ");
    Serial.print(SpeedH);
    Serial.print(" | Avg Speed: ");
    Serial.print(getAverageSpeed(), 2);
    Serial.print(" km/h | Avg HEX: ");
    Serial.println(String((uint8_t)round(getAverageSpeed()), HEX));
  }
  else 
    Serial.print(" simulation ends ");
}


void ads1115data() {
    int16_t adc0 = ads.readADC_SingleEnded(0);

    // ADS1115 output voltage
    voltage = ((adc0 + calib) * 0.1875) / 1000.0;

    // Actual battery voltage after reverse divider calculation
    vbat = voltage / Radjust;

    // Battery removed / invalid reading
    if (adc0 <= 0) {
        vbat = 0.0;
        currentSoc = 0.0;
    }
    else {
        int tableSize = sizeof(soc_table) / sizeof(soc_table[0]);

        // Above maximum table voltage
        if (vbat >= soc_table[0].voltage) {
            currentSoc = soc_table[0].soc;
        }
        // Below minimum table voltage
        else if (vbat <= soc_table[tableSize - 1].voltage) {
            currentSoc = soc_table[tableSize - 1].soc;
        }
        else {
            // Linear interpolation between two nearest voltage points
            for (int i = 0; i < tableSize - 1; i++) {
                if (vbat <= soc_table[i].voltage && vbat >= soc_table[i + 1].voltage) {
                    float v1 = soc_table[i].voltage;
                    float v2 = soc_table[i + 1].voltage;
                    float soc1 = soc_table[i].soc;
                    float soc2 = soc_table[i + 1].soc;

                    currentSoc = soc1 + ((vbat - v1) * (soc2 - soc1) / (v2 - v1));
                    break;
                }
            }
        }
    }

    Serial.println("ADS1115 Battery Data For App");
    Serial.print("Battery Voltage: ");
    Serial.print(vbat, 2);
    Serial.println(" V");

    Serial.print("Current SoC: ");
    Serial.print(currentSoc, 1);
    Serial.println(" %");
}


////New ADS Function to try to get correct values with the old ADS:
//// Universal calibration factor
// //Change this per ADS module after comparing with multimeter
// float voltageCalibrationFactor = 1.02937;


// // ================= ADS1115 FUNCTION =================

// void ads1115data() {

//     // Read ADS1115 channel 0
//     int16_t adc0 = ads.readADC_SingleEnded(0);

//     // Convert ADC reading to voltage at ADS input
//     voltage = ((adc0 + calib) * 0.1875) / 1000.0;

//     // Reverse voltage divider calculation
//     float rawVbat = voltage / Radjust;

//     // Apply universal calibration correction
//     vbat = rawVbat * voltageCalibrationFactor;

//     // Invalid reading / battery removed
//     if (adc0 <= 0) {
//         vbat = 0.0;
//         currentSoc = 0.0;
//         return;
//     }

//     // ================= SOC LOOKUP TABLE =================

//     int tableSize = sizeof(soc_table) / sizeof(soc_table[0]);

//     // Above maximum LUT voltage
//     if (vbat >= soc_table[0].voltage) {
//         currentSoc = soc_table[0].soc;
//     }

//     // Below minimum LUT voltage
//     else if (vbat <= soc_table[tableSize - 1].voltage) {
//         currentSoc = soc_table[tableSize - 1].soc;
//     }

//     // Linear interpolation
//     else {

//         for (int i = 0; i < tableSize - 1; i++) {

//             if (vbat <= soc_table[i].voltage &&
//                 vbat >= soc_table[i + 1].voltage) {

//                 float v1 = soc_table[i].voltage;
//                 float v2 = soc_table[i + 1].voltage;

//                 float soc1 = soc_table[i].soc;
//                 float soc2 = soc_table[i + 1].soc;

//                 currentSoc =
//                     soc1 +
//                     ((vbat - v1) * (soc2 - soc1) / (v2 - v1));

//                 break;
//             }
//         }
//     }

//     // ================= DEBUG =================

//     Serial.print("ADC Raw: ");
//     Serial.println(adc0);

//     Serial.print("Battery Voltage: ");
//     Serial.print(vbat, 2);
//     Serial.println(" V");

//     Serial.print("Current SoC: ");
//     Serial.print(currentSoc, 1);
//     Serial.println(" %");
// }



/*
void updateTripDistance(uint8_t SpeedH) {
  unsigned long currentTime = millis();
  if (lastSpeedTime == 0) {
    lastSpeedTime = currentTime;
    return;
  }
  unsigned long deltaT = currentTime - lastSpeedTime;
  lastSpeedTime = currentTime;

  float hours = deltaT / 3600000.0;
  tripDistance += SpeedH * hours; // km
  Serial.print("tripDistance: "); Serial.print(tripDistance ); Serial.println(" KMs" );
  Serial.println("==========");
}
*/




/*
This function is basically a trip state machine for the bike. Every time we call it in loop(), it:
-) keeps updating distance,
-) decides when a trip starts,
-) decides when a trip ends,
-) detects a battery swap and “closes out” the previous swap segment,
-) tracks charge-cycle info,
-) prints a big debug dump.
*/

void manageTrip(uint8_t SpeedH, int currentSoc, String battID) {
  updateTripDistance(SpeedH);
  // Start trip
  if (!tripActive && SpeedH > 5) {
    tripActive = true;
    //tripDistance = 0; will be 0 to calculate swap distance
    currentBatteryID = battID;
      if (!startSocCaptured) {
      startTripSoc = currentSoc;    //  Capture SOC only once
      startSocCaptured = true;      //  Block further overwrites
      Serial.print("Trip Start SOC: "); Serial.println(startTripSoc);
    }

     // If no swapStartSOC recorded yet for this swap cycle
    if (swapTripCount == 0) {
      swapStartSOC = currentSoc;
    }
    Serial.println("Trip Started...");
  }

  // Speed is zero end trip detection
  if (tripActive && SpeedH == 0) {
    if (speedZeroSince == 0) speedZeroSince = millis();
    else if (millis() - speedZeroSince > tripEndThreshold) {
      // Trip ended
      storeLastTrip(currentSoc); //end trip
    }
  } else {
    speedZeroSince = 0; // Reset if moving
  }

  // Detect battery swap
  if (isBatterySwapped(battID)) {
    // Save previous swap trip data
    lastSwapTripDistance = currentSwapTripDistance;
    lastSwapTripCount = swapTripCount;
    lastSwapStartSOC = swapStartSOC;
    lastSwapEndSOC = swapEndSOC;
 
    saveTripLogIfNew(false, true, false); // after swap detected
    storelastSTSOCToEEPROM(lastSwapTripCount, lastSwapStartSOC, lastSwapEndSOC);
    storeLastSwapTripDistance(lastSwapTripDistance);
    // Reset for new swap trip cycle
    currentSwapTripDistance = 0;
    swapTripCount = 0;
    swapStartSOC = currentSoc;
    swapEndSOC = currentSoc;

    lastBatteryID = battID;
  }

  detectChargeCycleOnTripStart(currentSoc, currentBatteryID);


    // Debugging Output
  Serial.println("======= Trip Stats =======");
  Serial.print("Current BatteryID: "); Serial.println(currentBatteryID);
  Serial.print("Last BatteryID: "); Serial.println(lastBatteryID);
  Serial.print("Trip Active: "); Serial.println(tripActive);
  Serial.print("Trip Distance: "); Serial.println(tripDistance);
  Serial.print("Last Trip Distance: "); Serial.println(lastTripDistance);
  Serial.print("Start SOC: "); Serial.println(lastTripSocStart);
  Serial.print("End SOC: "); Serial.println(lastTripSocEnd);

  Serial.print("Swap Trip Distance: "); Serial.println(currentSwapTripDistance);
  Serial.print("Swap Trip Count: "); Serial.println(swapTripCount);
  Serial.print("Swap Start SOC: "); Serial.println(swapStartSOC);
  Serial.print("Swap End SOC: "); Serial.println(swapEndSOC);

  Serial.print("Last Swap Distance: "); Serial.println(lastSwapTripDistance);
  Serial.print("Last Swap Count: "); Serial.println(lastSwapTripCount);
  Serial.print("Last Swap Start SOC: "); Serial.println(lastSwapStartSOC);
  Serial.print("Last Swap End SOC: "); Serial.println(lastSwapEndSOC);
  Serial.print("Total Trips: "); Serial.println(tripCounter);

  Serial.println("Charge Trip Info:");
  Serial.print("Current Charge Trip Start SOC: "); Serial.println(currentChargeTripStartSOC);
  Serial.print("Current Charge Trip End SOC: "); Serial.println(currentChargeTripEndSOC);
  Serial.print("Current Charge Trip Distance: "); Serial.println(currentChargeTripDistance, 2);
  Serial.print("Last Charge Trip Start SOC: "); Serial.println(lastChargeTripStartSOC);
  Serial.print("Last Charge Trip End SOC: "); Serial.println(lastChargeTripEndSOC);
  Serial.print("Last Charge Trip Distance: "); Serial.println(lastChargeTripDistance, 2);
  Serial.println("==========================");
}


// Update trip distance as vehicle moves
void updateTripDistance(uint8_t SpeedH) {
  unsigned long currentTime = millis();
  if (lastSpeedTime == 0) {
    lastSpeedTime = currentTime;
    return;
  }
  unsigned long deltaT = currentTime - lastSpeedTime;
  lastSpeedTime = currentTime;

  float hours = deltaT / 3600000.0;
  distance += SpeedH * hours; // km
  tripDistance += SpeedH * hours;
  currentSwapTripDistance += SpeedH * hours;
  currentChargeTripDistance += SpeedH * hours;

  Serial.print("Trip Distance: ");
  Serial.print(tripDistance);
  Serial.println(" KM");
  Serial.print("Current SwapTrip Distance: ");
  Serial.print(currentSwapTripDistance);
  Serial.println(" KM");
   Serial.print("Current ChargeTrip Distance: ");
  Serial.print(currentChargeTripDistance);
  Serial.println(" KM");
}

// Called when trip ends
void storeLastTrip(int currentSoc) {
  lastTripDistance = tripDistance;
  lastTripSocStart = startTripSoc;
  lastTripSocEnd = currentSoc; 
  swapEndSOC = currentSoc; // Update swap end SOC after every completed trip

  saveTripLogIfNew(true, false, false); // after trip end
  storeTripSOCsToEEPROM(lastTripSocStart, lastTripSocEnd, swapEndSOC);
  storeTripDistanceToLittleFS(lastTripDistance);

  tripDistance = 0;
  tripActive = false;
  speedZeroSince = 0;
  startTripSoc = 0;
  

  tripCounter++;  //
  swapTripCount++; 
  startSocCaptured = false;
  persistSwapTripData();

  Serial.println("Trip Ended...");
  Serial.print("Trip Count: "); Serial.println(tripCounter);
  Serial.print("Distance: "); Serial.println(lastTripDistance);
  Serial.print("Start SOC: "); Serial.println(lastTripSocStart);
  Serial.print("End SOC: "); Serial.println(lastTripSocEnd);
  Serial.println("========");
}

bool isBatterySwapped(String newID) {
  return newID != lastBatteryID;
}

void persistSwapTripData() {
  EEPROM.begin(EEPROM_SIZE);
  int len = currentBatteryID.length();
  EEPROM.write(EEPROM_LAST_BATT_ID_ADDR, len);
  for (int i = 0; i < len && i < 20; i++) {
    EEPROM.write(EEPROM_LAST_BATT_ID_ADDR + 1 + i, currentBatteryID[i]);
  }
  EEPROM.write(EEPROM_INIT_FLAG_ADDR, 0xAA);
  EEPROM.write(EEPROM_TRIP_FLAG_ADDR, tripActive ? 1 : 0);
  EEPROM.commit();

  File file = LittleFS.open("/swap_trip.txt", "w");
  if (file) {
    file.print(currentSwapTripDistance, 2); file.print(",");
    file.print(tripDistance, 2); file.print(",");
    file.print(startTripSoc); file.print(",");
    file.print(swapStartSOC); file.print(",");
    file.print(swapEndSOC); file.print(",");
    file.print(tripCounter); file.print(",");
    file.print(swapTripCount); file.print(",");
    file.print(currentChargeTripStartSOC); file.print(",");
    file.print(currentChargeTripEndSOC); file.print(",");
    file.print(currentChargeTripDistance, 2);
    file.close();
  }
}

void restoreSwapTripData() {
  EEPROM.begin(EEPROM_SIZE);
  if (EEPROM.read(EEPROM_INIT_FLAG_ADDR) != 0xAA) return;

  int len = EEPROM.read(EEPROM_LAST_BATT_ID_ADDR);
  char idBuffer[21] = {0};
  for (int i = 0; i < len && i < 20; i++) {
    idBuffer[i] = EEPROM.read(EEPROM_LAST_BATT_ID_ADDR + 1 + i);
  }
  lastBatteryID = String(idBuffer);
  tripActive = EEPROM.read(EEPROM_TRIP_FLAG_ADDR) == 1;

  File file = LittleFS.open("/swap_trip.txt", "r");
  if (file) {
    String data = file.readStringUntil('\n');
    
    // Split CSV values
    int idx[10]; // We expect up to 10 values now
    idx[0] = -1;
    for (int i = 1; i < 10; i++) {
      idx[i] = data.indexOf(',', idx[i - 1] + 1);
      if (idx[i] == -1 && i < 9) {
        // Missing expected values
        file.close();
        return;
      }
    }

    // Parse values safely
    currentSwapTripDistance     = data.substring(0, idx[1]).toFloat();
    tripDistance                = data.substring(idx[1] + 1, idx[2]).toFloat();
    startTripSoc                = data.substring(idx[2] + 1, idx[3]).toInt();
    swapStartSOC                = data.substring(idx[3] + 1, idx[4]).toInt();
    swapEndSOC                  = data.substring(idx[4] + 1, idx[5]).toInt();
    tripCounter                 = data.substring(idx[5] + 1, idx[6]).toInt();
    swapTripCount               = data.substring(idx[6] + 1, idx[7]).toInt();
    currentChargeTripStartSOC   = data.substring(idx[7] + 1, idx[8]).toInt();
    currentChargeTripEndSOC     = data.substring(idx[8] + 1, idx[9]).toInt();
    currentChargeTripDistance   = data.substring(idx[9] + 1).toFloat();

    file.close();
  }
  restoreTripData();
  restorelastSwap();
  restoreLastChargeCycleData();

  // Finalize unfinished trip
  if (tripActive) {
    Serial.println("Detected unfinished trip. Finalizing...");
    storeLastTrip(swapEndSOC);
    tripActive = false;
    persistSwapTripData();
  }
}
/*
void detectShutdownStoreState(int currentSoc) {
  if (tripActive) {
    persistSwapTripData();
  }
}*/


/*
 This function is a safety backup that periodically saves trip progress while the vehicle 
 is running, so if power shuts off suddenly you don’t lose trip data. Think of it as an autosave for your trip state.
 detectShutdownStoreState() checks if important trip data changed and, if it did, stores it to persistent memory (likely EEPROM or flash).
 It saves things like:
 -) current SOC
 -) trip distance
 -) swap trip info
 so if the device shuts down unexpectedly, we can recover the latest state.
*/
//eeprom smart managment
void detectShutdownStoreState(int currentSoc) {
  static unsigned long lastPersistTime = 0;
  static int lastRecordedSoc = -1;
  static float lastRecordedTripDistance = -1;

  unsigned long now = millis();

  bool socChanged = (currentSoc != lastRecordedSoc);
  bool distanceChanged = (abs(tripDistance - lastRecordedTripDistance) > 0.09);  // Threshold to avoid float noise
  bool timeElapsed = (now - lastPersistTime > 10000); // 10 seconds

  //if (tripActive && (socChanged || distanceChanged || timeElapsed)) {
   if (tripActive && (socChanged || distanceChanged )) {
    swapEndSOC = currentSoc;
    persistSwapTripData();

    // Update last recorded state
    lastPersistTime = now;
    lastRecordedSoc = currentSoc;
    lastRecordedTripDistance = tripDistance;
  }
}

void storeTripSOCsToEEPROM(int startSoc, int endSoc, int swapEnd) {
  EEPROM.write(EEPROM_TRIP_SOC_ADDR, startSoc);
  EEPROM.write(EEPROM_TRIP_SOC_ADDR + 1, endSoc);
  EEPROM.write(EEPROM_TRIP_SOC_ADDR + 2, swapEnd);
  EEPROM.commit();
}
void storeTripDistanceToLittleFS(float distance) {
  File file = LittleFS.open("/last_trip.txt", "w");
  if (file) {
    file.printf("%.2f", distance);
    file.close();
  }
}
void restoreTripData() {
  EEPROM.begin(EEPROM_SIZE);
  lastTripSocStart = EEPROM.read(EEPROM_TRIP_SOC_ADDR);
  lastTripSocEnd = EEPROM.read(EEPROM_TRIP_SOC_ADDR + 1);
  swapEndSOC = EEPROM.read(EEPROM_TRIP_SOC_ADDR + 2);

  File file = LittleFS.open("/last_trip.txt", "r");
  if (file) {
    String val = file.readStringUntil('\n');
    lastTripDistance = val.toFloat();
    file.close();
  }
}

void storelastSTSOCToEEPROM(int swapCount, int startSOC, int endSOC) {
  EEPROM.write(EEPROM_LASTST_SOC_ADDR, swapCount);
  EEPROM.write(EEPROM_LASTST_SOC_ADDR + 1, startSOC);
  EEPROM.write(EEPROM_LASTST_SOC_ADDR + 2, endSOC);
  EEPROM.commit();
}
void storeLastSwapTripDistance(float dist) {
  File file = LittleFS.open("/lastswap_dist.txt", "w");
  if (file) {
    file.printf("%.2f", dist);
    file.close();
  }
}
void restorelastSwap() {
  EEPROM.begin(EEPROM_SIZE);
  lastSwapTripCount = EEPROM.read(EEPROM_LASTST_SOC_ADDR);
  lastSwapStartSOC = EEPROM.read(EEPROM_LASTST_SOC_ADDR + 1);
  lastSwapEndSOC = EEPROM.read(EEPROM_LASTST_SOC_ADDR + 2);

  File file = LittleFS.open("/lastswap_dist.txt", "r");
  if (file) {
    String val = file.readStringUntil('\n');
    lastSwapTripDistance = val.toFloat();
    file.close();
  }
}

void detectChargeCycleOnTripStart(int currentSOC, String battID) {
  if (!tripActive && !chargeCycleDetected) {
    int socDelta = currentSOC - lastTripSocEnd;

    if (socDelta >= chargeSOCThreshold && battID == lastBatteryID) {
      // Charge cycle inferred
      lastChargeTripStartSOC = currentSOC;
      lastChargeTripEndSOC = lastTripSocEnd;
      lastChargeTripDistance = lastTripDistance;

      // Reset current charge trip tracking
      currentChargeTripStartSOC = currentSOC;
      currentChargeTripEndSOC = 0;
      currentChargeTripDistance = 0;

      saveTripLogIfNew(false, false, true); // after charge detected
      storeLastChargeCycleData(
        lastChargeTripStartSOC,
        lastChargeTripEndSOC,
        lastChargeTripDistance
      );

      Serial.println(" Charge Cycle Detected:");
      Serial.print("charg trip Start SOC: "); Serial.println(lastChargeTripStartSOC);
      Serial.print("charge trip End SOC: "); Serial.println(lastChargeTripEndSOC);
      Serial.print("charge Trip Distance: "); Serial.println(lastChargeTripDistance);

      chargeCycleDetected = true;
    }
  }

  // Update current charge trip data if trip has started
  if (tripActive && chargeCycleDetected) {
    currentChargeTripEndSOC = currentSOC;
    currentChargeTripDistance = tripDistance;
  }

  // Reset detection when trip ends
  if (!tripActive && chargeCycleDetected) {
    chargeCycleDetected = false;
  }
}

void storeLastChargeCycleData(int startSOC, int endSOC, float distance) {
  File file = LittleFS.open("/last_charge.txt", "w");
  if (file) {
    file.print(startSOC); file.print(",");
    file.print(endSOC); file.print(",");
    file.print(distance, 2);
    file.close();
  }
}

void restoreLastChargeCycleData() {
  File file = LittleFS.open("/last_charge.txt", "r");
  if (file) {
    String data = file.readStringUntil('\n');
    int idx1 = data.indexOf(',');
    int idx2 = data.indexOf(',', idx1 + 1);

    if (idx1 > 0 && idx2 > idx1) {
      lastChargeTripStartSOC = data.substring(0, idx1).toInt();
      lastChargeTripEndSOC = data.substring(idx1 + 1, idx2).toInt();
      lastChargeTripDistance = data.substring(idx2 + 1).toFloat();
    }

    file.close();
  }
}

/*
void saveTripLogIfNew(bool isTrip, bool isSwap, bool isCharge) {
  if (!isTrip && !isSwap && !isCharge) return;  // nothing new to log

  trimCSVIfNeeded();  // trims oldest lines if file grows too large

  if (!LittleFS.exists("/ev_trip_logs.csv")) {
    File csv = LittleFS.open("/ev_trip_logs.csv", "w");
    csv.println("TripDist,TripStartSOC,TripEndSOC,SwapDist,SwapStartSOC,SwapEndSOC,ChargeDist,ChargeStartSOC,ChargeEndSOC,BatteryID");
    csv.close();
  }

  File csv = LittleFS.open("/ev_trip_logs.csv", "a");
  if (csv) {
    csv.printf("%.2f,%d,%d,%.2f,%d,%d,%.2f,%d,%d,%s\n",
      lastTripDistance, lastTripSocStart, lastTripSocEnd,
      lastSwapTripDistance, lastSwapStartSOC, lastSwapEndSOC,
      lastChargeTripDistance, lastChargeTripStartSOC, lastChargeTripEndSOC,
      lastBatteryID.c_str());
    csv.close();
  }
}
*/

void saveTripLogIfNew(bool isTrip, bool isSwap, bool isCharge) {
  if (!isTrip && !isSwap && !isCharge) return;

  File file = LittleFS.open("/trip_logs.csv", "a");
  if (file) {
    file.print(lastTripDistance); file.print(",");
    file.print(lastTripSocStart); file.print(",");
    file.print(lastTripSocEnd); file.print(",");
    file.print(lastSwapTripDistance); file.print(",");
    file.print(lastSwapTripCount); file.print(",");
    file.print(lastSwapStartSOC); file.print(",");
    file.print(lastSwapEndSOC); file.print(",");
    file.print(lastChargeTripDistance); file.print(",");
    file.print(lastChargeTripStartSOC); file.print(",");
    file.print(lastChargeTripEndSOC); file.print(",");
    file.println(lastBatteryID);
    file.close();
  }

  trimCSVIfNeededRolling(); // keep as rolling window
}

/*
void trimCSVIfNeeded() {
  File csv = LittleFS.open("/ev_trip_logs.csv", "r");
  if (!csv) return;

  size_t size = csv.size();
  if (size < 4096) { // about 4KB limit, adjust if needed
    csv.close();
    return;
  }

  // Read all lines, skip first 10 to trim oldest
  String allLines[60];
  int lineCount = 0;

  while (csv.available() && lineCount < 60) {
    allLines[lineCount++] = csv.readStringUntil('\n');
  }
  csv.close();

  // Rewrite CSV with header + last lines
  csv = LittleFS.open("/ev_trip_logs.csv", "w");
  csv.println("TripDist,TripStartSOC,TripEndSOC,SwapDist,SwapStartSOC,SwapEndSOC,ChargeDist,ChargeStartSOC,ChargeEndSOC,BatteryID");
  for (int i = 10; i < lineCount; i++) {  // keep last ~50 entries
    csv.println(allLines[i]);
  }
  csv.close();
}
*/

void trimCSVIfNeededRolling() {
  File file = LittleFS.open("/ev_trip_logs.csv", "r");
  if (!file) return;

  // Read all lines into a vector
  std::vector<String> lines;
  while (file.available()) {
    String line = file.readStringUntil('\n');
    if (line.length() > 0) lines.push_back(line);
  }
  file.close();

  // Trim only if line count exceeds limit
  const int maxLines = 500;
  const int trimLines = 50;

  if ((int)lines.size() > maxLines) {
    lines.erase(lines.begin(), lines.begin() + trimLines); // remove oldest 50

    File newFile = LittleFS.open("/trip_logs.csv", "w");
    for (auto &line : lines) {
      newFile.println(line);
    }
    newFile.close();
  }
}
