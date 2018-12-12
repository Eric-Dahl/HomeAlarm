#include "AlarmData.h"

int free__Ram()
{
	extern int __heap_start, *__brkval;
	int v;
	return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

	User::User(String *args, byte *tempRFID){	//assign all values

		for (unsigned int i = 0; i < args[0].length() && !(name.isFull()); i++){    	//name (char by char)	(unsigned not very important)
			name.add(args[0].charAt(i));
		}

		for (unsigned int i = 0; i < args[1].length() && !(phone.isFull()); i++){      //phone (char by char)	(unsigned not very important)
			phone.add(args[1].charAt(i));
		}

		if (args[2] == F("0") || args[2] == F("1")){						//hasRestrictedAccess (+ format check)
			hasRestrictedAccess = (bool)args[2].toInt();
		}
		else{
			hasRestrictedAccess = 0;	//Q: should one gain access if something goes wrong?
		}

		for (int i = 0; i < 4; i++){										//RFID
			RFID[i] = tempRFID[i];
		}

	}

	RFTransmitter::RFTransmitter(String *args) : lowBatt(0), lowBattDebounce(0) {
		for (unsigned int i = 0; i < args[0].length() && !(name.isFull()); i++){    //fill in new name (char by char)	(unsigned not very important)
			name.add(args[0].charAt(i));
		}
		D1 = (bool)(args[1].toInt());
		D2 = (bool)(args[2].toInt());
		D3 = (bool)(args[3].toInt());
		//front_door = (bool)args[4].toInt();
	}



//______________________________Users & Transmitters_____________________________________________________
	int AlarmData::getUserpos(String username){ //searches for users[x].name that equals name		returns x

		for (byte i = 0; i < users.size(); i++){
			String name = users.get(i)->name.get();
			if (name.equals(username)){
				return i;
			}
		}
		return -1;
	}
	String AlarmData::listUsers(){ //>loop users.toString
		String returnstr = F("Name (phone), \"restricted access\"\n\n");
		for (byte i = 0; i < users.size(); i++){
			returnstr += users.get(i)->toString();
			Serial.println((int)free__Ram());
		}
		Serial.println((int)free__Ram());
		return returnstr;
	}

	int AlarmData::getTransmitterpos(String transmittername){	//searches for tranmitters[x].name that equals name		returns x
		for (byte i = 0; i < transmitters.size(); i++){
			String name = transmitters.get(i)->name.get();
			if (name.equals(transmittername))
				return i;
		}
		return -1;
	}
	String AlarmData::listTransmitters(){ //>loop transmitters.toString        
		String returnstr = F("Name, (ID) low battery\n\n");
		for (byte i = 0; i < transmitters.size(); i++){
			returnstr += transmitters.get(i)->toString();
		}
		return returnstr;
	}
//______________________________verification, matching_____________________________________________________

	int AlarmData::verifyPhone(String number){ //match with users[?].phone and returns the pos in vector users, no match return	-1   
		if (users.isEmpty()) return -1;
		number.trim();
		for (byte i = 0; i < users.size(); i++){
			String dbNum = users.get(i)->phone.get();
			dbNum.trim();
			if (number.equals(dbNum)){
				return i;
			}
		}
		return -1;
	}

	bool AlarmData::verifyRFID(byte *RFID){ //match it w/ users[?].RFID and return true if there is a match, otherwise false
		for (byte i = 0; i < users.size(); i++){
			for (byte j = 0; j <= 3; j++){
				Serial.print(users.get(i)->RFID[j]);
				Serial.print(" vs. ");				
				Serial.println(RFID[j]);

				if (users.get(i)->RFID[j] != RFID[j]){
					Serial.println("break");
					break;
				}
				else{
					Serial.println("!!not break");
				}
				if (j == 3)
					return true;
			}
		}
		return false;
	}
	int AlarmData::matchRFdata(bool *ID){ //returns the position in vector transmitters, if no match return -1  
		for (byte i = 0; i < transmitters.size(); i++){
			if (transmitters[i].D1 == ID[0] && transmitters[i].D2 == ID[1] && transmitters[i].D3 == ID[2])
				return i;
		}
		return -1;
	}