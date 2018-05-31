void handleLPNoteOn(byte channel, byte pitch, byte velocity) {

	int matrixCursor = LPtoMatrix[pitch] + (currentPage * 8) + (scrollOffset * 20);
	if (seqMatrix[matrixCursor] > 0) {
		seqMatrix[matrixCursor] = 0;
		Serial.print("turned it off - ");
	} 
	else {
		Serial.print("turned it on - ");
		seqMatrix[matrixCursor] = 123;
	}
	sendWire2microBitTrackAndNote(pitch, trackToSend);
	currentPage = 100; //force page update
	updatePage();
	currentPage = 0; //force page update
	handleCursor();
	//Serial.println(matrixCursor);



	//digitalWrite(LEDPIN, HIGH);
	//sendWire2microBit(pitch);
}

void handleLPNoteOff(byte channel, byte pitch, byte velocity) {

}

void handleLPCC(byte channel, byte CC, byte val) {
	byte buttWasPressed = CC - 103;
	if (val > 0) {
		Serial.println(buttWasPressed);
		switch (buttWasPressed)
		{
		case 1:
			runClock = !runClock;
			break;
		default:
			trackToSend = buttWasPressed -1;
			Serial.print("trackToSend = ");
			Serial.println(trackToSend);
			break;
		}
	}
}