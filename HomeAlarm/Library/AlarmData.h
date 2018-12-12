#ifndef AlarmData_H
#define AlarmData_H

#ifndef __AVR_ATmega328P__
#define __AVR_ATmega328P__
#endif
#ifndef Arduino_h
#include "Arduino.h"
#endif

#include "Avec.h"
#include "Avec.cpp"

#include "WString.h"	//only for IntelliSense purposes 
//#define byte unsigned char	//only for IntelliSense purposes 

class User{
public:

	Astring<6> name; //ex. Eric
	Astring<13> phone;
	bool hasRestrictedAccess;    //if this is set true the user has no phone access when extendedAccess == false
	byte RFID[4] = { 255, 255, 255, 255 };

	User() : hasRestrictedAccess(1){}
	User(String args[], byte tempRFID[]);		//args - name, phone, hasRestrictedAccess	tempRFID - RFID[4]
	User(const User& rval) :
		name(rval.name),
		phone(rval.phone),
		hasRestrictedAccess(rval.hasRestrictedAccess),
		RFID{ rval.RFID[0], rval.RFID[1], rval.RFID[2], rval.RFID[3] } {}	//copy constructor

	User& operator =(const User& rval){
		name = rval.name;
		phone = rval.phone;
		hasRestrictedAccess = rval.hasRestrictedAccess;
		for (int i = 0; i < 4; i++) { RFID[i] = rval.RFID[i]; }
		return *this;
	}
    
	String toString() { return (String)name.get() + (String)F(" (") + (String)phone.get() + (String)(hasRestrictedAccess ? F("), \"1\"\n") : F("), \"0\"\n")); }
};

class RFTransmitter{
public:

	Astring<10> name;    //ex.garage

	bool lowBatt; //when a "low battery" SMS is sent this becomes true, prevents sms spamming
	uint32_t lowBattDebounce;//when the battery has just gone low the value may flicker between high and low (3 times within some minutes when tested). This clause is to prevent this flicker from annoying messages (batt low/restored/low/restored...
	bool D1; //the transmitter's ID ( (in order) D1, D2, D3 config on the transmitter's HT12E ) (D0 is batt. indicator)
	bool D2;
	bool D3;

	RFTransmitter(String *args);
	RFTransmitter() : lowBatt(0), lowBattDebounce(0), D1(1), D2(1), D3(1){}
	RFTransmitter(const RFTransmitter& rval) : 
		name(rval.name),
		lowBatt(rval.lowBatt), 
		lowBattDebounce(rval.lowBattDebounce),
		D1(rval.D1), D2(rval.D2), D3(rval.D3)
	{}

	RFTransmitter& operator =(const RFTransmitter& rval){
		name = rval.name;
		lowBatt = rval.lowBatt;
		lowBattDebounce = rval.lowBattDebounce;
		D1 = rval.D1;	D2 = rval.D2;	D3 = rval.D3;
		return *this;
	}

	String toString(){ return (String)name.get() + (String)F(", (") + (String)D1 + (String)D2 + (String)D3 + (String)F(") (") + (String)lowBatt + (String)F(")\n"); }

private:
	
	
};

class AlarmData
{
	private:
		bool alarmstate;
		bool extendedAccess; //all users have access to set the alarm
    public:

		AlarmData() : alarmstate(0), extendedAccess(0) {}				//default constructor
		AlarmData(const AlarmData &rval) : alarmstate(rval.alarmstate), extendedAccess(rval.extendedAccess), users(rval.users), transmitters(rval.transmitters) {}   // copy constructor

		AlarmData& operator =(const AlarmData& rval){
			alarmstate = rval.alarmstate;
			extendedAccess = rval.extendedAccess;
			users = rval.users;
			transmitters = rval.transmitters;
			return *this;
		}

		const byte ADMIN = 0; //users[ADMIN]	(users[0] is admin)


//___________________________________________setters, getters, is.._____________________________________________________

		void setState(const bool alarmstate){ this->alarmstate = alarmstate; }; //true = on, false = off
		bool getState() const { return alarmstate; } 	//return alarmstate
		void setExtendedAccess(bool setter){ extendedAccess = setter; } //set the var extendedAccess
		bool isExtendedAccess() { return extendedAccess; }

//_______________________________________________Datavectors_____________________________________________________

	Avec<User, 8> users; //stores those who have access to the alarm (can turn it on/off)

		int getUserpos(String username); //searches for user[x].name that equals name		returns x
        String listUsers(); //>loop users.toString
        

	Avec<RFTransmitter, 5> transmitters; //the motion transmitter modules connected to the alarm via radio frequency

		int getTransmitterpos(String transmittername);	//searches for trans[x].name that equals name		returns x
        String listTransmitters(); //>loop transmitters.toString                           

//________________________________________verification, matching_____________________________________________________

        int verifyPhone(String number); //match with users[?].phone and returns the position in vector users, if no match return -1         
													
        bool verifyRFID(byte RFID[10]); //match it w/ users[?].RFID and return true if there is a match, otherwise false

        int matchRFdata(bool *ID); //returns the position in vector transmitters, if no match return -1  

    protected:

};

#endif // AlarmData_H

//uint64_t max-18,446,744,073,709,551,615