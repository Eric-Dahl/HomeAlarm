#ifndef ARDUINO_VECTOR_CPP
#define ARDUINO_VECTOR_CPP

#include "Avec.h"


template <typename T, byte _size>
Avec<T, _size>::Avec(const Avec &rval) : effective_size(rval.effective_size){
	if (effective_size > MAX_SIZE){
		effective_size = MAX_SIZE;
	}
	for (int i = 0; i < effective_size; i++){
		collection[i] = rval.collection[i];
	}
}

template <typename T, byte _size>
Avec<T, _size>& Avec<T, _size>::operator = (const Avec& rval){

	effective_size = rval.effective_size;
	if (effective_size > MAX_SIZE){
		effective_size = MAX_SIZE;
	}

	for (int i = 0; i < effective_size; i++){
		collection[i] = rval.collection[i];
	}
	return *this;
}




template <typename T, byte _size>
void Avec<T, _size>::add(T item){		//adds item to collection and increases effective_size by 1
	if (isFull())		//prevent overflow
		return;
	collection[effective_size++] = item;
	//effective_size++;   //increment size
}


template <typename T, byte _size>
void Avec<T, _size>::remove(byte pos){		//removes the item at position pos (it�s an overwrite)
	if (isEmpty())		//prevent size to go minus
		return;
	one_step_forward(pos);		//fill up the removed space (by moving the items behind one step forward)
	effective_size--;   //decrement size
}

template <typename T, byte _size>
void Avec<T, _size>::one_step_forward(byte from){	//moves all items after arg.from 1 step forward
	if (from > MAX_SIZE || from < 0) return;		//prevents overflowing of array items
	for (byte i = from + 1; i <= MAX_SIZE; i++){
		collection[i - 1] = collection[i];
	}
}


template <byte _size>
void Astring<_size>::remove(byte pos){
	Avec<char, _size>::remove(pos);
	clear(Avec<char, _size>::effective_size);		// nulls the "unnecessary" part of the string
}

template <byte _size>
void Astring<_size>::clear(byte from){
	Avec<char, _size>::effective_size = 0;
	while (from < Avec<char, _size>::MAX_SIZE){
		Avec<char, _size>::collection[from++] = 0;
	}
}


#endif	//ARDUINO_VECTOR_CPP