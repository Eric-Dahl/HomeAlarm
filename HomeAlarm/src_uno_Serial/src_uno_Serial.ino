#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h> //for BT
#include <avr/sleep.h>                  
#include <avr/eeprom.h> //reference - http://www.nongnu.org/avr-libc/user-manual/group__avr__eeprom.html#ga0ebd0e867b6f4a03d053801d3508f8de

#include <AlarmData.h>
#include <GSMControlUno.h>

    
//*** IMPORTANT ***
    /*
    * For use with Mega
    *      SoftwareSerial with Serial3 (RFID_RF_BT) and
    *      Serial with Serial2 (GSMControl.cpp)
    * something more???
    */

//*****NOTES*****
/*
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

//***** BOARD PINS *****
/*
Serial (GSM RX/TX)  0,1
GSM interrupt       2  (INT0)
Motion interrupt    3  (INT1)   */
#define RFID_RST    4           //not connected, though
#define RF_D3       5           //data from HT12D(RF reciever decoder) D3 pin
#define RF_D2       6           //data from HT12D(RF reciever decoder) D2 pin
#define RF_D1       7           //data from HT12D(RF reciever decoder) D1 pin
#define RF_BATT     8           //data from HT12D(RF reciever decoder) D0 pin
#define ALARMSTATE_LEDPIN 9     //to board led, tells alarm state
#define RFID_SS     10          //SPI SS
//#define RFID_MOSI 11          //SPI MOSI   no need to define, therefore commented
//#define RFID_MISO 12          //SPI MISO
//#define RFID_SCK  13          //SPI clock
#define BT_TX       A1          //Bluetooth softserial pins
#define BT_RX       A2           
#define FRONTDOORPIN A5         //read motion sensor on RFID-module


    //flags checked in checkInterruptFlags(), the root of the sketch (see loop())
    volatile int ISRmotionDetectedFlag = 0; // !0  > starts motionDetectedRoutine()     is set !0 by Motion interrupt (ISRmotionDetected())
    volatile bool ISRgsmFlag = false;       //true > starts gsmRecieveRoutine()         is set true by GSM interrupt  (ISRgsm())
    bool BT_Active = false;                 //true > starts Bluetooth_communication()

    using namespace GSMControlUno;

    AlarmData alarmdata;
    MFRC522 RFID(RFID_SS, RFID_RST);

    int freeRam(){
        extern int __heap_start, *__brkval;
        int v;
        return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
    }

    void setup() {
        Serial.begin(9600);

        GSMControlUno::init();
        SPI.begin();            // Init SPI bus
        RFID.PCD_Init();     // Init MFRC522
        Serial.println((int)freeRam());

        pinMode(RF_BATT, INPUT);
        pinMode(RF_D1, INPUT);
        pinMode(RF_D2, INPUT);
        pinMode(RF_D3, INPUT);
        pinMode(ALARMSTATE_LEDPIN, OUTPUT);
        pinMode(FRONTDOORPIN, INPUT);

        eeprom_read_block((void*)&alarmdata, (void*)20, sizeof(alarmdata));

        String username = alarmdata.users.get(0)->name.get();   //add trim()
        username.trim();
        String userphone = alarmdata.users.get(0)->phone.get();
        userphone.trim();

        Serial.println(!username.equals("Peter"));
        Serial.println(!userphone.equals("+46739665891"));
        Serial.println(alarmdata.users.get(0)->hasRestrictedAccess != false);
        
        if (!username.equals(F("Peter")) || !userphone.equals(F("+46739665891")) || alarmdata.users.get(0)->hasRestrictedAccess != false){    //no data in database then EEPROM reset + add admin to database, .get() means extra \0\0 at the end if string is not filled up entirely
            EEPROM_clear();
            alarmdata.users.clear();
            alarmdata.transmitters.clear();

            String args1[3] = { "Peter", "+46739665891", "0" };
            byte rfid1[4] = { 0x1D, 0xF8, 0x53, 0xC5 };
            User admin(args1, rfid1);

            alarmdata.users.add(admin);
        }

        Serial.println(alarmdata.listUsers());
        Serial.println(alarmdata.listTransmitters());

        attachInterrupt(1, ISRmotionDetected, RISING); //0 = pin D2  1 = pin D3
        attachInterrupt(0, ISRgsm, FALLING);
    }

    void loop() {    //The loop 
        Serial.println(F("loop!"));
        checkInterruptFlags();
        Serial.println(F("Sleep mode!"));
        delay(100);
        sleepMode();
    }

    void setAlarm(bool state){    //Turn alarm on/off
        alarmdata.setState(state);
        Serial.print(F("Led: "));
        Serial.println((int)state);
        setLed(state);
        EEPROM_update();
    }

    void sleepMode(){
        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        noInterrupts();          // make sure we don't get interrupted before we sleep
        sleep_enable();          // enables the sleep bit in the mcu register
        //attachInterrupt (0, wake, LOW);  // wake up on low level    (The other interrupts replace this)
        interrupts();           // interrupts allowed now, next instruction WILL be executed
        sleep_cpu();            // here the device is put to sleep
    }

    void EEPROM_update(){
        eeprom_update_block((const void*)&alarmdata, (void*)20, sizeof(alarmdata));
    }
    void EEPROM_clear(){
        for (int addr = 0; addr < 1024; addr++){    //EEPROM size 1024 bytes
            while (!eeprom_is_ready()){} //w8 until eeprom is ready for next write
            eeprom_update_byte((uint8_t*)addr, 0);  //write 0 to byte in EEPROM (loops)
        }
    }
