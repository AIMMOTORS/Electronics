/*
Author: Muhammad Hashir Bin Khalid
Date: 3-8-2023
This file contains functions to read data from VOTOL EM-100 motor controller's UART interface.
*/

//SoftwareSerial SerialPort(5, 4); //Define hardware connections     before 
SoftwareSerial SerialPort(D1, D2); //Define hardware connections          change
//SoftwareSerial SerialPort(4, 5, false, 256); //Define hardware connections

void setupUart() {
  SerialPort.begin(9600);
  delay(1000);
}

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
    byte mybytes[24];
    int bytecount = SerialPort.readBytes(mybytes, 24);
    if (mybytes[0] == 0xC0 && mybytes[1] == 0x14) {         //Check if data is valid and correct
      bikeSt.readSuccess = 1;

      //Find Gear
      if (mybytes[20] & 0b00000100) bikeSt.gear = -1;
      else if ((mybytes[20] & 0b00000010) && (mybytes[20] & 0b00000001)) bikeSt.gear = 4;
      else if (mybytes[20] & 0b00000010) bikeSt.gear = 3; 
      else if (mybytes[20] & 0b00000001) bikeSt.gear = 2;
      else bikeSt.gear = 1;

      //Find Brake status
      if ((mybytes[20] & 0b10000000) || (mybytes[20] & 0b00010000)) bikeSt.brake = 1;
      else bikeSt.brake = 0;

      //Find Speed
      float xs = mybytes[14] * 256 + mybytes[15];
      int Speed = round((-0.778295) + (0.0442666) * (xs));
      if (Speed<0 and Speed>-5) Speed=0;
      if (Speed>=0 and Speed<140) bikeSt.bikeSpeed = Speed;
    }
    else {
      if(DEBUGLVL>0){Serial.println("Error");}
      while (SerialPort.available() > 0) {SerialPort.read();}
    }
  
  if(DEBUGLVL>0){
  Serial.print("Motor Data: Gear=");
  Serial.print(bikeSt.gear);
  Serial.print(",Brake=");
  Serial.print(bikeSt.brake);
  Serial.print(",Speed=");
  Serial.println(bikeSt.bikeSpeed);}
  }
  else{
    if(DEBUGLVL>0){
      Serial.println("Motor data not received");
      }
    }
  return bikeSt;
}

/*
void setup()
{
  setupUart();
}
void loop()
{
  getUartData();
  delay(100);
}*/
