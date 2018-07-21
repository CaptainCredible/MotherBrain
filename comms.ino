


/*

void sendWire2microBit(int note) {
noteToSend = note;
trackOrNote = false;
digitalWrite(interruptPin, LOW);
isSending = true;
timeOutStamp = millis();
}

void send64BitInt() {
digitalWrite(interruptPin, LOW); //start by telling microbit to request track
isSending = true;
timeOutStamp = millis();
delay(1); //change so there is a timer polled and things can be done in background!
}

void send32BitInt(){
digitalWrite(interruptPin, LOW); //start by telling microbit to request track
isSending = true;
timeOutStamp = millis();
delay(1); //change so there is a timer polled and things can be done in background!
}

void sendTracksBuffer64() {
dataPacket64 = 0;
for (byte i = 0; i < 8; i++ ) {
dataPacket64 = dataPacket64 << 8;
dataPacket64 = dataPacket64 + tracksBuffer[i];
}

digitalWrite(interruptPin, LOW); //start by telling microbit to request track
i2cTimer = micros();
isSending = true;
timeOutStamp = millis();
//Serial.print("sent sendTracksBuffer interrupt and currentStep is ");
//Serial.println(currentStep);
//delay(1); //change so there is a timer polled and things can be done in background!
}

void sendWire2microBitTrackAndNote(byte traxx, byte note) {
trackOrNote = true;
noteToSend = note + (traxx << 8);
digitalWrite(interruptPin, LOW); //start by telling microbit to request track
isSending = true;
timeOutStamp = millis();
delay(2); //change so there is a timer polled and things can be done in background!
}

*/

unsigned long i2cFails = 0;
unsigned long successfullI2cs = 0;

void sendTracksBuffer() {
	digitalWrite(interruptPin, LOW); //start by telling microbit to request track
	//i2cTimer = micros();
	isSending = true;
	timeOutStamp = millis();
}

void sendUsbMidiPackage() {
	sentAMidiBuffer = true; //flag the fact that we are sending midi buffer
	sendTracksBuffer();
}


void debugRequestEvent() {
	i2cTimer = micros() - i2cTimer;
	Serial.print("transmission took ");
	Serial.print(i2cTimer);
	Serial.println(" microseconds!");
	isSending = false;
	digitalWrite(interruptPin, HIGH);
}



void requestEvent() {
	if (sentAMidiBuffer) {
		I2C_writeAnything(midiTracksBuffer16x8);
	}
	else {
		I2C_writeAnything(tracksBuffer16x8);
	}
	
	isSending = false;
	digitalWrite(interruptPin, HIGH);
	Serial.print("i2c OK! ");
	successfullI2cs++;
	Serial.print(successfullI2cs);
	Serial.print("   fails ");
	Serial.println(i2cFails);

	if (sentAMidiBuffer) {											//if we sent a midi buffer
		for (byte i = 0; i < 7; i++) {								//for every channel entry in buffer
			midiTracksBuffer16x8[i] = 0;							// clear buffer
		}
		Serial.println("erased midibuffer");
		sentAMidiBuffer = false;									//set flag back to normal buffers.
	}
}




#define timeOut  10
void checkTimeOut() {
	if (millis() - timeOutStamp > timeOut && isSending) {
		digitalWrite(interruptPin, HIGH);
		isSending = false;
		Serial.println(" i2c TIMEOUT! ");
		i2cFails++;
	}
}

