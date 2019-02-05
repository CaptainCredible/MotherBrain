byte oldPageMode = 0;

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
		if (follow) {    //if we are in follow mode, the page should correspond to the page the cursor is on. 
			if (currentPage != (currentStep >> 3) || forceUpdate || pageMode != oldPageMode) { //Time to flip the page
				
				clearPage();
				oldPageMode = mode;
				forceUpdate = false;
				//digitalWrite(ledApin, forceUpdate);
				if (prevPage != currentPage) {
					////Serial.println("PING");
					//displayPageNumber();	
				}
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
		handleTopLeds();
	}

	void handleTrackPage(byte trackPageToHandle) {

		byte trackPageWeAreHandling = trackPageToHandle - 1; //move 1-8 to 0-7
		if (isPoly[trackPageWeAreHandling]) {
			////Serial.println("drawing poly track");
			for (byte col = 0; col < 8; col++) {
				int seqMatrixCursor = col + (currentPage * 8) + (trackPageWeAreHandling * matrixTrackOffset);
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
			////Serial.println("drawing MIDI track");
			for (byte col = 0; col < 8; col++) {																				// use col to handle what column we are drawing on left to right
				int seqMatrixCursor = col + (currentPage * 8) + (trackPageWeAreHandling * matrixTrackOffset);					// find right seqmatrix slot to read from
				unsigned int rawVal = seqMatrix[seqMatrixCursor];																// 
				if (!altMidiTrack) {

					//rawVal = 0b0000000011111111 & rawVal;  //use only last 8 bits of int
					rawVal = rawVal << 8;					//hacky way of clearing first 8 bits works because we are bitshifting back in next operation
				}

				rawVal = rawVal >> 8;					//use first 8 bits

				if (col == 0) {							//debug
					////Serial.print("rawVal");
					////Serial.println(rawVal);
				}
				byte invertedVal = 127 - rawVal;			//invert to make the grid the right way round
				byte val = invertedVal - scrollOffset;			// translate to our viewing space by acounting for how far we have scrolled

				if (col == 0) {
					////Serial.print("val = ");
					////Serial.println(val);
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
		////////Serial.println("CLEARED PAGE");
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

	void clearTopLedsArray() { //clear the array that helps us keep track of what leds have and have not already been set
		for (int i = 0; i < 8; i++) {
			topLedWasSet[i] = 0;
		}
	}



	void handleTopLeds() {
		clearTopLedsArray(); //clear the array that helps us keep track of what leds have and have not already been set and also clear the leds
		//handleWhiteLeds();
		handleCurrentPageDisplay();
		handleCurrentPagePlayingDisplay();
		//handleFollowIndicator();
		handleTimeSigDisplay();
		handleCursor();
		byte colChanger = 0;
		/*
		if (follow) {
			if (currentStep%8 == 3){
				colChanger = 1;
			}
			launchPad.sendControlChange(107, 32-colChanger, 1);
		}
		*/
	}

	void handleWhiteLeds() {  //these leds show what page we are on (up to four)
		byte pageThatSeqIsOn = currentStep >> 3;
		digitalWrite(ledBpin, bitRead(pageThatSeqIsOn, 0));
		digitalWrite(ledApin, bitRead(pageThatSeqIsOn, 1));
	}

#define currentPageDisplayCol 28
	void handleCurrentPageDisplay() {
		if (!follow) { // no point displaying this if it is the same as playing page

			launchPad.sendControlChange(topButts[currentPage], currentPageDisplayCol, 1);
			topLedWasSet[currentPage] = true;
		}
		
	}
#define currentPagePlayingCol 33
	void handleCurrentPagePlayingDisplay() {
		////Serial.print("handledTopLeds page ");
		////Serial.println(currentPage);
		if (!topLedWasSet[currentPageThatIsPlaying]) {
			launchPad.sendControlChange(topButts[currentPageThatIsPlaying], currentPagePlayingCol, 1);
		}
		
		if (!topLedWasSet[previousPageThatWasPlaying]) { //if the led corresponding to the previous page was not set by a previous topledset routine
			launchPad.sendControlChange(topButts[previousPageThatWasPlaying], 0, 1);	//turn it off
		}
		topLedWasSet[currentPage] = true;
	}

#define timeSigDisplayCol 99
	void handleTimeSigDisplay() {
		if (oldTimeSig != timeSig) {
			launchPad.sendControlChange(topButts[7 - oldTimeSig], 0, 1);
			oldTimeSig = timeSig;
		}
		if (true){//(timeSig > 0) {
			launchPad.sendControlChange(topButts[7 - timeSig],timeSigDisplayCol, 1);
			topLedWasSet[7 - timeSig] = true;
		}
	}

	void handleCursor() {
		//byte colChanger = 0;
		if (currentPage == currentStep >> 3) {
			if (!topLedWasSet[lastStep % 8]) {
				launchPad.sendControlChange(topButts[lastStep % 8], 0, 1);
			}
			launchPad.sendControlChange(topButts[currentStep % 8], seqLedColour, 1);  //sets seqstepled regardless of whether they are on or not
		}
		else if (currentPage == lastStep >> 3) {
			launchPad.sendControlChange(topButts[lastStep % 8], 0, 1);
			////Serial.println(lastStep);
		}

	}

	

	void changePageMode(byte newMode) {
		////Serial.print("changed pagemode to ");
		scrollOffset = trackScrollOffsets[newMode + altMidiTrack];
		//Serial.print("scrollOffset = ");
		//Serial.println(trackScrollOffsets[newMode + altMidiTrack]);

		if (newMode != pageMode) { //if a track is selected
			clearVertButts();
			pageMode = newMode;
			////Serial.print("pagemode = ");
			////Serial.println(pageMode);
			altMidiTrack = SHIFT;
			////Serial.print("isPoly = ");
			////Serial.println(isPoly[pageMode-1]);
		}
		else if (pageMode == 8 && !altMidiTrack) {
			altMidiTrack = true;
			digitalWrite(ZamLed, altMidiTrack);
			scrollOffset = trackScrollOffsets[newMode + altMidiTrack];
			//scrollOffsetSelectorOffset = 1;
		}

		else {						//if the same page is selected again and we are not on ZIM
			pageMode = 0;			//go to overview
			altMidiTrack = false;
			digitalWrite(ZamLed, altMidiTrack);
		}
		////Serial.print("pageMode = ");
		////Serial.println(pageMode);
		forceUpdate = true;
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
			//launchPad.sendNoteOn(vertButts[i], trackColours[i] * !isMuted[i], 1);
			launchPad.sendNoteOn(vertButts[i], trackColours[i] * !(bitRead(isMutedInt, i)), 1); //turn on the led unless corresponding bit in usMuted is 1
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

