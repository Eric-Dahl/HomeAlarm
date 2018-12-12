
// ISR:s (Interrupt Service Routines), wakes the program up and sets flags checked in checkInterruptFlags()
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

void checkInterruptFlags(){     //procedure: check flag, if flagged call a function and then unflag the flag
    //Serial.print(F("InterruptCheck start - "));
    if (ISRmotionDetectedFlag == 0 && ISRgsmFlag == false){     // if sleep mode ended for no reason (INT was not the cause)
        //Serial.println(F("None"));
    }//error
    else{
        if (ISRmotionDetectedFlag > 0){
            //Serial.println(F("Motion"));
            motionDetectedRoutine();
            ISRmotionDetectedFlag = 0; //reset flag
        }
        if (ISRgsmFlag == true){
            //Serial.println(F("GSM"));
            gsmRecieveRoutine();
            ISRgsmFlag = false; //reset flag
        }
    }

    if (BT_Active == true){ //i gsm- if-clause? 
        //Serial.println(F("BT Next"));
        Bluetooth_communication(1800000);   //enable bluetooth communication with alarm for 30 min (1800 000 ms)
        BT_Active = false;
    }
    //Serial.println(F("InterruptCheck end"));
}

bool readFrontdoor(){
    return analogRead(FRONTDOORPIN) > 250;//(FRONTDOORPIN);   //if pin is higher than 1,25V (usually outputs 2,5V (or 0V (no output)))
}

//__________________________________________________________________________________________________Routines

void motionDetectedRoutine() {
    //Serial.println(F("motionDetectedRoutine start"));
    //Serial.println((int)freeRam());

    if (readFrontdoor() == true){               // if motion at front door
        //Serial.println(F("Front door"));
        alarmdata.getState() ? //card found      card not found
           (w8forRFID(60000) ? setAlarm(false) : AlarmTriggered(F("Front door"))) :  //alarm currently on
           (w8forRFID(10000) ? setAlarm(true)  : delay(1));                         //alarm currently off
            
    }
    else {                                      // if motion at other transmitter
        //Serial.println(F("Other transmitter"));
        int vectorpos = alarmdata.matchRFdata(readRFmoduleID());    //get the transmitter's pos in transmitters (from the recieved ID)
        if (vectorpos != -1){                                       //RF module ID matched a transmitter in DB
            RFTransmitter* detectedSensor = alarmdata.transmitters.get(vectorpos); //get transmitter object (objects are passed by reference, therefore you can change the actual object)

            if (alarmdata.getState() == true){        //if alarm is turned on
                AlarmTriggered(detectedSensor->name.get());    //trigger
            }
            isBattLow(detectedSensor);
        }
        else{    //unknown transmitter ID
            eeprom_update_byte((uint8_t*)1000, 255);  //EEPROM warning flag (pos 1000)    EEP_UNKNOWN_SENSOR
        }
        delay(5000); //as the sensor pin is high for ~3 seconds and we don't want to go through this function several times on one sensor signal
    }
    EEPROM_update();
    //Serial.println(F("motionDetectedRoutine end"));
}

//SMS user interface
void gsmRecieveRoutine() {
    //Serial.println(F("GSMRecieveRoutine start"));
    String smsstring = recieveSMS(true);    // stores the all message data (necessary and unnecessary), arg. true -> deletes sms, so memory won't be full
    //Serial.println(smsstring);
    if (smsstring != F("")){    //if the sms was recieved (if no error)
        //Serial.println(F("SMS received"));
        //Serial.println((int)freeRam());
        SMS recievedsms(smsstring);     //make it an SMS object
        //Serial.println(recievedsms.number);
        int vectorpos = alarmdata.verifyPhone(recievedsms.number);    //Check if the sender is a user
        //Serial.println(vectorpos);
        if (vectorpos != -1){    //if the sender is a user

            //Serial.println(F("Sender verified"));
            User* sender = alarmdata.users.get(vectorpos);    //Who sent the message?
            //Serial.println((int)freeRam());
            if (sender->hasRestrictedAccess == false){
                String message2send = F("Message recieved");
                message2send.reserve(160);
                //Serial.println(sender->name.get());
                String sendername = sender->name.get();
                cmdHandler(recievedsms.message.command, recievedsms.message.args.get(), sendername, message2send); //call the cmdHandler to get a message to return to the sender
                //Serial.println(F("Sending..."));    
                sendSMS(recievedsms.number, message2send);                         //send back the message
                if (vectorpos != alarmdata.ADMIN){
                    sendSMS(alarmdata.users.get(alarmdata.ADMIN)->phone.get(), message2send);       //send the message to the admin, too
                }
            }
            else{
                String tempmsg = F("No access to SMS communication with the Alarm");
                sendSMS(recievedsms.number, tempmsg); //send back access denied message

            }

        }
    }
    //Serial.println(F("GSMRecieveRoutine end"));
}


