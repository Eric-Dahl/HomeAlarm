
void ISRmotionDetected(){
  sleep_disable();
  ISRmotionDetectedFlag++;
  //Serial.println(F("INT motion"));
}
void ISRgsm(){
  sleep_disable();
  ISRgsmFlag = true; 
  //Serial.println(F("INT gsm"));
}

void checkInterruptFlags(){
  Serial.print(F("InterruptCheck start - "));
  if(ISRmotionDetectedFlag == 0 && ISRgsmFlag == false){
      Serial.println(F("None"));
  }//error
  else{
      if(ISRmotionDetectedFlag > 0){
        Serial.println(F("Motion"));
        motionDetectedRoutine();
        ISRmotionDetectedFlag = 0; //reset flag
      }
      if(ISRgsmFlag == true){
        Serial.println(F("GSM"));
        gsmRecieveRoutine();
        ISRgsmFlag = false; //reset flag
      } 
  }
  
  if(BT_Active == true){ //i gsm- if-clause? 
    Serial.println(F("BT Next"));
    Bluetooth_communication(1800000);
    BT_Active = false;
  }
  Serial.println(F("InterruptCheck end"));
}

bool readFrontdoor(){
    return analogRead(A5) > 250;//(FRONTDOORPIN);   //if pin is higher than 1,25V (usually outputs 2,5V (or 0V (no output)))
}

//__________________________________________________________________________________________________Routines

void motionDetectedRoutine() {
    Serial.println(F("motionDetectedRoutine start"));
    Serial.println((int)freeRam());
    eeprom_update_byte((uint8_t*)1022, 99);
    //looong if-then-else-block        ?... <-(arg above true)   :...  <-(arg above false)
    if (readFrontdoor() == true){             //if motion at front door
        Serial.println(F("Front door"));
        alarmdata.getState()                      //...and if alarm is turned on...
            ? (w8forRFID(60000)                       //... and if RFID found and validated (within 60 sec)
                ? setAlarm(false)                         //...turn alarm off
                : AlarmTriggered(F("Front door")))                       //...trigger alarm
            : (w8forRFID(10000)                   //...and if alarm is not turned on and if RFID found and validated (within 10 sec)
                ? setAlarm(true)                      //turn alarm on
                : delay(1));                        //else
    }
    else {
        Serial.println(F("Other transmitter"));
        int vectorpos = alarmdata.matchRFdata(readRFmoduleID());    //get the transmitter's pos in transmitters (from the recieved ID)
        if(vectorpos != -1){    //RF module ID matched a transmitter
            RFTransmitter* detectedSensor = alarmdata.transmitters.get(vectorpos); //objects are passed by reference, therefore you can change the actual object

            if (alarmdata.getState() == true){        //if alarm is turned on
                AlarmTriggered(detectedSensor->name);    //trigger
            }
            isBattLow(detectedSensor);
        }
        else{    //unknown transmitter ID
            eeprom_update_byte((uint8_t*)EEP_UNKNOWN_SENSOR, 255);  //EEPROM warning    EEP_UNKNOWN_SENSOR
        }
        delay(5000); //as the sensor pin is high for ~3 seconds and we don't want to go through this function several times on one sensor signal
    }
    EEPROM_update();
    Serial.println(F("motionDetectedRoutine end"));
}

void gsmRecieveRoutine() {
    Serial.println(F("GSMRecieveRoutine start"));
    Serial.println((int)freeRam());
    //eeprom_update_byte((uint8_t*)1021, 99);
    
    String smsstring = recieveSMS(false);
    Serial.println((int)freeRam());
    Serial.println(smsstring);
    
    if (smsstring != F("")){    //if the sms was recieved (if no error)
      Serial.println(F("SMS received"));
      Serial.println((int)freeRam());
      SMS recievedsms(smsstring);     //make it an SMS object
      Serial.println(recievedsms.number);
      int vectorpos = alarmdata.verifyPhone(recievedsms.number);    //Check if the sender is a user
      //Serial.println((int)freeRam());
      Serial.println(vectorpos);
        if(vectorpos != -1){    //if the sender is a user

            Serial.println(F("Sender verified"));
            User* sender = alarmdata.users.get(vectorpos);    //Who sent the message?
            Serial.println((int)freeRam());
            if(sender->hasRestrictedAccess == false){
                Serial.println(F("&"));
                String message2send = F("Message recieved");
                message2send.reserve(160);        //not even here when line 101 commented (print "&")
                Serial.println(sender->name);
                String sendername = sender->name;
                cmdHandler(recievedsms.message.command, recievedsms.message.args.get(), sendername, message2send); //call the cmdHandler to get a message to return to the sender
                Serial.println(F("Sending..."));    //here when line 101 uncommented (print "&")
                sendSMS(recievedsms.number, message2send);                         //send back the message
                if(vectorpos != alarmdata.ADMIN){
                    sendSMS(alarmdata.users.get(alarmdata.ADMIN)->phone, message2send);       //send the message to the admin, too
                }
                
            }
            else{
               String tempmsg = F("No access to SMS communication with the Alarm");
               sendSMS(recievedsms.number, tempmsg); //send back access denied the message
            }
            
        }
    }
    Serial.println(F("GSMRecieveRoutine end"));
}




