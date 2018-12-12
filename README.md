# HomeAlarm
An Arduino-based home alarm with RFID, GSM and Bluetooth capabilities. More features are remote motion sensors and a database that stores all user info if the power should go off.

# Arduino Code
The end product was designed for UNO, but during development both UNO and MEGA were used, hence the src_uno, src_mega and src_unoSerial folders. All of them should work, the difference between src_uno and src_unoSerial is that in order to use Serial in the latter the GSM communication (originally on pins 0, 1) had to be moved.

# Classes (.cpp) and (.h)
In the Library folder there are a couple of own-written helper classes that the arduino code uses. 

# Sensors and Modules
All Sensors and Modules to make this project
GSM         GSM800
RFID        RFID-RC522
Bluetooth   HC-05
Motion sensor               HC-SR501
RF transmitter/reciever     Any compatible pair
Logic encoder/decoder       HT12E/HT12D
