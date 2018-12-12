void setup() {
  // put your setup code here, to run once:
  pinMode(A4, OUTPUT);
  digitalWrite(A4, 1);
  delay(5000);

  blinkled(0);  

  delay(5000);

  blinkled(1);

}

void loop() {
  // put your main code here, to run repeatedly:

}

void blinkled(bool status){
    byte limit;
    status ? limit = 5 : limit = 7;
    for(int i = (int)status; i < 5 + (int)status; i++){
        digitalWrite(A4, (bool)(i % 2));  //Sätt på/stäng av en Led som visar om larmet är på eller av
        //tone(pin, toneheight, 50);
        delay(150);
    }
}
