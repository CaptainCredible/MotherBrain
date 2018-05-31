
void updatePage() { // force is a boolean to force a page update even if its not a page flip
	
	if (prevPage != (currentStep >> 3)) { //Time to flip the page
		prevPage = currentPage;
		currentPage = currentStep >> 3;
		//Serial.println("updatedPage");
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
	Serial.print("handled page : ");
	Serial.println(currentPage);
}

byte reset[3] = { 176,0,0 };
void clearPage() {
	Serial1.write(reset, 3);
}

void LPSetLed(byte led, byte colour) {
	launchPad.sendNoteOn(LPMAP[led], colour,1);
}

void LPSetLedRaw(byte led, byte colour) {
	launchPad.sendNoteOn(led, colour, 1);
}

byte seqLedColour = 101;
void handleCursor() {
	launchPad.sendControlChange(topButts[lastStep%8], 0,1);
	launchPad.sendControlChange(topButts[currentStep % 8], seqLedColour,1);
}