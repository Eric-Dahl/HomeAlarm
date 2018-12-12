#include <avr/sleep.h>

#define VTPIN 3    //HT12D:s VT pin
int counthighsignals = 0;
int countlowsignals = 0;


volatile int ISRmotionDetectedFlag = 0;

void setup() {
    //pinMode(VTPIN, INPUT);
    pinMode(A0, INPUT);
    pinMode(A1, INPUT);
    pinMode(A2, INPUT);
    pinMode(A3, INPUT);


    Serial.begin(9600);

    attachInterrupt(4, ISRmotionDetected, RISING);
}

void loop() {
    if(ISRmotionDetectedFlag != 0){
        if(digitalRead(A0) == 1){
            Serial.print("OK...");
            Serial.println(counthighsignals);
            counthighsignals++;
        }
        if(digitalRead(A0) == 0){
            Serial.print("                          Low...");
            Serial.println(countlowsignals);
            countlowsignals++;
        }
        //Serial.print(digitalRead(A0));
        //Serial.print(digitalRead(A1));
        //Serial.print(digitalRead(A2));
        //Serial.println(digitalRead(A3));
        delay(5000);
    }
    delay(1000);
    sleepMode();
}

void sleepMode(){
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);   
  noInterrupts ();          // make sure we don't get interrupted before we sleep
  sleep_enable ();          // enables the sleep bit in the mcu register
  //attachInterrupt (0, wake, LOW);  // wake up on low level    (The other interrupts replace this)
  interrupts ();           // interrupts allowed now, next instruction WILL be executed
  sleep_cpu ();            // here the device is put to sleep
}

void ISRmotionDetected(){
  sleep_disable();
  ISRmotionDetectedFlag++;
}
/*
void setup() {
    for(int i = 3; i < 7; i++){
        pinMode(i, INPUT);
    }
    Serial.begin(9600);
}

void loop() {
    for(int i = 3; i < 7; i++){
        Serial.print(digitalRead(i));
    }
    Serial.println();
            delay(200);
}
*/
