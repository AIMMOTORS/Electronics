//SoftwareSerial SerialPort(D2, D1); //RX,TX D1, D2 Define hardware connections          for old IOT
SoftwareSerial SerialPort(D1, D2); //RX,TX D1, D2 Define hardware connections          for new IOT
//SoftwareSerial SerialPort(4, 5, false, 256); //Define hardware connections
/**
 * @brief Initializing serial communication
 */
void setupUart() {
  SerialPort.begin(9600); //motor controller serial
  //delay(1000);
}

/**
 *  @brief Gets data from Votol controller via UART interface like gear, speed and brake 
 */
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
      
      // //Find Gear
      // if (mybytes[20] & 0b00000100) bikeSt.gear = -1;
      // else if ((mybytes[20] & 0b00000010) && (mybytes[20] & 0b00000001)) bikeSt.gear = 4;
      // else if (mybytes[20] & 0b00000010) bikeSt.gear = 3; 
      // else if (mybytes[20] & 0b00000001) bikeSt.gear = 2;
      // else bikeSt.gear = 1;

      // Serial.print("Motor Data: Gear=");
      // Serial.println(bikeSt.gear);

      // //Find Brake status
      // if ((mybytes[20] & 0b10000000) || (mybytes[20] & 0b00010000)) bikeSt.brake = 1;
      // else bikeSt.brake = 0;

      // Serial.print("Brake=");
      // Serial.println(bikeSt.brake);

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
