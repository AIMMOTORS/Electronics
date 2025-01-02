ESP8266WebServer httpServer(80);

void setupAP(){
  Serial.print("  Starting AP: ");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(getssid(), "Nayel@123",0, 0);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("  AP: ");
  Serial.println(getssid());
  Serial.println();
  Serial.print("  AP IP address: ");
  Serial.println(IP);
  delay(1000);
  httpServer.on("/read", handleJson);
//    httpServer.on("/exit", handleExit);
    httpServer.begin();
}

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
 // String ssid = "NAYEL_1C 11";
  return ssid;
}

void handleJson(){

bikeDataStruct bikeSt =getUartData();
 //updateAtSpeedFetch(bikeSt.bikeSpeed);
//bmsbasic mybms =get_Basic_BMS_Data();
bmsData1 q =  get_BMS_Data1();
 StaticJsonDocument<256> doc;
  doc["mac_address"] = String(WiFi.macAddress());
  doc["millis"] = String(millis());
  doc["trips_count"] = s.num_trips;
  doc["Speed"]= bikeSt.bikeSpeed;
  doc["swap_trips_count"] = s.num_swap_trips;
  doc["voltage"] = s.Voltage;
  doc["max_temp"] = q.max_temp;
  doc["current_SOC"] = s.current_soc;
  doc["current_ST_start_SOC"] = s.this_st_start_soc;
  doc["last_ST_start_SOC"] = s.last_st_start_soc;
  doc["last_ST_final_SOC"] = s.last_st_final_soc;

  JsonObject errors = doc.createNestedObject("errors");

//  // Add some errors
  errors["error_1"] = "Invalid password";
  errors["error_2"] = "MAC address not found";
//
//  // Serialize the JSON object to a string
  String jsonString;
  serializeJson(doc, jsonString);
   
 //httpServer.sendHeader("Content-Length", String(jsonString.length()));
 httpServer.send(200, F("application/json"), jsonString);
//  httpServer.send(200, F("text/html"), doc);

//========================================================================================================previous jason=============
 /* StaticJsonDocument<256> doc;
  doc["password"] = 123456;
  doc["mac_address"] = String(WiFi.macAddress());
   //doc["mac_address"] = String("34:94:54:8E:1C:11");
  doc["millis"] = String(millis());
  doc["trips_count"] = s.num_trips;
  doc["distance"] = s.distance;
  doc["current_trip_speed_avg"] = s.avg_speed_this_t;
  doc["speed_avg"] = s.avg_speed_this_t;
  //doc["speed_avg"] =30;
  doc["swap_trips_count"] = s.num_swap_trips;
  doc["current_SOC"] = s.current_soc;
  doc["current_ST_start_SOC"] = s.this_st_start_soc;
  doc["last_ST_start_SOC"] = s.last_st_start_soc;
  doc["last_ST_final_SOC"] = s.last_st_final_soc;
  doc["current_ST_kms"] = s.kms_this_st;
  doc["last_ST_kms"] = s.kms_last_st;
//  doc["last_ST_range"] = 100;//100 *s.kms_last_st/(s.last_st_start_soc - s.last_st_final_soc) ; >causing reset on 17/11/23
  //doc["range_avg"] = ;
  JsonObject errors = doc.createNestedObject("errors");
//
//  // Add some errors
  errors["error_1"] = "Invalid password";
  errors["error_2"] = "MAC address not found";
//
//  // Serialize the JSON object to a string
  String jsonString;
  serializeJson(doc, jsonString);
   
 //httpServer.sendHeader("Content-Length", String(jsonString.length()));
 httpServer.send(200, F("application/json"), jsonString);
//  httpServer.send(200, F("text/html"), doc);   */

}

void handlemyClient(){
//  if (WiFi.softAPgetStationNum() == 0) {
//    // No clients connected, put ESP32 into low-power mode or idle state
//    // esp_sleep_enable_timer_wakeup(1000000LL * 60); // 1 minute sleep
//    // esp_deep_sleep_start();
//    Serial.println("No clients connected, ESP32 is in idle state");
//  }
//  else{
//    httpServer.on("/read", handleJson);
////    httpServer.on("/exit", handleExit);
//    httpServer.begin();
//  }
    httpServer.handleClient();
  }
