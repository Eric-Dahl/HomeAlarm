#include <GSMControl.h>
#include <AlarmData.h>
#include <SoftwareSerial.h>

//using namespace GSMControl;

SoftwareSerial mySerial(8, 9);
//GSMControl at;
AlarmData alarmdata;

volatile bool interrupt = 0;

String str;

int freeRam () 
{
  extern int __heap_start, *__brkval; 
  int v; 
  //Serial.println(F("*"));
  //Serial.println((int)&__heap_start);
  //Serial.println((int)__brkval);
  //Serial.println((int)&v);
  //Serial.println((int)SP);
  //Serial.println((int)RAMEND);                    2303
  //Serial.println((int)RAMSTART);                  256
  //Serial.println(((int)RAMEND-(int)RAMSTART));    2047

  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
    byte rfid1[4] = {1, 1, 1, 1};

void setup() {
    Serial.begin(9600);
    mySerial.begin(9600); //115200 not available for autobauding


    Serial.println((int)freeRam());
    String args1[3] = {F("Elin"), F("+46725432411"), F("0")};
    User client(args1, rfid1);
    
    alarmdata.users.add(client);
    
    Serial.println(alarmdata.listUsers());
    
    
    Serial.println(recieveATcommand(15000));    //tar emot Call/SMS Ready...
    //GSMControl::init();
    //mySerial.flush();
    String INIT = F("INIT (");
    for (int i = 0; i < 2; i++){
        Serial.print(INIT + GSMControl::GSM_INIT_CMD[i] + F("): "));
        String RECIEVE = F("<recieve>\n");
        String recieve = Transfer_CMD(GSMControl::GSM_INIT_CMD[i], 5000);
        delay(50);
        Serial.println(RECIEVE + recieve + F("\n</recieve>"));
    }
    
    Serial.println(F("start"));
        
    /*
    const String SENDCMD = F("AT+CMGS=\"");
    String message = F("Testing...");
    if(sendSMS(F("+46739665891"), message)){
        Serial.println(F("true"));
    }
*/
    attachInterrupt(1, InterruptServiceRoutine, FALLING);
}

void InterruptServiceRoutine(){
    interrupt = true;
}

void loop() {
    
    if(interrupt == 1){
        gsmRecieveRoutine();
        interrupt = 0;
        Serial.print(F("***************************"));
    }
    //test_cmdHandler();
}

void logStr(String original){
  for(int i = 0; i <= original.length(); i++){
    Serial.print((char)original[i]);
    Serial.print("\t");
    Serial.println((byte)original[i]);
  }
}

void test_cmdHandler(){
    bool flag = 0;
    while(Serial.available()){
        flag = 1;
        str += (char)Serial.read();
        delay(20);
    }
    if(flag == 1){
        GSMControl::Message msg(str);
        String sendmsg;
        sendmsg.reserve(185);
        cmdHandler(msg.command, msg.args.get(), "EricD", sendmsg);
        Serial.println(sendmsg);
        str = "";
    }
}


