void handleLPNoteOn(byte channel, byte pitch, byte velocity) {
	//digitalWrite(LEDPIN, HIGH);
	sendWire2microBit(pitch);
}

void handleLPNoteOff(byte channel, byte pitch, byte velocity) {
	
}

void handleLPCC(byte channel, byte CC, byte val) {
	byte buttWasPressed = CC - 103;
	Serial.println(buttWasPressed);
}