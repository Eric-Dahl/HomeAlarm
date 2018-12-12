#include <AlarmDatabase.h>

String args1[3] = {"name1", "070-111 111", "1"};
byte rfid1[4] = {1, 1, 1, 1};

AlarmDatabase al;

void setup() {
    Serial.begin(9600);
    addfunc();
    Serial.print("Kör!");
    String crasher = "reeeefjksbjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjsssssssssssssssshhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhobssssssssssssssssssssoiiiiiiiiiiiiiiiiiiiiiiiiishhh";
    String crash2 = "hhhhhhhhhhhhhhhhhhhhhhhhhhhhhheeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee";
    String crash3 = "eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeesbeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeebbbbsbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbssbpbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
    Serial.println(al.listClients());
}

void loop() {
    
}

void addfunc(){
    String args2[3] = {"name2", "070-222 222", "0"};
    byte rfid2[4] = {2, 2, 2, 2};
    ClientValidation cli1(args1, rfid1);
    al.clients.add(cli1);
    ClientValidation cli2(args2, rfid2);
    al.clients.add(cli2);
    args2[0] = "";
}
