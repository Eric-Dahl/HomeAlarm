
bool *readRFmoduleID() {
    static bool ret[] = { (bool)digitalRead(RF_D1), (bool)digitalRead(RF_D2), (bool)digitalRead(RF_D3) };
    return ret;
}

// Tells the users to change batteries on the distant motion sensors when needed (they run on 9V batteries)
void isBattLow(RFTransmitter* Sensor){
    //when the battery has just gone low the value may flicker between high and low (3 times within some minutes when tested).
    //This clause is to prevent this flicker from annoying messages (batt low/restored/low/restored...
    if (Sensor->lowBattDebounce != 0){
        if (millis() - Sensor->lowBattDebounce >= ((uint32_t)1000 * 3600 * 24 * 14)){ //time span 2 weeks (to make sure the value is steady)
            Sensor->lowBattDebounce = 0;
        }
        else{
            return;     //as value still may flicker
        }
    }

    bool low_now = isRFbatteryLow();        //Transmitter battery-level right now
    bool low_previous = Sensor->lowBatt;    //Transmitter battery-level last time

    if (low_now != low_previous && low_now == true) {    //Has the battery lvl has changed since last time?
        const String BATTERY_LEVEL = F("Low battery ");        //nice message        
        SMSallusers(BATTERY_LEVEL + F("at: ") + Sensor->name.get());    //send message (battery low/restored)

        Sensor->lowBatt = low_now;        //change transmitters[?].lowBatt to the other (bool) value (through pass by reference)
        Sensor->lowBattDebounce = millis();
    }
}
bool isRFbatteryLow() {
    return !(digitalRead(RF_BATT));   //check if the battery on the RF transmitter is low
}


// searches for and verifies RFID cards
bool w8forRFID(uint32_t duration) { //duration before trigger, waiting for someone to show a RFID card (milliseconds
    Serial.print(F("w8forRFID start\tduration: "));
    Serial.println(duration);
    uint32_t startTime = millis();

    while (!RFID.PICC_IsNewCardPresent() || !RFID.PICC_ReadCardSerial()) { //rfid module performs a search for nearby cards(return true if found)  &&  reads the uid of the card found to the var RFID.uid.uidByte
        if (duration <= millis() - startTime) {    //duration timed out
            Serial.print(F("w8forRFID end - "));
            Serial.println(F("false"));
            return false;
        }
        delay(20);
    }
    
    if (!(alarmdata.verifyRFID(RFID.uid.uidByte))){
        w8forRFID(duration - (millis() - startTime));   // if verification not ok recall function with the time of duration that remains
    }
    
    return true;                                        // else return true
}

//Bluetooth user interface (pretty much the same as gsmRecieveRoutine(), just different platforms (SMS/BT))
void Bluetooth_communication(uint32_t duration) {
    SoftwareSerial Bluetooth(BT_RX, BT_TX);    //Serial3 for Mega, SoftwareSerial for Uno     
    Bluetooth.begin(9600);

    Serial.println(F("in BT"));
    Serial.println((int)freeRam());
    uint32_t startTime = millis();
    String recv;    //container for Serial3/Softwareserial data recieved, which is managed
    Message BTmsg;
    while (duration >= millis() - startTime) {

        //checkInterruptFlags();          //Could have, but not necessary, as cmd "quit" exists

        recv = F("");
        while (Bluetooth.available()) {
            char c = Bluetooth.read();
            delay(5);
            if (c > (byte)127) {       //nordic and other more unusual characters
                delay(10);
                c = Bluetooth.read();
                c += 64;    //for some reason this method works :)
            }
            recv += c;
            Serial.println((int)freeRam());
        }
        if (recv == "" || recv.indexOf(F("Bluetooth Terminal ")) != -1){    // makes program ignore "Bluetooth Terminal (dis)connected"-messages, so they don't mess things up
            delay(50);
            continue;
        }
        Serial.println(recv);
        BTmsg = recv;
        if (BTmsg.command.equalsIgnoreCase(F("QUIT"))) {
            Serial.println(F("quit"));
            Bluetooth.print(F("Bluetooth communication ended"));
            break;
        }
        String btmsg2send;
        btmsg2send.reserve(160);
        Serial.println((int)freeRam());
        cmdHandler(BTmsg.command, BTmsg.args.get(), F("BT"), btmsg2send);
        Serial.println(btmsg2send);
        btmsg2send = tweakBTreturn(btmsg2send); //change to point arg?
        for (byte i = 0; i < btmsg2send.length(); i++){
            Bluetooth.print(btmsg2send[i]);
        }
        Bluetooth.print(F("\n"));
        Serial.println((int)freeRam());

    }
}


String tweakBTreturn(String &rawReturn) {   //translate nordic characters (among others)
    String BTreturn = "";        //container for message back to the sender (by Bluetooth)
    for (byte i = 0; i <= rawReturn.length(); i++) {
        if (rawReturn[i] > 127) {        //the same block as above (~line 73?), just reversed
            rawReturn[i] -= 64;
            BTreturn += (char)0;
            BTreturn += (char)195;
        }
        BTreturn += rawReturn[i];
    }

    return BTreturn;
}
