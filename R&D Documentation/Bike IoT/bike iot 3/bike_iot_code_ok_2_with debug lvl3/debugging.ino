void debugging() {
  char t='z';
  while (1) {
    if (t != '\n') {
      Serial.println("SETTINGS");
      Serial.print("a: Change Debug Level:");
      Serial.println(DEBUGLVL);
      Serial.println("b: Exit");
      Serial.println("c: Erase EEPROM");
      Serial.print("d: Change Motor Uart Enable:");
      Serial.println(motorUartEnable);
      Serial.print("e: Change CAN Enable:");
      Serial.println(canEnable);
      Serial.print("f: Change App Enable:");
      Serial.println(appEnable);
    }
    while (!Serial.available());
    t = Serial.read();
    if (t == 'a'||'A') {
      DEBUGLVL += 1;
      if (DEBUGLVL > MAXDEBUGLVL) DEBUGLVL = 0;
      EEPROM.write(DEBUG_ADDRESS, DEBUGLVL);
    }
    else if (t == 'b'||'B') {
      EEPROM.commit();
      break;
    }
    else if (t == 'c'||'C') {
      resetEEPROMData();
      Serial.println("EEPROM erased");
      loadEEPROMData();
    }
    else if (t == 'd'||'D') {
      motorUartEnable = !motorUartEnable;
      EEPROM.write(MOTOR_UART_ENABLE, motorUartEnable);
    }
    else if (t == 'e'||'E') {
      canEnable = !canEnable;
      EEPROM.write(CAN_ENABLE, canEnable);
    }
    else if (t == 'f'||'F') {
      appEnable = !appEnable;
      EEPROM.write(APP_ENABLE, appEnable);
    }

  }
  EEPROM.commit();
}


void loadConfigs(){
    DEBUGLVL = EEPROM.read(DEBUG_ADDRESS);
  motorUartEnable = (EEPROM.read(MOTOR_UART_ENABLE)>0);
  canEnable = (EEPROM.read(CAN_ENABLE)>0);
  appEnable = (EEPROM.read(appEnable)>0);  
}
