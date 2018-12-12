#include "GSMControlMega.h"


int free_Ram()
{
	extern int __heap_start, *__brkval;
	int v;
	return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}
//########### AT-return struktur: (atcmd skickat till modul(inkl.\r(\n))) \r\n (atcmd return) \r\n \r\n (OK\ERROR) \r\n

    GSMControlMega::SMS::SMS(String raw_data){
		byte phonestringStart = raw_data.indexOf(F("\""), raw_data.indexOf(F(","))) + 1; //the first " after the first , in the string (+ 1 to get into the 0th pos in the string)
        number = raw_data.substring(phonestringStart, raw_data.indexOf(F("\""), phonestringStart)); //the first " after the start of the phonestring
		Message msg(raw_data.substring(raw_data.lastIndexOf(F("\"")) + 1));    //after all arguments returns the rest of the string (the message)
        message = msg;//går det att göra message ctor
    }

	void GSMControlMega::Message::clear(){
		command = "";
		args.clear();
	}

	void GSMControlMega::Message::assign(String newval){
		newval.trim(); //remove all whitespace chars in the beginning and end of the string
		Serial.println(F("msg trimmed"));
		if (newval.indexOf(F(":")) == -1){	//if not a write-command
			Serial.println(F("msg if"));
			command = newval;					//then no args -> only the command is needed
			Serial.println(F("msg end if"));
		}
		else{									//if a write command (we need the args too
			Serial.println(F("msg else"));
			command = newval.substring(0, newval.indexOf(F(":")));
			byte last_i = command.length() + 1;

			for (byte i = newval.indexOf(F(";"), last_i); i != 255; i = newval.indexOf(F(";"), i + 1)){//finds all the commas, one for each iteration of the loop, and adds the strings in between to the vector args														Condition of the loop is i != 255 because that is -1 in byte form
				args.add(newval.substring(last_i, i));    //we want the information between the commas (therefore -1 in arg 2)
				Serial.println(free_Ram());
				Serial.println(i);
				last_i = i + 1;									//...and last_i is one more than last i
			}

			for (byte i = 0; i <= args.size(); i++){
				args.get(i)->trim();    //if there were extra spaces in between the commas (arg1; arg2;) | arg2| -> |arg2|
			}
			Serial.println(F("msg end else"));
		}
		Serial.println(F("end Message()"));
	}
	
	void GSMControlMega::init() { // set sms text mode(+CMGF=1) and the right char table(+CSCS="GSM")...
		//Serial.begin(9600);
		//Serial.println(F("GSM ctor start"));
		Serial2.begin(9600); //115200 not available for autobauding
		delay(200);
		Serial.println("start init");
		sendATcommand(F("AT"));
		//recieveATcommand(5000);
		Serial.println(recieveATcommand(5000));
		//Serial.println(Transfer_OK(F("AT"), 5000));
		
		//if (Transfer_OK(F("AT"), 5000) != F("OK")){
		//	Serial.println("GSM init err 1");
		//	eeprom_update_byte((uint8_t*)(EEP_GSM_INIT_ERR), 255);
		//}
		//recieveATcommand(10000);
		Serial.println(recieveATcommand(20000));
		for (byte i = 0; i < 2; i++){	//GSM_INIT_CMD only has [0] & [1] & [2] ( < 3 )
			
			//if (Transfer_OK(GSM_INIT_CMD[i], 5000) != F("OK")){
			//	Serial.print("GSM init err ");
			//	Serial.println(i + 2);
			//	eeprom_update_byte((uint8_t*)(EEP_GSM_INIT_ERR + i + 1), 255);
			//}
			Serial.println(Transfer_OK(GSM_INIT_CMD[i], (uint32_t)5000));
			//Transfer_OK(GSM_INIT_CMD[i], (uint32_t)5000);
		}
		//Serial.println(F("GSM ctor end"));
	}


	bool GSMControlMega::sendSMS(const String &number, const String &message){  //send an SMS via AT+CMGS
		const String ATCMGS = F("AT+CMGS=\"");
		sendATcommand(ATCMGS + number + F("\"\r"));
		Serial.println(message);

		Serial.println((int)free_Ram());
		String data_recieved = recieveATcommand(5000);
		Serial.println((int)free_Ram());
		Serial.println(data_recieved);
		if (data_recieved.indexOf(F(">")) != -1){
			Serial.println((int)free_Ram());
			sendATcommand(message);
			Serial.println((int)free_Ram());
			Serial2.print((char)26);//END_OF_SMS);
			Serial.print(recieveATcommand(15000));	//###############returnvalue
			//message += (char)END_OF_SMS;
			//if (Transfer_OK(message, 15000) == F("OK")){
			//	Serial.println("ok2");
			return true;
			//}
			eeprom_update_byte((uint8_t*)(EEP_GSM_SENDSMS_ERR2), 255);		//EEPROM error EEP_GSM_SENDSMS_ERR2'
			delay(10);	//for EEPROM write
		}
		eeprom_update_byte((uint8_t*)(EEP_GSM_SENDSMS_ERR1), 255);		//EEPROM error EEP_GSM_SENDSMS_ERR1
		return false;//(data_recieved.startsWith(F("+CMGS:")) && data_recieved.endsWith(F("OK"))); //<CR><LF><returnval><CR><LF> 
	}
	String GSMControlMega::recieveSMS(const bool deletesms){   //recieve and read SMS   +CMTI: "SM",int location    AT+CMGR=location        if delete=true deleteSMS() will be called and the SMS will be read and then deleted 
		//use as SMS sms(recieveSMS(..)) as return val is fitted for that purpose
		String data_recieved = recieveATcommand(15000);
		data_recieved.trim();
		if (data_recieved.indexOf(F("+CMTI:")) != -1){
			String smsPos = data_recieved.substring(data_recieved.indexOf(F(",")) + 1, data_recieved.length()); // takes the value val from <CR><LF>+CMTI:"SM",val<CR><LF>
			Serial.print(F("smsPos: "));
			Serial.println(smsPos);

			const String READCMD = F("AT+CMGR=");
			String recievedsms = Transfer_CMD(READCMD + smsPos, 15000);        //w8 15 sec to recieve all incoming data, which is processed in the ctor of SMS

			//recieveATconfirm() KOLLA OM CMGR returnar fÃ¶rst sms sen ok eller om allt kommer i ett svar
			if (deletesms == true){
				deleteSMS(smsPos);
			}
			if (recievedsms != F("")){
				return recievedsms;
			}
		}
		return "";    //if no sms recieved return null
		/*
		Serial.println("recSMS start");
		
		String data_recieved = recieveATcommand(10000);	//recieve data from gsm module
		Serial.println((int)free_Ram());
		Serial.println(data_recieved);
		data_recieved.trim();
		Serial.println(data_recieved);
		if (data_recieved.indexOf(F("+CMTI:")) != -1){
			Serial.println("recSMS past if");
			String smsPos = data_recieved.substring(data_recieved.indexOf(F(",")) + 1, data_recieved.length()); // takes the value val from <CR><LF>+CMTI:"",val<CR><LF>

			const String READCMD = F("AT+CMGR=");
			String recievedsms = Transfer_CMD(READCMD + smsPos, 15000);        //w8 15 sec to recieve raw SMS (sms + much more info)

			if (deletesms == true){
				if (deleteSMS(smsPos) == false){
					eeprom_update_byte((uint8_t*)(EEP_GSM_DELSMS_ERR), 255);		//EEPROM error EEP_GSM_DELSMS_ERR
				}
			}
			if (recievedsms == F("")){
				eeprom_update_byte((uint8_t*)(EEP_GSM_READSMS_ERR), 255);		//EEPROM error - nothing read from CMGR	EEP_GSM_READSMS_ERR
			}
			return recievedsms;
		}
		//eeprom_update_byte((uint8_t*)(EEP_GSM_RECSMS_ERR), 255);		//EEPROM error no EEP_GSM_RECSMS_ERR
		Serial.println("recSMS end");
		return "";    //if no sms recieved return null
		*/
	}

	bool GSMControlMega::deleteSMS(const String &location){ //delete an SMS via AT command    location-the location in memory
		const String DELETECMD = F("AT+CMGD=");
		return (Transfer_CMD(DELETECMD + location, 5000) == F("OK"));    //send the AT delete command
    }


	String GSMControlMega::Transfer_OK(const String &ATcommand, const uint32_t timeout){ //send an AT command to the GSM module  then, recieve either the "OK" from the GSM module(return true) or the "ERROR" (or anything else)(return false)
		sendATcommand(ATcommand);

        String data_recieved = recieveATcommand(timeout);
        
		return searchEndCommand(data_recieved);		//returns the end command if found, else empty string
    }
	String GSMControlMega::Transfer_CMD(const String &ATcommand, const uint32_t timeout){  //recieve the AT return value (or command) as a string. If any error the returnvalue is that errorcmd
		sendATcommand(ATcommand);

		String recieveStr = recieveATcommand(timeout);
		String endCommand = searchEndCommand(recieveStr);

		if (endCommand != "OK"){
			return endCommand;
		}
		recieveStr.trim();	//trim away all whitespace chars at both ends of the string
		recieveStr = recieveStr.substring(ATcommand.length() + 1, recieveStr.length() - 2); //removes the echo of the sendcommand and the "OK" at the end
		recieveStr.trim();	//trim away all whitespace chars at both ends of the string
		if (recieveStr == ""){		//if the command only returns the endcommand (no variables etc.)
			return endCommand;
		}
		return recieveStr;
	}
    
	void GSMControlMega::sendATcommand(const String ATcommand){
		Serial2.flush();
		byte len = ATcommand.length();
		for (byte i = 0; i < len; i++){
			Serial2.print(ATcommand[i]);
			delay(5);
		}
		Serial2.print(F("\r"));    //added a carriage return to indicate for GSM module that the command ends here
	}

	String GSMControlMega::recieveATcommand(const uint32_t timeout){
		String ATreturn;
		uint32_t starttime = millis();
		//Serial2.flush();  (flushes away new arrived data?)               //flush the buffer
		do {
			while (Serial2.available()){    //now that we got an answer, we wanna read all data, till the stream ends
				ATreturn += (char)Serial2.read();    //append the new data to returnstring
			}
		} while (millis() - starttime < timeout && searchEndCommand(ATreturn) == "");

		return ATreturn;
	}
	String GSMControlMega::searchEndCommand(String &searchstr){
		searchstr.trim();	//trim away whitespace chars in end & beginning of string

		for (byte i = 0; i <= 3; i++){    //RX_ENDCOMMANDS.length -> 4		sista loopen -> [3]	därför i <= 3
			int commandPos = searchstr.indexOf(RX_ENDCOMMANDS[i]);//variable to be able to use index in two places without calling func again
			if (commandPos != -1)                                                                    //if an endcommand is found
				return searchstr.substring(commandPos, searchstr.length());//return entire endcommand(for cme,cms errors with return variables)
		}
		return "";
	}
	