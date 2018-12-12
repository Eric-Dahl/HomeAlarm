bool *readRFmoduleID() {
  static bool ret[] = {(bool)digitalRead(A1), (bool)digitalRead(A2), (bool)digitalRead(A3)};
  return ret;
}

void isBattLow(RFTransmitter* Sensor){
    //when the battery has just gone low the value may flicker between high and low (3 times within some minutes when tested).
    //This clause is to prevent this flicker from annoying messages (batt low/restored/low/restored...
    if (Sensor->lowBattDebounce != 0){  
        if(millis() - Sensor->lowBattDebounce >= (long)(1000 * 3600 * 24 * 14)){ //time span 2 weeks (to make sure the value is steady)
            Sensor->lowBattDebounce = 0;
        }
        else{
            return;     //as value still may flicker
        }
    }
    
    bool low_now = isRFbatteryLow();
    bool low_then = Sensor->lowBatt;        //Transmitter battery-level last time)

    if (low_now != low_then && low_now == true) {    //Has the battery lvl has changed since last time?
        const String BATTERY_LEVEL = F("Low battery ");        //nice message        
        SMSallusers(BATTERY_LEVEL + F("at: ") + Sensor->name);    //send message (battery low/restored)
        
        Sensor->lowBatt = low_now;        //change transmitters[?].lowBatt to the other (bool) value (through pass by reference)
        Sensor->lowBattDebounce = millis();
    }
}
bool isRFbatteryLow() {
  return !(digitalRead(RF_BATT));   //check if the battery on the RF transmitter is low
}



bool w8forRFID(uint32_t duration) { //duration before trigger, waiting for someone to show a RFID card (milliseconds
  Serial.print(F("w8forRFID start\tduration: "));
  Serial.println(duration);
  uint32_t startTime = millis();
  //Only ("new")? cards in state IDLE are invited. Sleeping cards in state HALT are ignored.    v !!!KOLLA!!! v
  while( ! RFID.PICC_IsNewCardPresent() || ! RFID.PICC_ReadCardSerial() ) { //rfid module performs a search for nearby cards(return true if found)  &&  reads the uid of the card found to the var RFID.uid
    if (duration <= millis() - startTime) { //duration timed out
      Serial.print(F("w8forRFID end - "));
      Serial.println(F("false"));
      return false;
    }
    delay(20);
  }
  if (alarmdata.verifyRFID(RFID.uid.uidByte)) {   //if verification ok, return true
    Serial.print(F("w8forRFID end"));
    Serial.println(F("true"));
    return true;
  }
  else {
    Serial.print(F("w8forRFID continue"));
    eeprom_update_byte((uint8_t*)(EEP_UNKNOWN_TAG), 255);    //EEPROM error  EEP_UNKNOWN_RFIDTAG
    w8forRFID(duration - (millis() - startTime));    //else recall function with the time of duration that remains
  }
  return false; //should never come here, though
}


void Bluetooth_communication(uint32_t duration) {
  //SoftwareSerial Bluetooth(BT_RX, BT_TX);
  Serial3.begin(9600);      //Serial3 for Mega, SoftwareSerial for Uno
  Serial.println(F("in BT"));
  Serial.println((int)freeRam());
  uint32_t startTime = millis();
  String recv;    //container for Serial3 data recieved, which is managed
  Message BTmsg;
  while (duration >= millis() - startTime) {
    
    //SSerial.listen();
    //checkInterruptFlags();          //????
    //Serial3.listen();
    
    recv = "";
    while (Serial3.available()) {
      char c = Serial3.read();
      delay(5);
      if (c > (byte)127) {       //nordic and other more unusual characters
        delay(10);
        c = Serial3.read();
        c += 64;    //for some reason this method works :)
      }
      recv += c;
      Serial.println((int)freeRam());
    }
    if(recv == "" || recv.indexOf(F("Bluetooth Terminal ")) != -1){
        delay(50);
        continue;
    }
    Serial.println(recv);
    Serial.println(F("past"));
    BTmsg = recv;
        Serial.println(F("past2"));
    if (BTmsg.command.equalsIgnoreCase(F("QUIT"))) {
          Serial.println(F("quit"));
          break;
    }
    Serial.println(F("past3"));
    String btmsg2send;
    btmsg2send.reserve(160);
      Serial.println((int)freeRam());
    cmdHandler(BTmsg.command, BTmsg.args.get(), F("BT"), btmsg2send);
      Serial.println(btmsg2send);
    btmsg2send = tweakBTreturn(btmsg2send); //change to point arg?
    for(byte i = 0; i < btmsg2send.length(); i++){
        Serial3.print(btmsg2send[i]);
    }
    //Serial3.println(tweakBTreturn(btmsg2send));    //send back the fixed version of the returnmessage
          Serial.println((int)freeRam());

  }
}


String tweakBTreturn(String &rawReturn) {   //translate nordic characters (among others)###################################################################### pointer "return"
  String BTreturn = "";        //container for message back to the sender (by Bluetooth)
  for (byte i = 0; i <= rawReturn.length(); i++) {
    if (rawReturn[i] > 127) {        //the same block as above, just reversed
      rawReturn[i] -= 64;
      BTreturn += (char)0;
      BTreturn += (char)195;
    }
    BTreturn += rawReturn[i];
  }
  Serial.println((int)freeRam());

  return BTreturn;
}




