

void SMSallusers(const String &message){
      Serial.println(F("SMSallUsers"));

    for(int i = 0; i < alarmdata.users.size(); i++){
        User* smsuser = alarmdata.users.get(i);
        if((smsuser->hasRestrictedAccess == false || alarmdata.isExtendedAccess() == true) && !(smsuser->phone.isEmpty())){ //filters out those with restricted access and those without a number
            sendSMS(smsuser->phone.get(), message);
        }    
    }
}

//The program's user interface 
void cmdHandler(String command, String args[3], String sender, String &msg2send){      
//The SMS & BT commands handler - Provides a way to communicate with the alarm through commands and args, either returning the value(s) asked for or changing values in the alarm, 
//such as switching it on/off or adding/removing users
    
    String original_command = command;
    command.toUpperCase();        //case will not affect the result

    Serial.println(F("IN cmdH"));

    const String _ = ""; //helps when adding strings together (concatenation) when the first string is a string literal ("lorem ipsum")
    const String BY_SENDER = _ + F(" by ") + sender + F(".");

    //_______________________________________________________________________________________________________________________________________________________Help
    
    if (command.equals(F("?"))){
        msg2send = F("Help - <cmd>?\n\nON (Help - ON? (<cmd>=ON))\nOFF\nSTATUS\nBLUETOOTH\nACCESS\nADDUSER\nREMOVEUSER\nLISTUSERS\nSETUSERPHONE\nADDSENSOR\nREMOVESENSOR\nLISTSENSORS");                                      
    }    
    else if (command.equals(F("<CMD>?"))){    //if someone misunderstands "Help: <cmd>?" in list of cmd:s
        msg2send = F("Replace <cmd> with one of the commands in list for help\n\nList commands - ?");                                      
    }
    else if (command.equals(F("I"))){
        msg2send = F("SYNTAX");                                      
    }
    else if (command.equals(F("SUFFIX"))){
        msg2send = F("Command Types\nType\tEnding\nRead\t''\tread info\nWrite\t':'\tsend info\tON, OFF & BLUETOOTH also write, but need no suffix\nHelp\t'?'\tread help");
    }
    else if (command.equals(F("SYNTAX"))){
        msg2send = F("Syntax - <cmd> (+ <info> if cmd sends information)");
    }   //command: info1; info2;
    else if (command.equals(F("ON?")) || command.equals(F("OFF?")) || command.equals(F("STATUS?")))
                                              {    msg2send = F("Send ON or OFF to turn alarm on/off and STATUS to read this status\nSend ? to list all commands");  }
    else if (command.equals(F("ACCESS?"))){        msg2send = F("ACCESS: on/off;\nWhen on, restricted users gain the SMS function. (They are not restricted anymore)");    }
    else if (command.equals(F("BLUETOOTH?"))){     msg2send = F("BLUETOOTH\nEnables bluetooth communication with the Alarm for 30 min or until turned off by sending QUIT through bluetooth");    }
    else if (command.equals(F("ADDUSER?"))){       msg2send = F("SCAN RFID-TAG FIRST (relate to new user)\nADDUSER: name; phone; [0/1;]<-optional 1 - restricted access, see ACCESS?");    }
    else if (command.equals(F("REMOVEUSER?"))){    msg2send = F("REMOVEUSER: name;");    }
    else if (command.equals(F("SETUSERPHONE?"))){  msg2send = F("SETUSERPHONE: name; new phone;");    }
    else if (command.equals(F("LISTUSERS?"))){     msg2send = F("LISTUSERS\nreturns a list of all Users' credentials");    }
    else if (command.equals(F("ADDSENSOR?"))){    msg2send = F("ADDSENSOR: name; ID1;ID2;ID3;\nIDs can be either 0/1 (eg. 0v/9v on pins D1-D3 on HT12E)");    }
    else if (command.equals(F("REMOVESENSOR?"))){ msg2send = F("REMOVESENSOR: name;");    }
    else if (command.equals(F("LISTSENSORS?"))){  msg2send = F("LISTSENSORS\nreturns a list of all connected sensors");    }
        
    //________________________________________________________________________________________________________________________________________________________Larmstatus
    else if (command.equals(F("STATUS"))){    //no argument
        msg2send = (String)alarmdata.getState();    //"return" alarm status
    }
    
    else if (command.equals(F("TRUE")) || command.equals(F("1")) || command.equals(F("ON"))){
        setAlarm(true);
        msg2send = _ + F("Alarm turned on") + BY_SENDER;
    }
    else if (command.equals(F("FALSE")) || command.equals(F("0")) || command.equals(F("OFF"))){
        setAlarm(false);
        msg2send = _ + F("Alarm turned off") + BY_SENDER;
    }
//____________________________________________________________________________________________________________________________________________________________MobiltjÃ¤nster

//IMPORTANT!!! - Scan the new RFID-card before adding the user - The latest scanned card will be associated with the new user!!!
     else if (command.equals(F("ADDUSER"))){    //arg1 - name    arg2 - phone    arg3 - (bool) hasRestrictedAccess (if restricted, the sms-command system wont work)
            User user(args, RFID.uid.uidByte);
            alarmdata.users.add(user);   
            msg2send =  args[0] + F(" added to users") + BY_SENDER;
    }
    else if (command.equals(F("REMOVEUSER"))){    //arg1 - name of user to be removed  
        if (args[0].equals(F("ALL"))){
            alarmdata.users.clear();
            msg2send = _ + F("All users removed") + BY_SENDER;
        }
        else{
            alarmdata.users.remove(alarmdata.getUserpos(args[0]));                    
            msg2send = args[0] + F(" removed from users") + BY_SENDER; 
        }
    }
    else if (command.equals(F("SETUSERPHONE"))){    //arg1 - name    arg2 - new phone
        User* user = alarmdata.users.get(alarmdata.getUserpos(args[0]));
        user->phone.clear();                                                     //clear old phone number
        for(unsigned int i = 0; i < args[1].length() && !(user->phone.isFull()); i++){    //fill in new phone number (char by char)       (unsigned not very important)
            user->phone.add(args[1].charAt(i));
        }
        msg2send = args[0] + F("'s phone number changed to ") + args[1] + BY_SENDER;     
    }
    
    else if (command.equals(F("LISTUSERS"))){    //no argument
        msg2send = alarmdata.listUsers();
    }
    else if (command.equals(F("ACCESS")) || command.equals(F("SETEXTENDEDACCESS"))){    //arg1 - (bool) sets the extended access  
        args[0].toUpperCase();
        Serial.println(args[0]);
        bool tester = args[0].equals(F("1"));
        alarmdata.setExtendedAccess(args[0].equals(F("TRUE")) || args[0].equals(F("1")) || args[0].equals(F("ON")));
        Serial.println(args[0].equals(F("TRUE")) || args[0].equals(F("1")));
        Serial.println((args[0].equals(F("TRUE")) || args[0].equals(F("1"))));
        String access = tester ? F("on") : F("off"); //alarmdata.isExtendedAccess()
        Serial.println(F("tester: "));
        Serial.println(tester);
        Serial.println(alarmdata.isExtendedAccess());
        msg2send =  _ + F("Extended access turned ") + access + BY_SENDER;
    }
    else if (command.equals(F("BLUETOOTH"))){    //Here is Bluetooth data recieved, managed and then sent back to the sender (by Bluetooth)
        Serial.println(F("BT"));
        BT_Active = true;   //flag that starts Bluetooth_communication() from checkInterruptFlags()
        msg2send = F("Bluetooth communication started");
    }
//____________________________________________________________________________________________________________________________________________________________Transmitter...
    else if (command.equals(F("ADDSENSOR"))){  //arg1 - name    arg2/3/4 - (bool) Value of D1/D2/D3 on the HT12E module (pin low(0V)-> 0 || pin high (9V)-> 1)
        RFTransmitter sensor(args);
        alarmdata.transmitters.add(sensor);
        msg2send = args[0] + F(" added to sensors") + BY_SENDER;
    }
    else if (command.equals(F("REMOVESENSOR"))){    //arg1 - sensorname
        if (args[0].equals(F("ALL"))){
            alarmdata.transmitters.clear();
            msg2send = _ + F("All sensors removed") + BY_SENDER;
        }
        else{
            alarmdata.transmitters.remove(alarmdata.getTransmitterpos(args[0]));
            msg2send = args[0] + F(" removed from sensors") + BY_SENDER;
        }
    }
    else if (command.equals(F("LISTSENSORS"))){    //no argument
        msg2send = alarmdata.listTransmitters();
    }
    else{
        msg2send = original_command + F(" invalid\n\n? - List of all commands + how to get help\ni - extra curiosa");
        //Send ? to get a list of all commands and how to use them.\n
        //Send i for curiosa.
    }
    EEPROM_update();
    
}

