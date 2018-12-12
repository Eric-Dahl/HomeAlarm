void AlarmTriggered(String location) {
          //Serial.println(F("ALARM ALARM!!!"));
  const String LARMET_GICK = F("Larmet gick ");
  SMSallusers(LARMET_GICK + F("i ") + location);

  if (w8forRFID(120000) == true) { //2min
    setAlarm(false);
    String tempmsg = F("alarm was turned off");
    SMSallusers(tempmsg);//send SMS with "alarm was turned off"
  }
  else {
    SMSallusers(LARMET_GICK + (String)ISRmotionDetectedFlag + F(" gånger."));    //send SMS with motionflag
  }
}

void setLed(bool state){
    for(int i = (int)state; i < (int)state + 5; i++){
        digitalWrite(A4, (bool)(i % 2));  //Sätt på/stäng av en Led som visar om larmet är på eller av
        //tone(pin, toneheight, 50);
        delay(150);
    }
}

