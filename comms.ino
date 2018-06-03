
void sendWire2microBit(int note) {
	noteToSend = note;
	trackOrNote = false;
	digitalWrite(interruptPin, LOW);
	isSending = true;
	timeOutStamp = millis();
}

void sendWire2microBitTrackAndNote(byte traxx, byte note) {
	Serial.print("sent track ");
	Serial.println(traxx);
	trackOrNote = true;
	noteToSend = note + (traxx << 8);
	digitalWrite(interruptPin, LOW); //start by telling microbit to request track
	isSending = true;
	timeOutStamp = millis();
	delay(2); //change so there is a timer polled and things can be done in background!
	
}

void send32BitInt(){
	digitalWrite(interruptPin, LOW); //start by telling microbit to request track
	isSending = true;
	timeOutStamp = millis();
	delay(1); //change so there is a timer polled and things can be done in background!
}

void send64BitInt() {
	digitalWrite(interruptPin, LOW); //start by telling microbit to request track
	isSending = true;
	timeOutStamp = millis();
	delay(1); //change so there is a timer polled and things can be done in background!
}

void requestEvent() {
//	I2C_writeAnything(dataPacket);  //32bit
	I2C_writeAnything(dataPacket64);
	isSending = false;
	digitalWrite(interruptPin, HIGH);
}

//void requestEvent() {
//	//I2C_writeAnything(noteToSend);
//	Wire.write((byte *)&noteToSend, sizeof(noteToSend)); //this was hard to figure out!!!!!!!
//	isSending = false;
//	digitalWrite(interruptPin, HIGH);
//}

#define timeOut  100
void checkTimeOut() {
	if (millis() - timeOutStamp > timeOut && isSending) {
		digitalWrite(interruptPin, HIGH);
		isSending = false;
		Serial.println(" i2c TIMEOUT! ");
	}
}

