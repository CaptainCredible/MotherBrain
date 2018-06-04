
void sendWire2microBit(int note) {
	noteToSend = note;
	trackOrNote = false;
	digitalWrite(interruptPin, LOW);
	isSending = true;
	timeOutStamp = millis();
}

void sendWire2microBitTrackAndNote(byte traxx, byte note) {
	//Serial.print("sent track ");
	//Serial.println(traxx);
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
	//Serial.print("sent interrupt and currentStep is ");
	//Serial.println(currentStep);
}

void sendTracksBuffer64() {
	dataPacket64 = 0;
	for (byte i = 0; i < 8; i++ ) {
		dataPacket64 = dataPacket64 << 8;
		dataPacket64 = dataPacket64 + tracksBuffer[i];
	}
	digitalWrite(interruptPin, LOW); //start by telling microbit to request track
	isSending = true;
	timeOutStamp = millis();
	//Serial.print("sent sendTracksBuffer interrupt and currentStep is ");
	//Serial.println(currentStep);
	delay(1); //change so there is a timer polled and things can be done in background!
}

void requestEvent() {
	I2C_writeAnything(dataPacket64);
	isSending = false;
	digitalWrite(interruptPin, HIGH);
	//Serial.print("responded and currentStep is ");
	//Serial.println(currentStep);
}


#define timeOut  1000
void checkTimeOut() {
	if (millis() - timeOutStamp > timeOut && isSending) {
		digitalWrite(interruptPin, HIGH);
		isSending = false;
		//Serial.println(" i2c TIMEOUT! ");
	}
}

