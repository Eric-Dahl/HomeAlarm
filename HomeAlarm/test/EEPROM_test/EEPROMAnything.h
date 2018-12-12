#include <EEPROM.h>
#include <Arduino.h>  // for type definitions

template <class T> int EEPROM_writeAnything(int ee, const T& value)	//
{
	const byte* p = (const byte*)(const void*)&value;
	unsigned int i;
	for (i = 0; i < sizeof(value); i++){
        while(!eeprom_is_ready()){} //w8 until eeprom is ready for next write
		EEPROM.update(ee++, *p++);
	}
	return i;
}

template <class T> int EEPROM_readAnything(int ee, T& value)
{
	byte* p = (byte*)(void*)&value;
	unsigned int i;
	for (i = 0; i < sizeof(value); i++){
        while(!eeprom_is_ready()){} //w8 until eeprom is ready for next write
		*p++ = EEPROM.read(ee++);
	}
	return i;
}
