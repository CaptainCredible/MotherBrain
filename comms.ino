
void sendWire2microBit(int note) {
	noteToSend = note;
	trackOrNote = false;
	digitalWrite(interruptPin, LOW);
	isSending = true;
	timeOutStamp = millis();
}

void sendWire2microBitTrackAndNote(byte note, byte track) {
	trackOrNote = true;
	noteToSend = note + (track << 8);
	digitalWrite(interruptPin, LOW); //start by telling microbit to request track
	isSending = true;
	timeOutStamp = millis();
}

int numberToTest = 10;
void requestEvent() {
	//I2C_writeAnything(noteToSend);
	Wire.write((byte *)&noteToSend, sizeof(noteToSend)); //this was hard to figure out!!!!!!!
	isSending = false;
	digitalWrite(interruptPin, HIGH);
	//Serial.print("sendt note ");
	//Serial.println(noteToSend);

}

#define timeOut  100
void checkTimeOut() {
	if (millis() - timeOutStamp > timeOut && isSending) {
		digitalWrite(interruptPin, HIGH);
		isSending = false;
		Serial.println(" i2c TIMEOUT! ");
	}
}