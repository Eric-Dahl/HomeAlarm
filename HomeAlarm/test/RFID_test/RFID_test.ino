#include <SPI.h>
#include <MFRC522.h>
#include <AlarmData.h>


MFRC522 RFID(10, 6);
AlarmData alarmdata;

void setup() {
    Serial.begin(9600);
    SPI.begin();            // Init SPI bus
    RFID.PCD_Init();     // Init MFRC522
    
    String args1[3] = {"Eric", "+46725432410", "0"};
    byte rfid1[4] = { 29, 248, 83, 197 };
    User me(args1, rfid1);
    alarmdata.users.add(me);
    
    if(w8forRFID(100000)){
        Serial.println("true");
    }
    else{
        Serial.println("false");
    }
}

void loop() {
  // put your main code here, to run repeatedly:
}


bool w8forRFID(uint32_t duration) { //duration before trigger, waiting for someone to show a RFID card (milliseconds
    Serial.println(F("w8forRFID start"));
  uint32_t startTime = millis();
  //Only ("new")? cards in state IDLE are invited. Sleeping cards in state HALT are ignored.    v !!!KOLLA!!! v
  while (!RFID.PICC_IsNewCardPresent() || !RFID.PICC_ReadCardSerial()) { //rfid module performs a search for nearby cards(return true if found)  &&  reads the uid of the card found to the var RFID.uid
    if (duration <= millis() - startTime) { //duration timed out
      Serial.println(F("w8forRFID time out"));
      return false;
    }
    delay(20);
  }
  for(int i = 0; i < 4; i++){
    Serial.println(RFID.uid.uidByte[i]);
  }
  if (alarmdata.verifyRFID(RFID.uid.uidByte)) {   //if verification ok, return true
    Serial.println(F("w8forRFID true"));
    return true;
  }
  else {
    Serial.println(F("w8forRFID Continue"));
    w8forRFID(duration - (millis() - startTime));    //else recall function with the time of duration that remains
  }
}


