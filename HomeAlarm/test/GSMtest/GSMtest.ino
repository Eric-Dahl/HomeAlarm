/*
  Software serial multple serial test

 Receives from the hardware serial, sends to software serial.
 Receives from software serial, sends to hardware serial.

 The circuit:
 * RX is digital pin 10 (connect to TX of other device)
 * TX is digital pin 11 (connect to RX of other device)

 Note:
 Not all pins on the Mega and Mega 2560 support change interrupts,
 so only the following can be used for RX:
 10, 11, 12, 13, 50, 51, 52, 53, 62, 63, 64, 65, 66, 67, 68, 69

 Not all pins on the Leonardo support change interrupts,
 so only the following can be used for RX:
 8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI).

 created back in the mists of time
 modified 25 May 2012
 by Tom Igoe
 based on Mikal Hart's example

 This example code is in the public domain.*/

 
#include <SoftwareSerial.h>
#include <ATCommands.h>

SoftwareSerial mySerial(10, 11); // RX, TX
ATCommands at;


volatile bool isRING = false;
bool seravailable = false;

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  //Serial.println("Goodnight moon!");
    
  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);
  
  //sendATcommand("AT");
  
  attachInterrupt(0, InterruptServiceRoutine, FALLING);

  //for(int i = 0; i < 256; i++){
  //}
}

void InterruptServiceRoutine(){
    isRING = true;
}
String str;
void loop() // run over and over
{
  String commandstring = "";
    
  if(isRING == true){
    Serial.println("RING!!!");
    int i = 0;
    while(digitalRead(2) == false){
        i++;
        delay(10);
    }
    
    Serial.print("RING pin low for: ");
    Serial.print(i*10);
    Serial.println("ms");
    isRING = false;
  }  
  
  while (mySerial.available()) {
    seravailable = true;
    Serial.println((char)mySerial.read());
    /*Serial.print((char)c);
    Serial.print('\t');
    Serial.println((byte)c);*/

    /*if (c > 127){
        while(!mySerial.available()){}
        c = mySerial.read();
        c += 64;
    }*/
    //Serial.println((char)c + "\t" + (byte)c);
  }
  if(seravailable == true){
      Serial.println(str);
      str = "";
      Serial.println("Out of loop!!!");
      seravailable = false;
  }
  
  
  while (Serial.available()) {
    seravailable = true;
    //mySerial.println(Serial.read());
    char c;
    c == (char)Serial.read();
    c == '#' ? mySerial.print((char)26) : mySerial.print(c);
    delay(20);
    /*if (c > 127) {
      c -= 64;
      mySerial.print((char)0);
      mySerial.print((char)195);
    }*/
    //c == (char)'#' ? mySerial.write(26) : mySerial.print((char)c);
    //Serial.print((char)c);
  }

if(seravailable == true){
      Serial.print("str: ");
      Serial.println(str);
      //Transfer_CMD(str, 5000);
      sendATcommand(str);
      str = "";
      Serial.println("Out of loop!!!");
      seravailable = false;
  }
}

String raw(String ATcommand, uint32_t timeout){
    sendATcommand(ATcommand);
    
    String rec = "";
    uint32_t starttime = millis();
    
    char c;
    while(millis() - starttime < timeout){
        if (mySerial.available()){    //now that we got an answer, we wanna read all data, till the stream ends
	    c = (char)mySerial.read();    //append the new data to returnstring
            Serial.print((char)c);
            Serial.print("\t");
            Serial.println((byte)c);
            rec += (String)c;
	}              
    }
    Serial.print(F("readAT: "));
    Serial.println(rec);
    return rec;
}

String Transfer_OK(String ATcommand, uint32_t timeout){ //send an AT command to the GSM module  then, recieve either the "OK" from the GSM module(return true) or the "ERROR" (or anything else)(return false)
		sendATcommand(ATcommand);

                String data_recieved = recieveATcommand(timeout);
        
		return searchEndCommand(data_recieved);		//returns the end command if found, else empty string
    }
    
//########### AT-return struktur: (atcmd skickat till modul(inkl.\r(\n))) \r\n (atcmd return) \r\n \r\n (OK\ERROR) \r\n

	String Transfer_CMD(String ATcommand, uint32_t timeout){  //recieve the AT return value (or command) as a string. If any error the returnvalue is that errorcmd
		sendATcommand(ATcommand);

		String recieveStr = "";
		String endCommand;

		uint32_t starttime = millis();
		do {
			String rec = recieveATcommand(timeout);
                        Serial.print(F("rec: "));
                        Serial.println(rec);
			recieveStr += rec;
			endCommand = searchEndCommand(rec);
                        if(millis() - starttime > timeout)    break;
		} while (endCommand == "");
                Serial.println(F("passed do-while"));
                Serial.print(F("endcommand: "));
                Serial.println(endCommand);
		if (endCommand != "OK"){
			return endCommand;
		}/*
		recieveStr.trim();	//trim away all whitespace chars at both ends of the string
		recieveStr.substring(ATcommand.length() + 1, recieveStr.length() - 2); //removes the echo of the sendcommand and the "OK" at the end
		recieveStr.trim();	//trim away all whitespace chars at both ends of the string
		if (recieveStr == ""){
			return endCommand;
		}*/
		return recieveStr;
    }
    
	
	String recieveATcommand(uint32_t timeout){
		String ATreturn;
		uint32_t starttime = millis();
		//Serial.flush();  (flushes away new arrived data?)               //flush the buffer
                char c;
		while(millis() - starttime < timeout){
                    if (mySerial.available()){    //now that we got an answer, we wanna read all data, till the stream ends
            	        c = (char)mySerial.read();    //append the new data to returnstring
                        Serial.print((char)c);
                        Serial.print("\t");
                        Serial.println((byte)c);
                        ATreturn += (String)c;
                    }              
                }
                Serial.print(F("readAT: "));
                Serial.println(ATreturn);
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
                String SEARCHSTR_LEN = F("searchstr: ");
                Serial.println(SEARCHSTR_LEN + searchstr);
		for (int i = 0; i <= 3; i++){    //RX_ENDCOMMANDS.length - 4		sista loopen - [3]	dÃ¤rfÃ¶r i <= 3
			int commandPos = searchstr.indexOf(at.RX_ENDCOMMANDS[i]);    //    variable to be able to use the index in two places without calling func again
			if (commandPos != -1)                                                                    //if an endcommand is found
				return searchstr.substring(commandPos, searchstr.length());    //return the entire endcommand(for cme,cms errors with return variables)
		}
            return "";
	}



