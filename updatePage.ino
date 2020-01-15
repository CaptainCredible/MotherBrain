byte oldselectedTrack = 0;

//need to separate curentpage from viewpage

#define pageNumberDisplayDuration 500 //how many ms to display page number for

#define numberCol 30

byte notesToWrite1[5] = { 86,82,78,74,70 };
byte notesToWrite2[11] = { 85,56,57,53,79,78,77,73,69,70,71 };
byte notesToWrite3[10] = { 85,56,57,53,79,78,75,69,70,71 };
byte notesToWrite4[9] = { 85,81,77,78,87,83,79,75,71 };

void displayPageNumber() {
	numIsDisplayed = true;
	pageNumberClearTimer = millis() + pageNumberDisplayDuration;
	switch (currentPage) {
	case 0: // draw 1 on page

		for (byte i = 0; i < 5; i++) {
			launchPad.sendNoteOn(notesToWrite1[i], numberCol, 1);
		}
		break;

	case 1: //draw 2 on page

		for (byte i = 0; i < 11; i++) {
			launchPad.sendNoteOn(notesToWrite2[i], numberCol, 1);
		}
		break;

	case 2: //draw 3 on page

		for (byte i = 0; i < 10; i++) {
			launchPad.sendNoteOn(notesToWrite3[i], numberCol, 1);
		}
		break;

	case 3: // draw 4 on page

		for (byte i = 0; i < 10; i++) {
			launchPad.sendNoteOn(notesToWrite4[i], numberCol, 1);
		}
		break;
	default:
		break;


	}
}

void updatePage(byte mode) { // forceUpdate is a boolean to force a page update even if its not a page flip

	if (follow) {
		int tempStep = currentStep;
		if (globalPolyRhythmEnable && polyRhythm[selectedTrack]) {
			tempStep = polyCurrentStep[selectedTrack];
		}
		if (currentPage != (tempStep >> 3) || forceUpdate || selectedTrack != oldselectedTrack) { //Time to flip the page	
			//Serial.println("POW");
			clearPage();

			oldselectedTrack = mode;
			forceUpdate = false;
			if (prevPage != currentPage) {
				//displayPageNumber();	
			}
			prevPage = currentPage;
			currentPage = tempStep >> 3;

			//Serial.print("currentPage = ");
			//Serial.println(currentPage);


			firstStepOfPage = currentPage * 8;
			updateVertButts();
			if (selectedTrack == 0) {
				handleOverviewPage();
			}
			else {
				handleTrackPage(selectedTrack);
			}
		}

	}
	else {  // if we are not in follow mode
		if (currentPage != (pageSelect) || forceUpdate || selectedTrack != oldselectedTrack) { //Time to flip the page
			clearPage();
			oldselectedTrack = mode;
			forceUpdate = false;
			//digitalWrite(ledApin, forceUpdate);
			prevPage = currentPage;
			currentPage = pageSelect;
			firstStepOfPage = currentPage * 8;
			updateVertButts();
			if (selectedTrack == 0) {
				handleOverviewPage();
			}
			else {
				handleTrackPage(selectedTrack);
			}
		}
	}
	handleTopLeds();
}

void handleTrackPage(byte trackPageToHandle) {

	byte trackPageWeAreHandling = trackPageToHandle - 1; //move 1-8 to 0-7
	if (isPoly[trackPageWeAreHandling]) {

		for (byte col = 0; col < 8; col++) {
			int seqMatrixCursor = col + (currentPage * 8) + (trackPageWeAreHandling * matrixTrackOffset);
			if (seqMatrixShift) {
				seqMatrixCursor += 256;
			}
			int val = seqMatrix[seqMatrixCursor];
			for (int i = 0; i < 8; i++) {
				if (bitRead(val, i + scrollOffset)) {
					int currentLedCursor = col + (8 * i
						); // where to point on the LPMAP array
					LPSetLed(currentLedCursor, trackColours[trackPageWeAreHandling]);
				}
			}
		}
	}
	else {//handle the non poly 127 note tracks
		for (byte col = 0; col < 8; col++) {																				// use col to handle what column we are drawing on left to right
			int seqMatrixCursor = col + (currentPage * 8) + (trackPageWeAreHandling * matrixTrackOffset);					// find right seqmatrix slot to read from
			if (seqMatrixShift) {
				seqMatrixCursor += 256;
			}
			unsigned int rawVal = seqMatrix[seqMatrixCursor];																// 
			if (!altMidiTrack) {

				//rawVal = 0b0000000011111111 & rawVal;  //use only last 8 bits of int
				rawVal = rawVal << 8;					//hacky way of clearing first 8 bits works because we are bitshifting back in next operation
			}

			rawVal = rawVal >> 8;					//use first 8 bits

			if (col == 0) {							//debug

			}
			byte invertedVal = 127 - rawVal;			//invert to make the grid the right way round
			byte val = invertedVal - scrollOffset;			// translate to our viewing space by acounting for how far we have scrolled

			if (col == 0) {

			}
			if (val > 0 & val < 9) {
				int LPmatrixCursor = col + ((val - 1) * 8);
				byte colWeWantToUse = trackColours[trackPageWeAreHandling];
				if (altMidiTrack) {
					colWeWantToUse = 3;
				}
				LPSetLed(LPmatrixCursor, colWeWantToUse);
			}

		}
	}


}

void handleOverviewPage() {
	//read seqMatrix and set Leds:
	for (byte row = 0; row < 8; row++) {
		for (byte col = 0; col < 8; col++) {
			byte ledCursor = (row * 8) + col;
			int seqMatrixCursor = col + firstStepOfPage + (row * matrixTrackOffset);
			if (seqMatrixShift) {
				seqMatrixCursor += 256;
			}
			if (seqMatrix[seqMatrixCursor] > 0) {
				LPSetLed(ledCursor, trackColours[row]);
				delay(1); //todo make wait in background ?
			}
		}
	}
}


byte reset[3] = { 176,0,0 };
void clearPage() {
	Serial1.write(reset, 3);
	pageWasCleared = true;

}

void LPSetLed(byte led, byte colour) {
	launchPad.sendNoteOn(LPMAP[led], colour, 1);
}

void LPSetLedRaw(byte led, byte colour) {
	launchPad.sendNoteOn(led, colour, 1);
}


void displayPagenumber() {

}

void clearTopLedsArray() { //clear the array that helps us keep track of what leds have and have not already been set

	for (int i = 0; i < 8; i++) {
		oldTopLedWasSet[i] = topLedWasSet[i];
		topLedWasSet[i] = 0;
	}
}



void handleTopLeds() {

	clearTopLedsArray(); //clear the array that helps us keep track of what leds have and have not already been set and also clear the leds
	handleCurrentPageDisplay();
	handleCurrentPageViewingDisplay();
	handleTimeSigDisplay();
	handleCursor();
	cleanupLeds();
	/*
	for (int i = 0; i < 8; i++) {
		Serial.print(oldTopLedWasSet[i]);
	}
	Serial.println("= old");


	for (int i = 0; i < 8; i++) {
		Serial.print(topLedWasSet[i]);
	}
	Serial.println("= new");

	for (byte i = 0; i < 8; i++) {
		oldTopLedWasSet[i] = topLedWasSet[i];
	}
	*/

}



#define currentPageDisplayCol 28
//#define currentPageDisplayCol 33
void handleCurrentPageDisplay() {   /// THE GREEN LED THAT SHOWS WHAT PAGE IS PLAYING IF NOT SAME AS PAGE WE ARE VIEWING!!!!
	if (!follow) { // no point displaying this if it is the same as playing page
		int tempStep = currentStep;
		if (globalPolyRhythmEnable && polyRhythm[selectedTrack]) {
			tempStep = polyCurrentStep[selectedTrack];
		}
		int tempPage = tempStep >> 3;
		launchPad.sendControlChange(topButts[tempPage], currentPageDisplayCol, 1);
		topLedWasSet[tempPage] = true;
	}

}

#define currentPageViewingCol 33 //ORANGE
void handleCurrentPageViewingDisplay() {   /// THE ORANGE LED THAT SHOWS WHAT PAGE WE ARE VIEWING!!!!
	launchPad.sendControlChange(topButts[currentPage], currentPageViewingCol, 1);
	topLedWasSet[currentPage] = true;
}

#define timeSigDisplayCol 99
void handleTimeSigDisplay() {
	if (oldTimeSig != timeSig || pageWasCleared) {
		launchPad.sendControlChange(topButts[7 - timeSig], timeSigDisplayCol, 1);
	}
	oldTimeSig = timeSig;
	topLedWasSet[7 - timeSig] = true;
	if (pageWasCleared) {
		pageWasCleared = false;
	}
}

void handleCursor() {
	int tempStep = currentStep;
	if (globalPolyRhythmEnable && polyRhythm[selectedTrack]) {
		tempStep = polyCurrentStep[selectedTrack];
	}
	if (currentPage == tempStep >> 3) {
		//if (!topLedWasSet[lastStep % 8]) {
		//	launchPad.sendControlChange(topButts[lastStep % 8], 0, 1);
		//}
		launchPad.sendControlChange(topButts[tempStep % 8], seqLedColour, 1);  //sets seqstepled regardless of whether they are on or not
		topLedWasSet[tempStep % 8] = true;
		Serial.print("tempStep % 8 = ");
		Serial.println(tempStep % 8);
	}
	else if (currentPage == lastStep >> 3) {
		launchPad.sendControlChange(topButts[lastStep % 8], 0, 1);
	}
}


void cleanupLeds() {
	Serial.println();
	for (byte i = 0; i < 8; i++) {
		if (!topLedWasSet[i] && oldTopLedWasSet[i]) { // if led is supposed to be off now but was on last time
			launchPad.sendControlChange(topButts[i], 0, 1);
			//Serial.print("*");
		}
		else {
			//Serial.print("-");
		}
	}
	for (int i = 0; i < 8; i++) {
		if (topLedWasSet[i]) {
			Serial.print("* ");
		}
		else {
			Serial.print("- ");
		}
	}

	Serial.println();
}


void changeselectedTrack(byte newMode) {
	scrollOffset = trackScrollOffsets[newMode + altMidiTrack];


	if (newMode != selectedTrack) { //if a track is selected
		clearVertButts();
		selectedTrack = newMode;
		altMidiTrack = SHIFT;
	}
	else if (selectedTrack == 8 && !altMidiTrack) {
		altMidiTrack = true;
		digitalWrite(ZamLed, altMidiTrack);
		scrollOffset = trackScrollOffsets[newMode + altMidiTrack];
		//scrollOffsetSelectorOffset = 1;
	}

	else {						//if the same page is selected again and we are not on ZIM
		selectedTrack = 0;			//go to overview
		altMidiTrack = false;
		digitalWrite(ZamLed, altMidiTrack);
	}

	forceUpdate = true;
	updatePage(selectedTrack);
}

void 	clearVertButts() {
	for (int i = 0; i < 8; i++) {

		launchPad.sendNoteOn(vertButts[i], 0, 1);
		delay(1);
	}
}

void setAllVertButts() {
	for (int i = 0; i < 8; i++) {
		//launchPad.sendNoteOn(vertButts[i], trackColours[i] * !isMuted[i], 1);
		launchPad.sendNoteOn(vertButts[i], trackColours[i] * !(bitRead(isMutedInt, i)), 1); //turn on the led unless corresponding bit in usMuted is 1
		delay(1);
	}
}



void updateVertButts() {
	if (selectedTrack == 0) {
		setAllVertButts();
	}
	else {
		launchPad.sendNoteOn(vertButts[selectedTrack - 1], trackColours[selectedTrack - 1], 1);
		delay(1);
	}
}

