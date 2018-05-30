
void sendWire2microBit(byte note) {
	noteToSend = note;
	isInt = false;
	digitalWrite(interruptPin, LOW);
	
}

void sendWire2microBitInt(byte note, byte track) {
	isInt = true;
	noteToSend = note;
	trackToSend = track;
	digitalWrite(interruptPin, LOW);
	isSending = true;
	timeOutStamp = millis();
}


void requestEvent() {
	if (isInt) {
		//Wire.beginTransmission();
		int val = noteToSend + (trackToSend << 8);
		Wire.write(val);  //not sure if this is sending as an int or if i need another argument here
	} else {
	Wire.write(noteToSend);
	digitalWrite(interruptPin, HIGH);
	isSending = false;
	}
}