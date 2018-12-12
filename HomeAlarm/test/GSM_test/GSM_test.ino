#include <SoftwareSerial.h>
//317
void setup() {
  // put your setup code here, to run once:
  SoftwareSerial softSer(2, 3);
  softSer.begin(9600);
  Serial.begin(9600);
  softSer.print("AT+CPIN=\"4074\";+CLCK=\"AI\",1;+CMGF=1;+CSCS=\"GSM\"");
  String ATreturn;
   while(!softSer.available()){                //w8 for serial data from gsm module
            
        }
        while(softSer.available()){    //now that we got an answer, we wanna read all data, till the stream ends
            ATreturn += softSer.read();    //append the new data to returnstring
        }
  Serial.print("done: " + ATreturn);
        
  //smssender.sendSMS("+46725432411", "Hej, Elin! :)");
}

void loop() {
  // put your main code here, to run repeatedly:

}
