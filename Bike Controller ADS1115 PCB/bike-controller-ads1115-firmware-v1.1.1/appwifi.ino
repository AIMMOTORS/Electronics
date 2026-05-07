//soft ip: http://192.168.4.1/read

ESP8266WebServer httpServer(80);

//Current function
void setupAP(){
  Serial.print("  Starting AP: ");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(getssid(), "Nayel@123",1, 0); //Prev from NED: WiFi.softAP(getssid(), "Nayel@123",0, 0); ////WiFi.softAP(ssid, password, channel, hidden, max_connection)

  IPAddress IP = WiFi.softAPIP();
  Serial.print("  AP: ");
  Serial.println(getssid());
  Serial.println();
  Serial.print("  AP IP address: ");
  Serial.println(IP);
  delay(1000);
  httpServer.on("/read", handleJson);
//    httpServer.on("/exit", handleExit);
  httpServer.on("/erase", handleEraseMemory); // to erase data
  httpServer.on("/allerase", handleEraseAllMemory); //erase all data runtime and stored
  httpServer.on("/download", handleDownloadCSV);  // download log files
  
    httpServer.begin();
}


//Another function to try

// void setupAP(){
//   Serial.print("  Starting AP: ");
//   WiFi.mode(WIFI_AP);
//   WiFi.softAP(getssid(), "Nayel@123",1, 0); //Prev from NED: WiFi.softAP(getssid(), "Nayel@123",0, 0); ////WiFi.softAP(ssid, password, channel, hidden, max_connection)

//   IPAddress IP = WiFi.softAPIP();
//   Serial.print("  AP: ");
//   Serial.println(getssid());
//   Serial.println();
//   Serial.print("  AP IP address: ");
//   Serial.println(IP);
//   delay(1000);
//   httpServer.on("/", []() {
//     httpServer.send(200, "text/html",
//       "<h2>NAYEL Bike Controller</h2>"
//       "<p>Server is running.</p>"
//       "<p><a href='/read'>Read JSON Data</a></p>"
//       "<p><a href='/download'>Download CSV</a></p>"
//     );
//   });

//   httpServer.on("/read", handleJson);
//   httpServer.on("/erase", handleEraseMemory);
//   httpServer.on("/allerase", handleEraseAllMemory);
//   httpServer.on("/download", handleDownloadCSV);

//   httpServer.begin();
// }


/**
 * @brief Performs cleanup actions (when a web server connection needs to be closed) like disabling cache and closing the server connection 
 */
void handleExit(){
  httpServer.sendHeader("Cache-Control", "no-cache");
  httpServer.close();
}

String getssid(){ //last 4 bytes of mac based ssid 
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char baseMacChr[18] = {0};//%02X:%02X:%02X:%02X:
  sprintf(baseMacChr, "%02X%02X", mac[4], mac[5]);   //mac[0], mac[1], mac[2], mac[3],
  String ssid = "NAYEL_" + String(baseMacChr);
  return ssid;
}

//New Func
void handleJson(){
  StaticJsonDocument<2048> doc;
  doc["mac_address"] = String(WiFi.macAddress());
  doc["SpeedH_dec"] = SpeedH;
  doc["speed_avg"] = avgSpeed;  //app variable
  JsonObject voltageObj = doc.createNestedObject("vbat");
  voltageObj["dec"] = vbat;
  doc["SOC"] = currentSoc;   
  doc["current_SOC"] = currentSoc;   //app variable
 // doc["Current_BatteryID_hex"] = battID;
  doc["Current_BatteryID_hex"]= currentBatteryID;
 // doc["Current_BatteryID"] = (currentBatteryID, HEX);
  doc["Last_BatteryID"] = lastBatteryID;
  doc["Trip Active"] = tripActive;
  doc["Distance_kms"] = distance;   //for debugg value and cross check
  doc["Trip_Distance_kms"] = tripDistance;
  doc["last_trip_distance_km"] = lastTripDistance;
  doc["last_trip_soc_start"] = lastTripSocStart * 0.1;
  doc["last_trip_soc_end"] = lastTripSocEnd * 0.1;
  doc["current_ST_kms"] = currentSwapTripDistance;  // app variable
  doc["Swap_Trip_Count"] = swapTripCount; 
  doc["Swap_Start_Soc"] = swapStartSOC* 0.1; 
  doc["Swap_End_Soc"] = swapEndSOC* 0.1;  
  doc["last_ST_kms"] = lastSwapTripDistance;   // app variable
  doc["Last_Swap_Trip_Count"] = lastSwapTripCount; 
  doc["Last_Swap_Start_Soc"] = lastSwapStartSOC * 0.1; 
  doc["Last_Swap_End_Soc"] = lastSwapEndSOC * 0.1;
  doc["Current_Charge_Trip_Start_SOC"] = currentChargeTripStartSOC * 0.1;
  doc["Current_Charge_Trip_End_SOC"] = currentChargeTripEndSOC * 0.1;
  doc["Current_Charge_Trip_Distance"] = currentChargeTripDistance;
  doc["last_Charge_Trip_Start_SOC"] = lastChargeTripStartSOC * 0.1;
  doc["last_Charge_Trip_End_SOC"] = lastChargeTripEndSOC * 0.1;
  doc["last_Charge_Trip_Distance"] = lastChargeTripDistance;
  doc["Total_trips"] = tripCounter;
  String jsonString;
  serializeJson(doc, jsonString);
   
 //httpServer.sendHeader("Content-Length", String(jsonString.length()));
 httpServer.send(200, F("application/json"), jsonString);
//  httpServer.send(200, F("text/html"), doc);
  
}

void handlemyClient(){

    httpServer.handleClient();
}
  //to erase eeprom and flash memory
void handleEraseMemory() {
  Serial.println("Erasing EEPROM and LittleFS...");

  // Erase EEPROM
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 0xFF);
  }
  EEPROM.commit();
  EEPROM.end();

  // Erase LittleFS
  LittleFS.begin();
  LittleFS.format(); // Wipes entire file system

  httpServer.send(200, "text/plain", "EEPROM and LittleFS erased successfully.");
  Serial.println("Erase complete.");
}

//to erase eeprom ,flash and runtime memory
void handleEraseAllMemory() {
  Serial.println("Erasing EEPROM, LittleFS, and clearing runtime variables...");

  // Erase EEPROM
  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 0xFF);
  }
  EEPROM.commit();
  EEPROM.end();

  // Erase LittleFS
  LittleFS.begin();
  LittleFS.format(); // Wipes entire file system

  // Reset all key runtime variables
  tripDistance = 0;
  lastTripDistance = 0;
  currentSwapTripDistance = 0;
  lastSwapTripDistance = 0;
  tripCounter = 0;
  swapTripCount = 0;
  lastSwapTripCount = 0;

  startTripSoc = 0;
  lastTripSocStart = 0;
  lastTripSocEnd = 0;
  swapStartSOC = 0;
  swapEndSOC = 0;
  lastSwapStartSOC = 0;
  lastSwapEndSOC = 0;

  tripActive = false;
  startSocCaptured = false;
  //tripEndingPending = false;
  currentChargeTripStartSOC = 0;
  currentChargeTripEndSOC = 0;
  currentChargeTripDistance = 0;
  lastChargeTripStartSOC = 0;
  lastChargeTripEndSOC = 0;
  lastChargeTripDistance = 0;

  speedZeroSince = 0;
  currentBatteryID = "";
  lastBatteryID = "";

  httpServer.send(200, "text/plain", "EEPROM, LittleFS, and all variables reset to default.");
  Serial.println("All memory and variables cleared.");
}

void handleDownloadCSV() {
  File f = LittleFS.open("/ev_trip_logs.csv", "r");
  if (!f) {
    httpServer.send(404, "text/plain", "No logs found");
    return;
  }
  httpServer.streamFile(f, "text/csv");
  f.close();
}