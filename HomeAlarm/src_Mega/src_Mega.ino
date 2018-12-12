#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h> //for BT
#include <avr/sleep.h>                  
#include <avr/eeprom.h> //reference - http://www.nongnu.org/avr-libc/user-manual/group__avr__eeprom.html#ga0ebd0e867b6f4a03d053801d3508f8de

#include <AlarmData.h>
#include <GSMControlMega.h>

/*
 *** IMPORTANT ***
 * For use with Uno
 *      Serial3 with SoftwareSerial (RFID_RF_BT) and 
 *      Serial2 with Serial (GSMControl.cpp)
 * something more???
 * 
         issues:
            -response time for GSM module & standard values for AT response functions  (works anyway...)
            -AT+CSQ-command???                                                         (works anyway...)
            
         hardware issues:
                    SOLVED (at least working)//GSM pins Vhigh = 2.8V - http://wiki.groundlab.cc/doku.php?id=microcontrollersgsm - höj med RTL-krets, sänk med zener(som i länk)
                    SOLVED//GSM can recieve SMS in sleep mode (DTR pin) Will it interrupt and issue too? <---- SOLVED by not using DTR pin & sleep mode (overvoltage warning when entering sleep mode)
            
         TODO:
            !!!!!TESTA!!!!!! - batt. indicator, 
                //TESTAT: transmit-recieve, RFID, vector(?), GSM, motion transmitter distance/sensitivity, transmit-recieve-distance (fungerar med antenn),
            -mega ver, uno ver                  CHECK!
            -finish command guide (cmd = "?")   CHECK!
            -ändra pins till bara defines       CHECK!
            
         TODO: (non-programming)
            -update eagle files                 NOPE
            -Skriv om highlightning-dokument    CHECK!
            -löd ihop moduler                   CHECK!

    transmitter battery life
        PIR         - ~65mA                  standby 50uA                    https://www.mpja.com/download/31227sc.pdf
        Batt Led    - ~20mA
        transmitter - ~20-40mA (5v, 9V)      standby 10uA
        HT12e       - 80-300mA (3V, 12V)     standby 0,1uA-2uA               https://people.ece.cornell.edu/land/courses/ece4760/FinalProjects/s2008/cl457_yft2/cl457_yft2/datasheets/HT12E.pdf
        battlvl (D0)-                                500uA

        TOTAL       - 150-400mA/trigger      standby ~700uA@6V-2,5mA@9V

        550mAh@9V batt -> 50h operating time (without triggers)

    eeprom cmd:s
      //eeprom_read_block(void * __dst, const void * __src, size_t __n); //Read a block of __n bytes from EEPROM address __src to SRAM __dst.
      //eeprom_update_block(const void * __src, void * __dst, size_t __n);    //Update a block of __n bytes to EEPROM address __dst from __src.
      //eeprom_update_byte((uint8_t*)1023, 99);
            
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

using namespace GSMControlMega;

AlarmData alarmdata;
MFRC522 RFID(10, 6);//(RFID_SS, RFID_RST);

int freeRam (){
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void setup() {
    Serial.begin(9600);
    GSMControlMega::init();
    SPI.begin();            // Init SPI bus
    RFID.PCD_Init();     // Init MFRC522
    Serial.println((int)freeRam());

    pinMode(A0, INPUT);
    pinMode(A1, INPUT);
    pinMode(A2, INPUT);
    pinMode(A3, INPUT);
    pinMode(A4, OUTPUT);
    pinMode(A5, INPUT);
    
    //eeprom_read_block((void*)&alarmdata, (void*)0, sizeof(alarmdata));
    
    if(alarmdata.users.isEmpty() && alarmdata.transmitters.isEmpty()){    //no data in database then EEPROM reset
        EEPROM_clear();

        String args1[3] = {"Peter", "+46739665891", "0"};
        byte rfid1[4] = {0x1D, 0xF8, 0x53, 0xC5};
        User admin(args1, rfid1);
    
        alarmdata.users.add(admin);
    }
    
    String args1[3] = {"Johan", "+46700139449", "0"};
    byte rfid1[4] = {0x1D, 0xF8, 0x53, 0xC5};
    User me(args1, rfid1);

    String sensargs[4] = {"Vardagsrum", "0", "1", "1"};
    RFTransmitter mytransmitter(sensargs);

    alarmdata.users.add(me);
    alarmdata.transmitters.add(mytransmitter);
    
    Serial.println(alarmdata.listUsers());
    Serial.println(alarmdata.listTransmitters());

    attachInterrupt(4, ISRmotionDetected, RISING); //0 = pin D2  1 = pin D3
    attachInterrupt(5, ISRgsm, FALLING); //##########Falling?###########
}

void loop() {    //The loop 
  Serial.println(F("loop!"));
  Serial.println((int)freeRam());
  checkInterruptFlags();
  Serial.println((int)freeRam());
  Serial.println(F("Sleep mode!"));
  delay(100);
  sleepMode();
}

void setAlarm(bool state){    //Turn alarm on/off
  alarmdata.setState(state);
  //Serial.print(F("Led: "));
  //Serial.println((int)state);
  setLed(state);
  EEPROM_update();
}

void sleepMode(){
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);   
  noInterrupts ();          // make sure we don't get interrupted before we sleep
  sleep_enable ();          // enables the sleep bit in the mcu register
  //attachInterrupt (0, wake, LOW);  // wake up on low level    (The other interrupts replace this)
  interrupts ();           // interrupts allowed now, next instruction WILL be executed
  sleep_cpu ();            // here the device is put to sleep
}

void EEPROM_update(){
    eeprom_update_block((const void*)&alarmdata, (void*)0, sizeof(alarmdata));
}
void EEPROM_clear(){
    for(int addr = 0; addr < 1024; addr++){    //EEPROM size 1024 bytes
        while(!eeprom_is_ready()){} //w8 until eeprom is ready for next write
        eeprom_update_byte((uint8_t*)addr, 0);  //write 0 to byte in EEPROM (loops)
    }
}








