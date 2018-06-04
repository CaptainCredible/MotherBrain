void handleLPNoteOn(byte channel, byte pitch, byte velocity) {

	int matrixCursor = LPtoMatrix[pitch] + (currentPage * 8) + (scrollOffset * 20);
	if (seqMatrix[matrixCursor] > 0) {
		seqMatrix[matrixCursor] = 0;
		//Serial.print("turned it off - ");
		LPSetLedRaw(pitch, 0);
	} 
	else {
		//Serial.print("turned it on - ");
		seqMatrix[matrixCursor] = 123;
		LPSetLedRaw(pitch, 123);
	}
	
	//updatePage(true); //instead of forcing an update of the entire page, i should write an algorithm that updates the coordinate in question
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
		case 6:
			currentStep = 255;
			handleStep();
			clockTimer = millis();
			if (!runClock) {
				currentStep = 0;
				updatePage(0); //TODO NEEDS TO BE CONDITIONAL
			}
			updatePage(0);
			handleCursor();
		case 7:
			currentStep--;
			updatePage(0); //TODO NEEDS TO BE CONDITIONAL
			handleCursor();
			break;
		case 8:
			currentStep++;
			updatePage(0); //TODO NEEDS TO BE CONDITIONAL
			handleCursor();
			break;
		default:
			trackToSend = buttWasPressed -1;
			Serial.print("trackToSend = ");
			Serial.println(trackToSend);
			break;
		}
	}
}