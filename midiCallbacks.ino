byte oldScrollOffset = 0;
byte trackScrollOffsets[10] = { 8,8,8,8,8,8,8,64,64,0 };


void handleLPNoteOn(byte channel, byte pitch, byte velocity) {
	switch (pitch)
	{
	case 8:
		changePageMode((pitch >> 4)+1);
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
			int matrixCursor = LPtoMatrix[pitch % 16] + (currentPage * 8) + trackSelector *matrixTrackOffset; //this is what matrix entry we are editing
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
					
				
				byte valToWrite = 0;

				if (storedVal == rowPressed+1) {
					valToWrite = 0;
				}
				else {
					valToWrite = rowPressed + 1;
					//seqMatrix[matrixCursor] = rowPressed+1;
				}

				if (!altMidiTrack) {
					seqMatrix[matrixCursor] = seqMatrix[matrixCursor] & 0b1111111100000000;  //clear LSB
					seqMatrix[matrixCursor] = seqMatrix[matrixCursor] | valToWrite;			// put our valToWrite in LSB
				} else {
					seqMatrix[matrixCursor] = seqMatrix[matrixCursor] & 0b0000000011111111;  //clear MSB
					seqMatrix[matrixCursor] = seqMatrix[matrixCursor] | valToWrite<<8;		//put our valtowrite in MSB
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
			if (!SHIFT) {
				runClock = !runClock;
			}
			else {
				currentStep = -1;
				clockTimer = -1;
				updatePage(pageMode);
			}			
			break;

		case 2:
			if (SHIFT) { //SET no of pages to 1
				seqLength = 8;
				currentStep = currentStep%seqLength;
			}
			break;

		case 3:
			if (SHIFT) { //SET no of pages to 2
				seqLength = 16;
				currentStep = currentStep%seqLength;
			}
			break;

		case 4: //this is the follow on off button
			if (SHIFT) { //SET no of pages to 3
				seqLength = 24;
				currentStep = currentStep%seqLength;
			} else {
			follow = !follow;
			launchPad.sendControlChange(107, 32 * follow, 1);
			//digitalWrite(ledBpin, follow);
			}
			break;

		case 5:
			if (SHIFT) { //SET no of pages to 4
				seqLength = 32;
				currentStep = currentStep%seqLength;
			}
			else {
			if (pageSelect== 0){				//if we are already at page 0
				pageSelect = numberOfPages-1;	//go back to last page
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
			pageSelect++;
			if (pageSelect >= numberOfPages) {
				pageSelect = 0;
			}
			updatePage(pageMode);
			//Serial.print("pageSelect = ");
			//Serial.println(pageSelect);
			break;

		case 7:
			oldScrollOffset = scrollOffset;
			scrollOffset--;
			limitScrollOffset();
			trackScrollOffsets[pageMode] = scrollOffset; // remember settings per track to be recalled when pagemode changes
			forceUpdate = true;
			updatePage(currentPage);
			Serial.print("scrollOffset = ");
			Serial.println(scrollOffset);
			break;
		case 8:
			oldScrollOffset = scrollOffset;
			scrollOffset++;
			limitScrollOffset();
			trackScrollOffsets[pageMode] = scrollOffset; // remember settings per track to be recalled when pagemode changesfg
			forceUpdate = true;
			updatePage(currentPage);
			Serial.print("scrollOffset = ");
			Serial.println(scrollOffset);
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
	Serial.println(isPoly[pageMode-1]);
	if (isPoly[pageMode-1]) {
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