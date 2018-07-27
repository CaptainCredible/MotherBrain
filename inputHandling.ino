byte oldScrollOffset = 0;
byte trackScrollOffsets[10] = { 8,8,8,8,8,8,8,64,64,0 };

void handleFollowIndicator() {
	for (byte i = 0; i < 8; i++) {

		if (follow) {
			launchPad.sendControlChange(topButts[i], followCol, 1);    //set all top LEDs green to say we are in follow mode
		}
		else {
			launchPad.sendControlChange(topButts[i], 0, 1);				//turn them all off to say we are not in follow mode
		}
	}
}

void handleLPNoteOn(byte channel, byte pitch, byte velocity) {
	switch (pitch)
	{
	case 8:
		if (SHIFT) {
			follow = !follow;
			viewPage = currentPage;
			handleFollowIndicator();
			handleTopLeds();												//draw red cursor led on top
		}
		else {
			changePageMode((pitch >> 4) + 1);
		}


		break;
	case 24:
		changePageMode((pitch >> 4) + 1);
		break;
	case 40:
		changePageMode((pitch >> 4) + 1);
		break;
	case 56:
		changePageMode((pitch >> 4) + 1);
		break;
	case 72:
		changePageMode((pitch >> 4) + 1);
		break;
	case 88:
		changePageMode((pitch >> 4) + 1);
		break;
	case 104:
		changePageMode((pitch >> 4) + 1);
		break;
	case 120:
		changePageMode((pitch >> 4) + 1);
		break;
	default:
		byte rowPressed = (pitch >> 4) + scrollOffset;
		byte colPressed = pitch % 8;
		////////////////////////////////////////////////////////////////


		if (pageMode == 0) { //IF WE ARE IN OVERVIEW MODE
			int matrixCursor = LPtoMatrix[pitch] + (currentPage * 8);
			if (seqMatrix[matrixCursor] > 0) {
				seqMatrix[matrixCursor] = 0;
				LPSetLedRaw(pitch, 0);
			}
			else {
				seqMatrix[matrixCursor] = 1;
				LPSetLedRaw(pitch, trackColours[pitch >> 4]);
			}
		}
		////////////////////////////////////////////////////////////////


		else { //if page mode isnt 0, we are not in overview mode and note
			byte trackSelector = pageMode - 1; // this is the track we are writing to basically
			int matrixCursor = LPtoMatrix[pitch % 16] + (currentPage * 8) + trackSelector * matrixTrackOffset; //this is what matrix entry we are editing
			Serial.print("isPoly = ");
			Serial.println(isPoly[trackSelector]);
			if (isPoly[trackSelector]) {								//if this is a polyphonic 8 output track
				if (bitRead(seqMatrix[matrixCursor], rowPressed)) {		//if this bit is set
					bitClear(seqMatrix[matrixCursor], rowPressed);		//clear it
					LPSetLedRaw(pitch, 0);								//turn that LED off
				}
				else {													//if that bit isn't set
					bitSet(seqMatrix[matrixCursor], rowPressed);		//set it
					LPSetLedRaw(pitch, trackColours[trackSelector]);
					//LPSetLed(pitch, trackColours[rowPressed]);		//turn that LED off
				}
				////Serial.print("that entry is now = ");
				////Serial.print(seqMatrix[matrixCursor]);
				////Serial.print(" in binary = ");
				////Serial.println(seqMatrix[matrixCursor], BIN);
			}
			else {														//if this is a mono 127 output track
				Serial.print("rowPressed = ");
				Serial.println(rowPressed);

				unsigned int storedVal = seqMatrix[matrixCursor];
				if (!altMidiTrack) {
					storedVal = storedVal << 8;
				}
				storedVal = storedVal >> 8;							//bad way to bitmask the most significant bits

				storedVal = 127 - storedVal;							//invert back to to normal.

				byte valToWrite = 0;


				if (storedVal == rowPressed + 1) {
					valToWrite = 0;
				}
				else {
					valToWrite = rowPressed + 1;
					//seqMatrix[matrixCursor] = rowPressed+1;
				}

				valToWrite = 127 - valToWrite;						//flip to make grid right way round

				if (!altMidiTrack) {
					seqMatrix[matrixCursor] = seqMatrix[matrixCursor] & 0b1111111100000000;  //clear LSB
					seqMatrix[matrixCursor] = seqMatrix[matrixCursor] | valToWrite;			// put our valToWrite in LSB
				}
				else {
					seqMatrix[matrixCursor] = seqMatrix[matrixCursor] & 0b0000000011111111;  //clear MSB
					seqMatrix[matrixCursor] = seqMatrix[matrixCursor] | valToWrite << 8;		//put our valtowrite in MSB
					Serial.print("wrote to alt track");
					Serial.println(seqMatrix[matrixCursor >> 8]);
				}




				forceUpdate = true;
				updatePage(pageMode);
			}
			//forceUpdate = true;
			//updatePage(pageMode);
		}
		break;
	}


	//handleTopLeds();
}

void handleLPNoteOff(byte channel, byte pitch, byte velocity) {

}

void handleLPCC(byte channel, byte CC, byte val) {
	byte buttWasPressed = CC - 103;
	if (val > 0) {
		////Serial.println(buttWasPressed);
		switch (buttWasPressed)
		{
		case 1:
			if (SHIFT) { //SET no of pages to 2
				seqLength = 8;
				currentStep = currentStep % seqLength;
			}
			break;

		case 2:
			if (SHIFT) { //SET no of pages to 2
				seqLength = 16;
				currentStep = currentStep % seqLength;
			}
			break;

		case 3:
			if (SHIFT) { //SET no of pages to 3
				seqLength = 24;
				currentStep = currentStep % seqLength;
			}
			break;

		case 4:
			if (SHIFT) { //SET no of pages to 4
				seqLength = 32;
				currentStep = currentStep % seqLength;
			}
			else {
				//digitalWrite(ledBpin, follow);
			}
			break;

		case 5:
			if (SHIFT) { //SET timesig to 5/8
				timeSig = 3;
			}
			else {
				if (pageSelect == 0) {				//if we are already at page 0
					pageSelect = numberOfPages - 1;	//go back to last page
				}
				else {
					pageSelect--;					//else decrement page by one
				}
				updatePage(pageMode);
				////Serial.print("pageSelect = ");
				////Serial.println(pageSelect);
			}
			break;

		case 6:
			if (SHIFT) {
				timeSig = 2; //set timesig to 3/3
			}
			else {
				pageSelect++;
				if (pageSelect >= numberOfPages) {
					pageSelect = 0;
				}
				updatePage(pageMode);
				//Serial.print("pageSelect = ");
				//Serial.println(pageSelect);
			}
			break;

		case 7:
			if (SHIFT) {
				timeSig = 1; //set timeSig to 7/8
			}
			else {
				oldScrollOffset = scrollOffset;
				scrollOffset--;
				limitScrollOffset();
				trackScrollOffsets[pageMode] = scrollOffset; // remember settings per track to be recalled when pagemode changes
				forceUpdate = true;
				updatePage(currentPage);
				Serial.print("scrollOffset = ");
				Serial.println(scrollOffset);
			}
			break;
		case 8:
			if (SHIFT) {
				timeSig = 0; //set timeSig to 4/4;
			}
			else {
				oldScrollOffset = scrollOffset;
				scrollOffset++;
				limitScrollOffset();
				trackScrollOffsets[pageMode] = scrollOffset; // remember settings per track to be recalled when pagemode changesfg
				forceUpdate = true;
				updatePage(currentPage);
				Serial.print("scrollOffset = ");
				Serial.println(scrollOffset);
			}
			
			break;
		default:
			trackToSend = buttWasPressed - 1;
			////Serial.print("trackToSend = ");
			////Serial.println(trackToSend);
			break;
		}
	}
}

void limitScrollOffset() {
	Serial.print("ispoly = ");
	Serial.println(isPoly[pageMode - 1]);
	if (isPoly[pageMode - 1]) {
		if (scrollOffset > 8) {

			scrollOffset = oldScrollOffset;   //limit scrollOffset to 0-7
		}
	}
	else {
		if (scrollOffset > 127) {
			scrollOffset = oldScrollOffset;   //limit scrollOffset to 0-7
		}
	}
}

#define minStepDuration 33

void handleKnobsAndButtons() {
	bool oldKnobA = knobA;
	knobA = analogRead(A1);

	bool oldKnobB = knobB;
	knobB = analogRead(A0);

	bool oldButtA = buttA;
	buttA = !digitalRead(buttApin);

	bool oldButtB = buttB;
	buttB = !digitalRead(buttBpin);

	bool oldButtC = buttC;
	buttC = !digitalRead(buttCpin);

	bool oldButtX = buttX;
	buttX = !digitalRead(buttXpin);
	SHIFT = buttX;
	//stepDuration = ((2048+minStepDuration) - (knobA << 1));

	if (buttA && !oldButtA) {
		if (!SHIFT) {
			runClock = !runClock;
		}
		else {
			currentStep = -1;
			clockTimer = -1;
			updatePage(pageMode);
		}
	}

	if (buttB) {
		int BPM = 10 + (knobB >> 2);
		int beatLength = 60000 / BPM;
		stepDuration = beatLength >> 3;
		Serial.print("BPM = ");
		Serial.println(BPM);
		Serial.print("beatLength = ");
		Serial.println(beatLength);
		Serial.print("Stepduration = ");
		Serial.println(stepDuration);
		Serial.println();
		Serial.println();

	}

	if (buttC && !oldButtC) {
		unsigned long now = millis();
		if (now - tapTempoTimer < 1000 && now - tapTempoTimer>minStepDuration * 2) {
			stepDuration = now - tapTempoTimer;
		}

		tapTempoTimer = now;

		if (!runClock) {
			lastStep = currentStep;
			if (SHIFT) {
				currentStep--;
			}
			else {
				currentStep++;
			}
			currentStep = currentStep % seqLength;
			handleStep();
			updatePage(pageMode);
		}
	}

	////Serial.print("stepDuration ");
	////Serial.println(stepDuration);
	//	//Serial.println(buttX);
}