
void updatePage() {
	
	if (currentPage != currentStep >> 3) { //Time to flip the page
		Serial.println("updatedPage");
		currentPage = currentStep >> 3;
		firstStepOfPage = currentPage * 8;
		clearPage();
		for (byte row = 0; row < 8; row++){
			for (byte col = 0; col < 8; col++) {
				byte ledCursor = (row * 8) + col;
				byte seqMatrixCursor = col + firstStepOfPage + (row * 20);
				if (seqMatrix[seqMatrixCursor] > 0) {
					LPSetLed(ledCursor, seqMatrix[seqMatrixCursor]);
					delay(1);
				}
			}
		}
	}
	handleCursor();
}

byte reset[3] = { 176,0,0 };
void clearPage() {
	Serial1.write(reset, 3);
}


void LPSetLed(byte led, byte colour) {
	launchPad.sendNoteOn(LPMAP[led], colour,1);
}


byte seqLedColour = 101;
void handleCursor() {
	launchPad.sendControlChange(topButts[lastStep%8], 0,1);
	launchPad.sendControlChange(topButts[currentStep % 8], seqLedColour,1);
}