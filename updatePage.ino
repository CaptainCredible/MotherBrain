void updatePage(byte mode) { // force is a boolean to force a page update even if its not a page flip

	if (currentPage != (currentStep >> 3)) { //Time to flip the page
		Serial.print("REDRAW   prev = ");
		Serial.print(prevPage);
		Serial.print(" current = ");
		Serial.println(currentStep >> 3);
		prevPage = currentPage;
		currentPage = currentStep >> 3;
		firstStepOfPage = currentPage * 8;
		clearPage();
		updateVertButts();
		for (byte row = 0; row < 8; row++) {
			for (byte col = 0; col < 8; col++) {
				byte ledCursor = (row * 8) + col;
				byte seqMatrixCursor = col + firstStepOfPage + (row * 20);
				if (seqMatrix[seqMatrixCursor] > 0) {
					LPSetLed(ledCursor, seqMatrix[seqMatrixCursor]);
					delay(1); //todo make wait in background ?
				}
			}
		}
	}
	handleCursor();
	Serial.print("handled overview page : ");
	Serial.println(currentPage);
}

byte reset[3] = { 176,0,0 };
void clearPage() {
	Serial1.write(reset, 3);
	
}

void LPSetLed(byte led, byte colour) {
	launchPad.sendNoteOn(LPMAP[led], colour, 1);
}

void LPSetLedRaw(byte led, byte colour) {
	launchPad.sendNoteOn(led, colour, 1);
}

byte seqLedColour = 101;
void handleCursor() {
	launchPad.sendControlChange(topButts[lastStep % 8], 0, 1);
	launchPad.sendControlChange(topButts[currentStep % 8], seqLedColour, 1);
}


void changePageMode(byte newMode) {
	clearVertButts();
	pageMode = newMode;
	if (pageMode == 0) {
		setAllVertButts();
	}
	else {
		launchPad.sendNoteOn(vertButts[pageMode - 1], trackColours[pageMode - 1], 1);
	}
	updatePage(pageMode);
}

void 	clearVertButts() {
	for (int i = 0; i < 8; i++) {
		
		launchPad.sendNoteOn(vertButts[i], 0, 1);
		delay(1);
	}
}

void setAllVertButts() {
	for (int i = 0; i < 8; i++) {
		
		launchPad.sendNoteOn(vertButts[i], trackColours[i], 1);
		delay(1);
	}
}

void updateVertButts() {
	if (pageMode == 0) {
		setAllVertButts();
	}
	else {
		launchPad.sendNoteOn(vertButts[pageMode - 1], trackColours[pageMode - 1], 1);
		delay(1);
	}
}

