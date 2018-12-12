#ifndef GSMControlMega_H
#define GSMControlMega_H

#ifndef __AVR_ATmega2560__
#define __AVR_ATmega2560__
#endif

#if defined(__AVR_ATmega2560__)

#ifndef Arduino_h
#include "Arduino.h"
#endif

#include "Avec.h"
#include "Avec.cpp"

#include <avr/pgmspace.h>
#include <avr/eeprom.h> //reference - http://www.nongnu.org/avr-libc/user-manual/group__avr__eeprom.html#ga0ebd0e867b6f4a03d053801d3508f8de
#include "WString.h"	//only for IntelliSense purposes 
#include "SoftwareSerial.h"



namespace GSMControlMega {

	#define END_OF_SMS 26

	#define EEP_GSM_INIT_ERR 1000	//at eeprom address no.1000-1002, the error mapping starts (errors within the GSMControl ctor)
	#define EEP_GSM_SENDSMS_ERR1 1003	//address 1003 - AT+CMGS fail
	#define EEP_GSM_SENDSMS_ERR2 1004	//address 1004 - could not send message
	#define EEP_GSM_RECSMS_ERR 1005		//address 1005 - no CMTI: cmd recieved
	#define EEP_GSM_READSMS_ERR 1006	//address 1006 - AT+CMGR fail
	#define EEP_GSM_DELSMS_ERR 1007		//address 1007 - AT+CMGD fail


/*
    To make use of the Message ctor your sms messages must follow the following syntax:     
    
    command:arg1;arg2;arg_n;    (command<colon>arg1<semicolon>arg2<semicolon>arg3<semicolon>)
    command: arg1; arg2; arg_n;  (command<colon><space>arg1<semicolon><space>arg2<semicolon><space>arg_n<semicolon>)      should work too
    You can max pull it to this extent - (command<colon><XXX>arg1<XXX><semicolon><XXX>semicolon<XXX><semicolon>.......<semicolon>)      
			<XXX> - <any whitespace char> - \n, \t, \r, vertical tab    all chars removed by String::trim()
    
    If you use another syntax or no syntax at all the conversion from raw message to variables will fail!
	Remember to use semicolon after the last argument too!
*/
	
	class Message{ 
	public:			//sms (message part) struct with the variables of a command message fitting this class
		Message() : command("") {}	//default constructor
		Message(String raw_data){ assign(raw_data); }//divides the raw data into command and args 
		Message(const Message &rval) : command(rval.command), args(rval.args){}	//copy constructor
		Message& operator =(const Message& rval){
			command = rval.command;
			args = rval.args;
			return *this;
		}
		Message& operator =(const String& rval){
			clear();
			assign(rval);
			return *this;
		}


		String command;
		Avec<String, 10> args;
	private:
		void clear();		//resets all variables in Message object
		void assign(String newval);	//"Fills" the object with values from string (command syntax)
	};


	class SMS{          
	public:			//SMS struct with the regular sms data
		SMS();
		SMS(String raw_data);   //put the raw data with the same syntax as AT+CMGR return value and it will be converted to number and message TIP:only use this ctor with data from AT+CMGR or such
		SMS(const SMS &rval) : number(rval.number), message(rval.message){}	//copy constructor
		SMS& operator =(const SMS& rval){
			number = rval.number;
			message = rval.message;
			return *this;
		}

		String number;
		Message message;
	};


	//GSMControl() {} //default ctor
	//GSMControl& operator =(const GSMControl& rval){}

	void init();	//set sms text mode(+CMGF=1) and the right char table(+CSCS="GSM")
    
    bool sendSMS(const String &number, const String &message);	//send an SMS via AT+CMGS
    String recieveSMS(const bool deletesms);   //recieve and read SMS   +CMTI: "SM",int location    AT+CMGR=location        if delete=true deleteSMS() will be called and the SMS will be read and then deleted
	bool deleteSMS(const String &location); //delete an SMS via AT command


	//The var timeout is the time in milliseconds to wait for GSM answer (for all functions)

	String Transfer_OK(const String &ATcommand, const uint32_t timeout); //Send an AT-command - get "OK" if successful, else error codes
	String Transfer_CMD(const String &ATcommand, const uint32_t timeout); //Send an AT-command - get the AT-returncommand, or error codes

	String recieveATcommand(const uint32_t timeout);  //timeout the same as above. recieve the un-modified AT return value (or command) as a string
    
  //private:  //comment away if you run test programs
	const String RX_ENDCOMMANDS[4] = { "OK", "+CME ERROR", "+CMS ERROR", "ERROR" };
	const String GSM_INIT_CMD[2] = { "AT+CMGF=1", "AT+CSCS=\"GSM\"" }; //"AT+CLCK=\"AI\",1" doesn't work

	void sendATcommand(const String ATcommand); //send an AT command to the GSM module  
	String searchEndCommand(String &searchstr);	//recieve either one of the strings from RX_ENDCOMMANDS (with arguments for +CME & +CMS) or "" (empty String) if none of these are found

};

#endif //defined(__AVR_ATmega2560__)

#endif //GSMControlMega_H