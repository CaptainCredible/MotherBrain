byte oldPageMode = 0;

//need to separate curentpage from viewpage
void updatePage(byte mode) { // forceUpdate is a boolean to force a page update even if its not a page flip
	if (follow) {    //if we are in follow mode, the page should correspond to the page the cursor is on. 
		if (currentPage != (currentStep >> 3) || forceUpdate || pageMode != oldPageMode) { //Time to flip the page
			clearPage();
			oldPageMode = mode;
			forceUpdate = false;
			digitalWrite(ledApin, forceUpdate);
			prevPage = currentPage;
			currentPage = currentStep >> 3;
			firstStepOfPage = currentPage * 8;
			updateVertButts();
			if (pageMode == 0) {
				handleOverviewPage();
			}
			else {
				handleTrackPage(pageMode);
			}
		}
	}
	else {  // if we are not in follow mode
		if (currentPage != (pageSelect) || forceUpdate || pageMode != oldPageMode) { //Time to flip the page
			clearPage();
			oldPageMode = mode;
			forceUpdate = false;
			//digitalWrite(ledApin, forceUpdate);
			prevPage = currentPage;
			currentPage = pageSelect;
			firstStepOfPage = currentPage * 8;
			updateVertButts();
			if (pageMode == 0) {
				handleOverviewPage();
			}
			else {
				handleTrackPage(pageMode);
			}
		}
	}
	handleCursor();
}

void handleTrackPage(byte trackPageToHandle) {
	byte trackPageWeAreHandling = trackPageToHandle - 1; //move 1-8 to 0-7
	if (isPoly[trackPageWeAreHandling]) {
		for (byte col = 0; col < 8; col++) {             
			int seqMatrixCursor = col + (currentPage * 8) + (trackPageWeAreHandling * matrixTrackOffset);
			int val = seqMatrix[seqMatrixCursor];
			for (int i = 0 ; i < 8; i++) {
				if (bitRead(val, i + scrollOffset)) {
					int currentLedCursor = col + (8 * i
						); // where to point on the LPMAP array
					LPSetLed(currentLedCursor, trackColours[trackPageWeAreHandling]);
				}
			}
		}
	}
	else {//handle the non poly 127 note tracks
		for (byte col = 0; col < 8; col++) {
			int seqMatrixCursor = col + (currentPage * 8) + (trackPageWeAreHandling * matrixTrackOffset);
			int val = seqMatrix[seqMatrixCursor];
			if (val > 0) {
				int LPmatrixCursor = col + ((val - 1) * 8);
				LPSetLed(LPmatrixCursor, trackColours[trackPageWeAreHandling]);
			}
			
		}
	}
	
	
}

void handleOverviewPage() {
	//read seqMatrix and set Leds:
	for (byte row = 0; row < 8; row++) {
		for (byte col = 0; col < 8; col++) {
			byte ledCursor = (row * 8) + col;
			byte seqMatrixCursor = col + firstStepOfPage + (row * matrixTrackOffset);
			if (seqMatrix[seqMatrixCursor] > 0) {
				LPSetLed(ledCursor, trackColours[row]);
				delay(1); //todo make wait in background ?
			}
		}
	}
}


byte reset[3] = { 176,0,0 };
void clearPage() {
	//Serial.println("CLEARED PAGE");
	Serial1.write(reset, 3);
	//delay(1000);

}

void LPSetLed(byte led, byte colour) {
	launchPad.sendNoteOn(LPMAP[led], colour, 1);
}

void LPSetLedRaw(byte led, byte colour) {
	launchPad.sendNoteOn(led, colour, 1);
}


void displayPagenumber() {

}

byte seqLedColour = 3;
void handleCursor() {
	byte colChanger = 0;
	if (currentPage == currentStep >> 3) {
		launchPad.sendControlChange(topButts[lastStep % 8], 0, 1);
		launchPad.sendControlChange(topButts[currentStep % 8], seqLedColour - colChanger, 1);
	}
	
}

void changePageMode(byte newMode) {
	//Serial.print("changed pagemode to ");

	if (newMode != pageMode) { //if a track is selected
		clearVertButts();
		pageMode = newMode;
	}
	else {						//if the same page is selected again
		pageMode = 0;			//go to overview
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

