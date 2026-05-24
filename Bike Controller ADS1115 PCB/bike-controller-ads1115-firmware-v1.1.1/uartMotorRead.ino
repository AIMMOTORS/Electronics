//SoftwareSerial SerialPort(D2, D1); //RX,TX D1, D2 Define hardware connections          for old IOT
SoftwareSerial SerialPort(D6, D5); //D1, D2   D6, D5 RX, TX, inverted, buffer size//SoftwareSerial SerialPort(4, 5, false, 256); //Define hardware connections
/**
 * @brief Initializing serial communication
 */
void setupUart() {
  SerialPort.begin(9600);
  //SerialPort.setTimeout(1000);
  Serial.println("Motor UART Enabled");
}

// /**
//  *  @brief Gets data from Votol controller via UART interface like gear, speed and brake 
//  */
////New func
// bikeDataStruct getUartData() {
//   bikeDataStruct bikeSt;

//   bikeSt.readSuccess = 0;
//   bikeSt.gear = 0;
//   bikeSt.brake = 0;
//   bikeSt.bikeSpeed = 0;

//   byte sendbytes[] = {
//     0xC9, 0x14, 0x02, 0x53, 0x48, 0x4F, 0x57,
//     0x00, 0x00, 0x00, 0x00, 0x00,
//     0xAA, 0x00, 0x00, 0x00,
//     0x25, 0xAA, 0x05, 0xB5,
//     0x00, 0x41, 0x08, 0x0D
//   };

//   // Clear old leftover bytes BEFORE sending request
//   while (SerialPort.available() > 0) {
//     SerialPort.read();
//   }

//   // Send request to motor controller
//   SerialPort.write(sendbytes, 24);

//   // Wait until 24 bytes arrive, max 500 ms
//   unsigned long start = millis();
//   while (SerialPort.available() < 24 && millis() - start < 500) {
//     yield();
//   }

//   // Read response
//   byte mybytes[24];
//   int bytecount = 0;

//   while (SerialPort.available() > 0 && bytecount < 24) {
//     mybytes[bytecount++] = SerialPort.read();
//   }

//   Serial.print("UART bytecount = ");
//   Serial.println(bytecount);

//   Serial.print("Frame = ");
//   for (int i = 0; i < bytecount; i++) {
//     if (mybytes[i] < 16) Serial.print("0");
//     Serial.print(mybytes[i], HEX);
//     Serial.print(" ");
//   }
//   Serial.println();

//   // Validate frame
//   if (/*bytecount == 24 &&*/ mybytes[0] == 0xC0 && mybytes[1] == 0x14) {
//     bikeSt.readSuccess = 1;

//     // Gear
//     if (mybytes[20] & 0b00000100) {
//       bikeSt.gear = -1;
//     }
//     else if ((mybytes[20] & 0b00000010) && (mybytes[20] & 0b00000001)) {
//       bikeSt.gear = 4;
//     }
//     else if (mybytes[20] & 0b00000010) {
//       bikeSt.gear = 3;
//     }
//     else if (mybytes[20] & 0b00000001) {
//       bikeSt.gear = 2;
//     }
//     else {
//       bikeSt.gear = 1;
//     }

//     // Brake
//     if ((mybytes[20] & 0b10000000) || (mybytes[20] & 0b00010000)) {
//       bikeSt.brake = 1;
//     }
//     else {
//       bikeSt.brake = 0;
//     }

//     // Speed
//     float xs = mybytes[14] * 256 + mybytes[15];
//     int Speed = round((-0.778295) + (0.0442666 * xs));

//     if (Speed < 0 && Speed > -5) {
//       Speed = 0;
//     }

//     if (Speed >= 0 && Speed < 140) {
//       bikeSt.bikeSpeed = Speed;
//     }

//     SpeedH = bikeSt.bikeSpeed;

//     Serial.print("Motor Data: Gear = ");
//     Serial.println(bikeSt.gear);

//     Serial.print("Brake = ");
//     Serial.println(bikeSt.brake);

//     Serial.print("Speed = ");
//     Serial.println(bikeSt.bikeSpeed);

//     Serial.print("SpeedH HEX = ");
//     Serial.println(SpeedH, HEX);
//   }
//   else {
//     Serial.println("Invalid or incomplete UART frame");
//   }

//   return bikeSt;
// }





////Old Function
bikeDataStruct getUartData() {
  bikeDataStruct bikeSt;

  //Send request
  byte sendbytes[] = {0xC9, 0x14, 0x2, 0x53, 0x48, 0x4F, 0x57, 0x0, 0x0, 0x0, 0x0, 0x0, 0xAA, 0x0, 0x0, 0x0, 0x25, 0xAA, 0x5, 0xB5, 0x0, 0x41, 0x8, 0xD};
  for (int i = 0; i <= 23; i++) {
    SerialPort.write(sendbytes[i]);
  }
  bikeSt.readSuccess =0;
  
  //Wait for receiving data
  if (SerialPort.available()) {
    while (SerialPort.available() > 0) {SerialPort.read();}
    //Serial.println("Inside Serial");
    byte mybytes[24];
    int bytecount = SerialPort.readBytes(mybytes, 24);
    if (mybytes[0] == 0xC0 && mybytes[1] == 0x14) {         //Check if data is valid and correct
      //Serial.println("Inside Serial port 1 ");
      bikeSt.readSuccess = 1;
      
      //Find Gear
      if (mybytes[20] & 0b00000100) bikeSt.gear = -1;
      else if ((mybytes[20] & 0b00000010) && (mybytes[20] & 0b00000001)) bikeSt.gear = 4;
      else if (mybytes[20] & 0b00000010) bikeSt.gear = 3; 
      else if (mybytes[20] & 0b00000001) bikeSt.gear = 2;
      else bikeSt.gear = 1;

      Serial.print("Motor Data: Gear=");
      Serial.println(bikeSt.gear);

      //Find Brake status
      if ((mybytes[20] & 0b10000000) || (mybytes[20] & 0b00010000)) bikeSt.brake = 1;
      else bikeSt.brake = 0;

      Serial.print("Brake=");
      Serial.println(bikeSt.brake);

      //Find Speed
      float xs = mybytes[14] * 256 + mybytes[15];
      int Speed = round((-0.778295) + (0.0442666) * (xs));
      
      if (Speed<0 and Speed>-5) Speed=0;
      if (Speed>=0 and Speed<140) bikeSt.bikeSpeed = Speed;
      //SpeedH=(bikeSt.bikeSpeed,HEX);
      //Serial.println("SpeedH: "); Serial.println(SpeedH);
      Serial.print("Speed=");
      Serial.println(bikeSt.bikeSpeed);
      SpeedH = bikeSt.bikeSpeed;
      Serial.print("SpeedH: "); 
      Serial.println(SpeedH,HEX);
    }
    else {
      //if(DEBUGLVL>0){Serial.println("Error");}
       while (SerialPort.available() > 0) {SerialPort.read();}
    }
  
  }

   return bikeSt;
}
