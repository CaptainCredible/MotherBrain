void handleLPNoteOn(byte channel, byte pitch, byte velocity) {
	sendWire2microBit(pitch);
	digitalWrite(LEDPIN, HIGH);
}

void handleLPNoteOff(byte channel, byte pitch, byte velocity) {
	
}

void handleLPCC(byte channel, byte CC, byte val) {

}