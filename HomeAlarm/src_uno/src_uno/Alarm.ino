
// ALARM ALARM ALARM!!! 
void AlarmTriggered(String location) {
    //Serial.println(F("ALARM ALARM!!!"));
    const String ALARM_MSG = F("Your alarm was just triggered "); //F(ALARM!!! ALARM!!!)    //F("Larmet gick ");
    SMSallusers(ALARM_MSG + F("\nLocation: ") + location);

    if (w8forRFID(120000) == true) { //2min
        setAlarm(false);
        String tempmsg = F("alarm was turned off");
        SMSallusers(tempmsg);//send SMS "alarm was turned off"
    }
    else {
        SMSallusers(ALARM_MSG + (String)ISRmotionDetectedFlag + F(" times"));    //send SMS with motionflag
    }
}


void setLed(bool state){
    for(int i = (int)state; i < (int)state + 5; i++){
        digitalWrite(ALARMSTATE_LEDPIN, (bool)(i % 2));  //Sätt på/stäng av en Led som visar om larmet är på eller av
        //tone(pin, toneheight, 50);
        delay(150);
    }
}

