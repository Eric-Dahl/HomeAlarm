#include <EEPROM.h>

#include <avr/eeprom.h>
#include <EEPROMAnything.h>
#include <AlarmData.h>
#include <string.h>
#include "Avec.h"

AlarmData alarmdata;
String clidata[3] = {"Peter", "+46739665891", "0"};
String clidata2[3] = {"Elin", "+46725432411", "0"};
byte rval[4] = {0x1D, 0xF8, 0x53, 0xC5};
String sampledata[4] = {"RFtransoo", "0", "1", "0"};
User client(clidata, rval);
User client2(clidata2, rval);
RFTransmitter sample(sampledata);

class TestCharArr{
public:
    Avec <char, 20> str;
    int xtra = 0x31D2;
}test;

void setup() {
  Serial.begin(9600); 
  
/*
  for (int i = 0; i < 1; i++){
      alarmdata.users.add(client);
      alarmdata.users.add(client2);
      alarmdata.transmitters.add(sample);
  }
  */
  updateData();
  //printEEPROM();
  readData();
  //Serial.println((unsigned int)alarmdata.users.get(0));
}

void readCharData(){

  eeprom_read_block((void*)&test, (void*)20, sizeof(test));

  Serial.print("str: ");
  Serial.println(test.str.get());
  Serial.println("int: " + (String)test.xtra);
}

void updateCharData(){

  char* temp = "Hello World!";
  
  //Serial.println("\n\nsize: " + (String)sizeof(&temp) + "\n\n");
  for(int i = 0; i < 13; i++){
    test.str.add(temp[i]);
  }

  clearEEPROM();

  printEEPROM();
  Serial.println("\n\nsize: " + (String)sizeof(test) + "\n\n");
  eeprom_update_block((const void*)&test, (void*)20, sizeof(test));
  printEEPROM();
}

void readData(){    //from eeprom
  //alarmdata.users.clear();
  eeprom_read_block((void*)&alarmdata, (void*)20, sizeof(alarmdata));
  while(!eeprom_is_ready()){} //w8 until eeprom is ready for next write
  //EEPROM_readAnything(0, alarmdata);

  Serial.println("\n\n\nClients:");
  Serial.println(alarmdata.listUsers());
  delay(100);
  Serial.println("\n\n\nTransmitters:");
  Serial.println(alarmdata.listTransmitters());
  Serial.println();
  Serial.print(alarmdata.users.get(0)->name.get());
  Serial.print(F(", "));
  Serial.println(alarmdata.users.get(0)->phone.get());

  //EEPROM_writeAnything(0, alarmdata);
  eeprom_update_block((const void*)&alarmdata, (void*)20, sizeof(alarmdata));
}

void updateData(){
  printEEPROM();

  for (int i = 0; i < 1; i++){
      alarmdata.users.add(client);
      alarmdata.users.add(client2);
      alarmdata.transmitters.add(sample);
  }
  Serial.println("\n\n\nClients:");
  Serial.println(alarmdata.listUsers());
  Serial.println("\n\n\nTransmitters:");
  Serial.println(alarmdata.listTransmitters());
  
  
  //EEPROM_writeAnything(0, alarmdata);
  eeprom_update_block((const void*)&alarmdata, (void*)20, sizeof(alarmdata));

  printEEPROM();
/*
  for(int i = 0; i < 3; i++){
      eeprom_update_byte((uint8_t*)(1000 + i), i);
  }
  for(int i = 1000; i < 1003; i++){
      Serial.println(eeprom_read_byte((uint8_t*)i));
  }
  */
}

void printEEPROM(){
  for(int i = 20; i < 80; i++){
      Serial.print((String)i + ":\t");
      delay(20);
      byte b = EEPROM.read(i);
      delay(20);
      Serial.print(b);
      delay(20);
      Serial.print("\t");
      Serial.println((char)b);
      delay(20);
  }
}

void clearEEPROM(){
    for(int i = 20; i < 40; i++){
      EEPROM.update(i, 0);
      delay(20);
  }
}

void loop() {
  // put your main code here, to run repeatedly:

}


/*
1
T=öFW (ïûü¿½…Š;JïÛ), "0"
ð�ôÙï («÷ÜCþ{öq]–î), "0"
•ÄÞ¦T?!é, (010) (0)

2
T<öFW (ïûü¿½…Š{JïÛ), "0"
ð‰õÙï («÷¼Cþ{öq]î), "0"
•ÄÞ¦T=)é, (010) (0)

3
t�öFW (ïûü¿½‡Š{JïÛ), "0"
ð‰õÙï («÷œÃþ{öq]^î), "0"
•ÄÞ¦T?)í, (010) (0)

4
T<öFW (ïûü¿½…Š;JïÛ), "0"
ð�õÑï («÷ÜCþ{öq]–^î), "0"
•oäÞ¦Ô=)é, (010) (0)

5
t=öFW (ïûü¿½…Š;JïÛ), "0"
ð‰õÙï («÷¼Cþ{öq]–î), "0"
•ÄÞ¦Ô?!é, (010) (0)




0:    0    
1:  0    
2:  0    
3:  102 f
4:  4   
5:  6   
6:  0    
7:  5   
8:  0    
9:  111 o
10: 4   
11: 13  

12: 0    
13: 12  
14: 0    
15: 0    
16: 121 y
17: 132 „
18: 143 �
19: 154 š
20: 127 
21: 4   
22: 6   
23: 0    
24: 5   
25: 0    
26: 136 ˆ
27: 4   
28: 13  

29: 0    
30: 12  
31: 0    
32: 0    
33: 121 y
34: 132 „
35: 143 �
36: 154 š
37: 152 ˜
38: 4   
39: 6   
40: 0    
41: 0    
42: 0    
43: 161 ¡
44: 4   
45: 13  

46: 0    
47: 0    
48: 0    
49: 1   
50: 255 ÿ
51: 255 ÿ
52: 255 ÿ
53: 255 ÿ
54: 177 ±
55: 4   
56: 6   
57: 0    
58: 0    
59: 0    
60: 186 º
61: 4   
62: 13  

63: 0    
64: 0    
65: 0    
66: 1   
67: 255 ÿ
68: 255 ÿ
69: 255 ÿ
70: 255 ÿ
71: 2   
72: 202 Ê
73: 4   
74: 10  

75: 0    
76: 9       
77: 0    
78: 0    
79: 0    
80: 0    
81: 0    
82: 0    
83: 0    
84: 1   
85: 0    
86: 215 ×
87: 4   
88: 10  

89: 0    
90: 0    
91: 0    
92: 0    
93: 0    
94: 0    
95: 0    
96: 0    
97: 1   
98: 1   
99: 1   











20:    22  
21: 67  C
22: 0    
23: 71  G
24: 7   
25: 10  

26: 0    
27: 10  

28: 0    
29: 10  

30: 6   
31: 13  

32: 0    
33: 12  
34: 0    
35: 0    
36: 121 y
37: 132 „
38: 143 �
39: 154 š
40: 84  T
41: 7   
42: 10  

43: 0    
44: 10  

45: 0    
46: 35  #
47: 6   
48: 13  













20:     0    alarmstate 
21:     0    extendedaccess
22: 200 0   ADMIN
_____________________________
23: 121 y   
24: 7   
25: 10      reserved chars name

26: 0    
27: 10  

28: 0    
29: 15    ?max chars phone?
30: 6     curr chars name?
31: 15    max chars phone
32: 0    
33: 12    curr chars phone
34: 0    
35: 93  ]  hasRestrictedAccess
36: 121 y  RFID
37: 132 „  RFID
38: 143 �  RFID
39: 154 š  RFID











*/
