
bool sendSMS(String number, String message){  //send an SMS via AT+CMGS
		const String ATCMGS = F("AT+CMGS=\"");
		sendATcommand(ATCMGS + number + F("\"\r"));
                
                String data_recieved = recieveATcommand(5000);
                if(data_recieved.indexOf(">") != -1){
                    if(Transfer_OK(message + (char)26 + F("\r"), 5000) == "OK"){
                        return true;
                    }
                }
		return false;//(data_recieved.startsWith(/*(\r\n)* /F("+CMGS:")) && data_recieved.endsWith(F("OK")/*(\r\n)* /)); //<CR><LF><returnval><CR><LF> 
    }
    String recieveSMS(bool deletesms){   //recieve and read SMS   +CMTI: "SM",int location    AT+CMGR=location        if delete=true deleteSMS() will be called and the SMS will be read and then deleted
        
        String data_recieved = recieveATcommand(15000);
        data_recieved.trim();
        if(data_recieved.indexOf(F("+CMTI:")) != -1){
            String smsPos = data_recieved.substring(data_recieved.indexOf(F(",")) + 1, data_recieved.length()); // takes the value val from <CR><LF>+CMTI:"SM",val<CR><LF>
            Serial.print(F("smsPos: "));
            Serial.println(smsPos);
            
	    const String READCMD = F("AT+CMGR=");
	    String recievedsms = Transfer_CMD(READCMD + smsPos, 15000);        //w8 15 sec to recieve all incoming data, which is processed in the ctor of SMS
            
//recieveATconfirm() KOLLA OM CMGR returnar fÃƒÂ¶rst sms sen ok eller om allt kommer i ett svar
            if(deletesms == true){
                deleteSMS(smsPos);
            }
    	    if (recievedsms != F("")){
              	return recievedsms;
    	    }
        }
        return "";    //if no sms recieved return null
    }
    bool deleteSMS(String location){ //delete an SMS via AT command    location-the location in memory
		  const String DELETECMD = F("AT+CMGD=");
		  return (Transfer_OK(DELETECMD + location, 5000) == F("OK"));    //send the AT delete command
    }


String Transfer_OK(String ATcommand, uint32_t timeout){ //send an AT command to the GSM module  then, recieve either the "OK" from the GSM module(return true) or the "ERROR" (or anything else)(return false)
		sendATcommand(ATcommand);

                String data_recieved = recieveATcommand(timeout);
        
		return searchEndCommand(data_recieved);		//returns the end command if found, else empty string
    }
    
//########### AT-return struktur: (atcmd skickat till modul(inkl.\r(\n))) \r\n (atcmd return) \r\n \r\n (OK\ERROR) \r\n

	String Transfer_CMD(String ATcommand, uint32_t timeout){  //recieve the AT return value (or command) as a string. If any error the returnvalue is that errorcmd
		sendATcommand(ATcommand);

		String recieveStr = recieveATcommand(timeout);
                Serial.println(recieveStr);
		String endCommand = searchEndCommand(recieveStr);
                        
                Serial.print(F("endcommand: "));
                Serial.println(endCommand);
		if (endCommand != "OK"){
		    return endCommand;
		}
		recieveStr.trim();	//trim away all whitespace chars at both ends of the string
		recieveStr = recieveStr.substring(ATcommand.length() + 1, recieveStr.length() - 2); //removes the echo of the sendcommand and the "OK" at the end
		recieveStr.trim();	//trim away all whitespace chars at both ends of the string
		if (recieveStr == ""){
	            return endCommand;
		}
		return recieveStr;
    }
    
	
	String recieveATcommand(uint32_t timeout){
		String ATreturn;
		uint32_t starttime = millis();
		//Serial.flush();  (flushes away new arrived data?)               //flush the buffer
    char c;
		do {
      while (mySerial.available()){    //now that we got an answer, we wanna read all data, till the stream ends
        c = (char)mySerial.read();    //append the new data to returnstring
        Serial.print((char)c);
        Serial.print("\t");
        Serial.println((byte)c);
        ATreturn += (String)c;
      }
    } while(millis() - starttime < timeout && searchEndCommand(ATreturn) == "");
    
		return ATreturn;
	}
void sendATcommand(String ATcommand){
                /*for(int i = 0; i < ATcommand.length(); i++){
		    mySerial.write(ATcommand[i]);    //added a carriage return to indicate for GSM module that the command ends here
                }*/
                mySerial.print(ATcommand + F("\r"));
	}
	String searchEndCommand(String searchstr){
                searchstr.trim();
		for (int i = 0; i <= 3; i++){    //RX_ENDCOMMANDS.length - 4		sista loopen - [3]	dÃ¤rfÃ¶r i <= 3
			int commandPos = searchstr.indexOf(GSMControl::RX_ENDCOMMANDS[i]);    //    variable to be able to use the index in two places without calling func again
			if (commandPos != -1)                                                                    //if an endcommand is found
				return searchstr.substring(commandPos, searchstr.length());    //return the entire endcommand(for cme,cms errors with return variables)
		}
            return "";
	}

void gsmRecieveRoutine() {
    
  //if (isSMS() == true) {
    String smsstring = recieveSMS(false);
    String SMSSTRING = F("smsstring: ");
    Serial.println(SMSSTRING + smsstring);
    
    if (smsstring != F("")){    //if the sms was recieved (if no error)
      GSMControl::SMS recievedsms(smsstring);     //make it an SMS object
      Serial.print(F("phone: "));
      logStr(recievedsms.number);
      int vectorpos = alarmdata.verifyPhone(recievedsms.number);    //Check if the sender is a client
      String printstr0 = F("vectorpos: ");
      if(vectorpos != -1){
          Serial.println(printstr0 + vectorpos);
      }
      printstr0 = F("Msg cmd: ");
      Serial.println(printstr0);
      logStr(recievedsms.message.command);
      Serial.println((int)freeRam());
      String printstr1 = F("msg args [");
      for(int i = 0; i < 4; i++){
          Serial.println(printstr1 + (String)i + "]: ");
          logStr(*recievedsms.message.args.get(i));
      }
        
      if(vectorpos != -1){    //if the sender is a client
          User* sender = alarmdata.users.get(vectorpos);    //Who sent the message?
          String msgtosend;
          msgtosend.reserve(185);
          cmdHandler(recievedsms.message.command, recievedsms.message.args.get(), sender->name, msgtosend); //call the cmdHandler to get a message to return to the sender
          
          String printstr2 = sender->name + F(" sent this message\nmsg2send (back): ") + msgtosend;
          Serial.println(printstr2);
          if(sendSMS(recievedsms.number, msgtosend)){
              Serial.println(F("Message returned"));
          }
//          EEPROM_update();
      }
    }  
  //}
}

