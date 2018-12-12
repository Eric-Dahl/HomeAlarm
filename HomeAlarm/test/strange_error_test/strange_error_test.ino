#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h> //for BT
#include <avr/sleep.h>                  
#include <avr/eeprom.h> //reference - http://www.nongnu.org/avr-libc/user-manual/group__avr__eeprom.html#ga0ebd0e867b6f4a03d053801d3508f8de

#include <AlarmData.h>
#include <GSMControl.h>


void gsmRecieveRoutine();
/*
         issues:
            -response time for GSM module & standard values for AT response functions
            -AT+CSQ-command???
            
         hardware issues:
                    //GSM pins Vhigh = 2.8V - http://wiki.groundlab.cc/doku.php?id=microcontrollersgsm - höj med RTL-krets, sänk med zener(som i länk)
                    //GSM can recieve SMS in sleep mode (DTR pin) Will it interrupt and issue too? <---- SOLVED by not using DTR pin & sleep mode (overvoltage warning when entering sleep mode)
            
         TODO:
            !!!!!TESTA!!!!!! - batt. indicator, transmit-recieve-distance,
                //TESTAT: transmit-recieve, RFID, vector(?), GSM, motion transmitter distance/sensitivity, 
            -finish command guide (cmd = "?")
            -ändra pins till bara defines <- När alla pins fått namn, ändra i alla dokument
            
         TODO: (non-programming)
            -update eagle files
            -Skriv om highlightning-dokument
            -löd ihop moduler 
            -fixa komponenter - beställ (fler motion transmitters)
            
*/
#define ALARMSTATE_LEDPIN 4//TODO_______________________________________________________
#define RFID_RST 9
#define RFID_SS 10
//#define RFID_MOSI 11
//#define RFID_MISO 12
//#define RFID_SCK 13 //SPI clock
#define RF_BATT A0  //data from HT12D(RF reciever decoder) D0 pin
//#define RF_D1 A1    //data from HT12D(RF reciever decoder) D1 pin
//#define RF_D2 A2    //data from HT12D(RF reciever decoder) D2 pin
//#define RF_D3 A3    //data from HT12D(RF reciever decoder) D3 pin
//#define FRONTDOORPIN  //read motion transmitter on RFID-module
#define INTERRUPT_PIN 3
#define BT_RX 6
#define BT_TX 7
//byte RF_DATAPINS[3] = 
//byte GSMPINS[3] = {}; //

#define EEP_UNKNOWN_SENSOR 1008 //EEPROM adress 1008 is kept for logging if an unknown transmitter signal is recieved
#define EEP_UNKNOWN_TAG 1009    //EEPROM adress 1009 is kept for logging if anyone has scanned an unknown RFID-tag

volatile int ISRmotionDetectedFlag = 0;
volatile bool ISRgsmFlag = false;

bool BT_Active = false;

//GSMControl gsm;
using namespace GSMControl;

AlarmData alarmdata;

int freeRam () 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void setup() {
    Serial.begin(9600);
    //GSMControl::init();
 
    String args1[3] = {"Eva", "+46700872445", "0"};
    byte rfid1[4] = {0x1D, 0xF8, 0x53, 0xC5};
    User me(args1, rfid1);

    String sensargs[4] = {"Vardagsrum", "0", "1", "1"};
    RFTransmitter mytransmitter(sensargs);

    alarmdata.users.add(me);
    //delay(100);
    alarmdata.transmitters.add(mytransmitter);
    Serial.println(alarmdata.listUsers());
    Serial.println(alarmdata.listTransmitters());

    gsmRecieveRoutine();
}

void loop(){
}



void gsmRecieveRoutine() {
/*    Serial.println(F("GSMRecieveRoutine start"));
    Serial.println((int)freeRam());
    //eeprom_update_byte((uint8_t*)1021, 99);
    
    String smsstring = recieveSMS(false);
    Serial.println((int)freeRam());
    Serial.println(smsstring);
    
    if (smsstring != F("")){    //if the sms was recieved (if no error)
      Serial.println(F("SMS received"));
      Serial.println((int)freeRam());*/
      SMS recievedsms("");     //make it an SMS object
      recievedsms.number = "+46700872445";
      //recievedsms.message.command =
      Serial.println(recievedsms.number);
      int vectorpos = alarmdata.verifyPhone(recievedsms.number);    //Check if the sender is a user
      //Serial.println((int)freeRam());
      Serial.println(vectorpos);
        if(vectorpos != -1){    //if the sender is a user

            Serial.println(F("Sender verified"));
            User* sender = alarmdata.users.get(vectorpos);    //Who sent the message?
            Serial.println((int)freeRam());
            if(sender->hasRestrictedAccess == false){
                //Serial.println(F("&"));
                String message2send = F("Message recieved");
                //message2send.reserve(160);        //not even here when line 101 commented (print "&")
                Serial.println(sender->name);
                String sendername = sender->name;
                //cmdHandler(recievedsms.message.command, recievedsms.message.args.get(), sendername, message2send); //call the cmdHandler to get a message to return to the sender
                Serial.println(F("Sending..."));    //here when line 101 uncommented (print "&")
                //sendSMS(recievedsms.number, message2send);                         //send back the message
                if(vectorpos != alarmdata.ADMIN){
                    //sendSMS(alarmdata.users.get(alarmdata.ADMIN)->phone, message2send);       //send the message to the admin, too
                }
                
            }
            else{
               String tempmsg = F("No access to SMS communication with the Alarm");
               //sendSMS(recievedsms.number, tempmsg); //send back access denied the message
            }
            
        }
    
    Serial.println(F("GSMRecieveRoutine end"));
}

