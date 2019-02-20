
void debugaroonie() {
	digitalWrite(shiftLed, HIGH);
	delay(1000);
	digitalWrite(shiftLed, LOW);
	delay(500);
}





void preHandleUSBNoteOn(byte inChannel, byte inNumber, byte inVelocity) {
	//Serial.println("MIDI");
	if (inVelocity > 0) {
		HandleUsbNoteOn(inNumber, inVelocity, inChannel);
	}
	else {
		HandleUsbNoteOff(inNumber, inVelocity, inChannel);
	}
};



void handleUSBstop() {
	midiClockRunning = false;
	clearTopLedsArray();
	handleTopLeds();
}

#ifdef OLDSCHOOLUSB

void usbmidiprocessing() {
	internalClockSelect = runClock;
	
	while (MIDIUSB.available() > 0) {
		
		MIDIEvent e = MIDIUSB.read();
		// IF NOTE ON WITH VELOCITY GREATER THAN ZERO
		if ((e.type == NOTEON) && (e.m3 > 0)) {
			HandleUsbNoteOn(e.m2, e.m3, e.m1 - 144);
		}
		// IF USB NOTE OFF
		else if (e.type == NOTEOFF) {
			HandleUsbNoteOff(e.m2, e.m3, e.m1 - 144);
		}
		// IF NOTE ON W/ ZERO VELOCITY
		else if ((e.type == NOTEON) && (e.m3 == 0)) {
			//if (!internalClockSelect) {
			HandleUsbNoteOff(e.m2, e.m3, e.m1 - 144);
			//}
		}
		else if (e.type == TICK) {
			handleUsbMidiClockTicks();
			if (e.m1 == 252) {  //this is stop				
				midiClockRunning = false;
				clearTopLedsArray();
				handleTopLeds();
			}
		}
		
		else if (e.type == RESTART) {
			
			resetSeq();
			
		}
	}
	
	if (MIDIUSB.available() == 0 && hadANoteOn) {  //if there is no message but there was on prev iteration
		timeOutDeadline = millis() + USBReceiveTimeOutThresh; //start the timer
		hadANoteOn = false;
		waitingForTimeOut = true;
	}

	if (waitingForTimeOut) { //if we are waiting to see if there are any more messages for this step
		if (millis() > timeOutDeadline) { //if we timed out
			sendUsbMidiPackage();
			waitingForTimeOut = false;
		}
	}
	
}


#endif // OLDSCHOOLUSB




void resetSeq() {
	currentStep = -1;
}

void debugInt(unsigned int valToPrint) {


}

void hijackUSBMidiTrackBuffer(byte val, byte slot) {
	if (!waitingForTimeOut) {
		clearMidiTracksBuffer();
		bitSet(midiTracksBuffer16x8[slot], val);				//set corresponding bit in corresponding int in the buffer to be sent
		sendUsbMidiPackage();
	}
}

void HandleUsbNoteOn(byte note, byte velocity, byte channel) {
	if (channel < 9) {
		timeOutDeadline = millis() + USBReceiveTimeOutThresh; //start the timer
		waitingForTimeOut = true;
		//Serial.print("MIDI CHANNEL ");
		//Serial.println(channel);
		prevNoteOnTime = millis();
		hadANoteOn = true;
		//Serial.println(note);
		if (isPoly[channel]) { // if this midi channel corresponds to a polyphonic orchestra channel
			if (note < 16) {												//make syre we dont overflow
				bitSet(midiTracksBuffer16x8[channel], note);				//set corresponding bit in corresponding int in the buffer to be sent
		//		Serial.println("POLY CHANNEL");
			}
		}
		else { //if this midi channel is controlling a monophonic (127 note) orchestra channel
		   //determine witch bits we are using
			if (channel == 6 || channel == 8) { //these are high bits on ints 6 and 7 in the buffer 
				midiTracksBuffer16x8[channel-1] = midiTracksBuffer16x8[channel-1] & 0b0000000011111111; // use bitmask to clear any previous values held in the most significant bits, leaving LSB alone
				midiTracksBuffer16x8[channel-1] = midiTracksBuffer16x8[channel-1] | (note << 8);         //shift note value left by 8 and compound (logical or) it to the rest
			}
			else if (channel == 7 || channel == 9) { //if it is tracks 8 or 10 they are the least significant bits
				midiTracksBuffer16x8[channel] = midiTracksBuffer16x8[channel] & 0b1111111100000000; // use bitmask to clear any previous values held in the least significant bits, leaving MSB alone
				midiTracksBuffer16x8[channel] = midiTracksBuffer16x8[channel] | note;         // compound (logical or) the int in the buffer with the note we want to add 
			} 
			debugInt(midiTracksBuffer16x8[7]);
		}
	}
}

void HandleUsbNoteOff(byte note, byte velocity, byte channel) {

}


void USBContinue() {
	
}

void timeCodeQuarterFrame(byte receiveData) {
	
}

void handleUsbMidiClockTicks() {
	runClock = false;
	lastMidiClockReceivedTime = millis();
	midiClockRunning = true;
	midiClockCounter++;
	midiClockCounter = midiClockCounter % midiClockDiv;
	if (midiClockCounter == 0) {
		midiClockStep();
	}
}
