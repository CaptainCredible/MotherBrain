byte oldScrollOffset = 0;
byte trackScrollOffsets[11] = { 8,8,8,8,8,8,8,8,64,64,0 };

bool musMutesPage = false;

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


void handlePageButtons(byte buttonToHandle) {
	if (SHIFT) {
		if (selectedTrack == 0) {
			if (bitRead(isMutedInt, buttonToHandle >> 4)) {
				bitClear(isMutedInt, buttonToHandle >> 4);
			}
			else {
				bitSet(isMutedInt, buttonToHandle >> 4);
			}
			//Serial.println(isMutedInt, BIN);
			sendMutes();
			setAllVertButts();
		}
		else {
			//Serial.print("selectedTrack = ");
			//Serial.println(selectedTrack);
			if (selectedTrack != 8 & selectedTrack != 9) {
				triggerImmediately(selectedTrack - 1, 15 - ((buttonToHandle >> 4) + scrollOffset));
			}
			else { //if its a mono midi track
				uint16_t myNumber = (buttonToHandle >> 4);
				//Serial.print("myNumber = ");
				//Serial.println(myNumber);

				triggerImmediately(selectedTrack - 1, myNumber);
			}
		}

	}
	else {
		changeselectedTrack((buttonToHandle >> 4) + 1);
	}

}

void handleLPNoteOn(byte channel, byte pitch, byte velocity) {
	switch (pitch)
	{


	case 8: //bob
		handlePageButtons(pitch);
		break;


	case 24: //Tim
		handlePageButtons(pitch);
		break;


	case 40: //Ted
		handlePageButtons(pitch);
		break;


	case 56: //Pat
		handlePageButtons(pitch);
		break;


	case 72: //Cat
		handlePageButtons(pitch);
		break;


	case 88: //Dad
		handlePageButtons(pitch);
		break;


	case 104: //Mum
		handlePageButtons(pitch);
		break;


	case 120: //zim
		handlePageButtons(pitch);
		break;

	case 116: //musicianmutes work in progress
		if (SHIFT) {
			//musMutesPage = !musMutesPage;
			//Serial.println(musMutesPage);
			//updatePage(selectedTrack);
			break;
		}
	case 112: // Save / storeseq
		if (SHIFT) {
			storeSeq();
			break;
		}

	case 114: //nuke / clearseqpage
		if (SHIFT) {
			clearSeqPage();
			break;
		}

	case 113: // loadseq
		if (SHIFT) {
			recallSeq();
			forceUpdate = true;
			updatePage(selectedTrack);
			break;
		}

		
	default:
		//Serial.println(pitch);
		byte rowPressed = (pitch >> 4) + scrollOffset;
		byte colPressed = pitch % 8;
		////////////////////////////////////////////////////////////////
		if (selectedTrack == 0) { //IF WE ARE IN OVERVIEW MODE
			int matrixCursor = LPtoMatrix[pitch] + (currentPage * 8);
			
			if (seqMatrixShift) {
				matrixCursor += 256;
			}

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
			byte matrixTrackSelector = selectedTrack - 1; // this is the track we are writing to basically
			int matrixCursor = LPtoMatrix[pitch % 16] + (currentPage * 8) + matrixTrackSelector * matrixTrackOffset; //this is what matrix entry we are editing
			if (seqMatrixShift) {
				matrixCursor += 256;
			}
			//////Serial.print("isPoly = ");
			//////Serial.println(isPoly[matrixTrackSelector]);
			if (isPoly[matrixTrackSelector]) {								//if this is a polyphonic 8 output track
				if (bitRead(seqMatrix[matrixCursor], rowPressed)) {		//if this bit is set
					bitClear(seqMatrix[matrixCursor], rowPressed);		//clear it
					LPSetLedRaw(pitch, 0);								//turn that LED off
				}
				else {													//if that bit isn't set
					bitSet(seqMatrix[matrixCursor], rowPressed);		//set it
					LPSetLedRaw(pitch, trackColours[matrixTrackSelector]);
					//LPSetLed(pitch, trackColours[rowPressed]);		//turn that LED off
				}
				//////////Serial.print("that entry is now = ");
				//////////Serial.print(seqMatrix[matrixCursor]);
				//////////Serial.print(" in binary = ");
				//////////Serial.println(seqMatrix[matrixCursor], BIN);
			}
			else {														//if this is a mono 127 output track
				//////Serial.print("rowPressed = ");
				//////Serial.println(rowPressed);

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
					//////Serial.print("wrote to alt track");
					//////Serial.println(seqMatrix[matrixCursor >> 8]);
				}

				forceUpdate = true;
				updatePage(selectedTrack);
			}

		}
		break;
	}

}

void handleLPNoteOff(byte channel, byte pitch, byte velocity) {
}



void handleLPCC(byte channel, byte CC, byte val) {
	byte buttWasPressed = CC - 103;
	if (val > 0) {
		//////////Serial.println(buttWasPressed);
		if (!SHIFT) {
			switch (buttWasPressed)
			{
			case 1:
				if (selectedTrack != 0 && globalPolyRhythmEnable && polyRhythm[selectedTrack]) {
					desiredPolyStartStep[selectedTrack] = 0;
				}
				else {
					desiredStartStep = 0;
				}

				break;

			case 2:
				if (selectedTrack != 0 && globalPolyRhythmEnable && polyRhythm[selectedTrack]) {
					desiredPolyStartStep[selectedTrack] = 8;
				}
				else {
					desiredStartStep = 8;
				}
				break;

			case 3:
				if (selectedTrack != 0 && globalPolyRhythmEnable && polyRhythm[selectedTrack]) {
					desiredPolyStartStep[selectedTrack] = 16;
				}
				else {
					desiredStartStep = 16;
				}
				break;

			case 4:
				if (selectedTrack != 0 && globalPolyRhythmEnable && polyRhythm[selectedTrack]) {
					desiredPolyStartStep[selectedTrack] = 24;
				}
				else {
					desiredStartStep = 24;
				}
				break;

			case 5:
					if (pageSelect == 0) {				//if we are already at page 0
						pageSelect = numberOfPages - 1;	//go back to last page
					}
					else {
						pageSelect--;					//else decrement page by one
					}
					updatePage(selectedTrack);
				break;

			case 6:
					pageSelect++;
					if (pageSelect >= numberOfPages) {
						pageSelect = 0;
					}
					updatePage(selectedTrack);
					////////Serial.print("pageSelect = ");
					////////Serial.println(pageSelect);
				break;

			case 7:
				if (selectedTrack != 0) {
					oldScrollOffset = scrollOffset;
					scrollOffset--;
					limitScrollOffset();
					trackScrollOffsets[selectedTrack + altMidiTrack] = scrollOffset; // remember settings per track to be recalled when selectedTrack changes
					forceUpdate = true;
					updatePage(selectedTrack);
				}
				else {
					if (seqMatrixShift) {
						seqMatrixShift = false;
						forceUpdate = true;
						updatePage(selectedTrack);
					}
				}
					

				break;
			case 8:
				if (selectedTrack != 0) {
					oldScrollOffset = scrollOffset;
					scrollOffset++;
					limitScrollOffset();
					trackScrollOffsets[selectedTrack + altMidiTrack] = scrollOffset; // remember settings per track to be recalled when selectedTrack changesfg
					forceUpdate = true;
					updatePage(currentPage);
				}
				else {
					if (!seqMatrixShift) {
						seqMatrixShift = true;
						forceUpdate = true;
						updatePage(currentPage);
					}
				}
					
				break;
			default:
				trackToSend = buttWasPressed - 1;
				break;

			}
			calculateStartAndEndSteps();
			calculateSeqLengths();
			currentStep = currentStep % seqLength;


			if (currentStep < startStep || currentStep > endStep) {
				currentStep = startStep + (currentStep % 8);
				updatePage(selectedTrack);
			}
			if (globalPolyRhythmEnable) {
				if (polyCurrentStep[selectedTrack] < polyStartStep[selectedTrack] || polyCurrentStep[selectedTrack] > polyEndStep[selectedTrack]) {
					polyCurrentStep[selectedTrack] = polyStartStep[selectedTrack] + (polyCurrentStep[selectedTrack] % 8);
					updatePage(selectedTrack);
				}
			}
			// ADD POLY VERSION OF ABOVE CODE (I DID)


		}
		else {
			////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			///////////// SHIFT ///////////////////////////////////////////////////////////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

			switch (buttWasPressed)
			{
			case 1:
				if (selectedTrack != 0 && globalPolyRhythmEnable && polyRhythm[selectedTrack]) {
					desiredPolyEndStep[selectedTrack] = 8;
				}
				else {
					desiredEndStep = 8;
				}

				break;

			case 2:
				if (selectedTrack != 0 && globalPolyRhythmEnable && polyRhythm[selectedTrack]) {
					desiredPolyEndStep[selectedTrack] = 16;
				}
				else {
					desiredEndStep = 16;
				}
				break;

			case 3:
				if (selectedTrack != 0 && globalPolyRhythmEnable && polyRhythm[selectedTrack]) {
					desiredPolyEndStep[selectedTrack] = 24;
				}
				else {
					desiredEndStep = 24;
				}
				break;

			case 4:
				if (selectedTrack != 0 && globalPolyRhythmEnable && polyRhythm[selectedTrack]) {
					desiredPolyEndStep[selectedTrack] = 32;
				}
				else {
					desiredEndStep = 32;
				}
				break;

			case 5:
				timeSig = 3;
				handleTimeSigDisplay();
				oldTimeSig = 3;
				cleanupLeds();
				break;

			case 6:
				timeSig = 2; //set timesig to 3/3
				handleTimeSigDisplay();
				oldTimeSig = 2;
				cleanupLeds();
				break;

			case 7:
				timeSig = 1; //set timeSig to 7/8
				handleTimeSigDisplay();
				oldTimeSig = 1;
				cleanupLeds();

				break;
			case 8:
				timeSig = 0; //set timeSig to 4/4;
				handleTimeSigDisplay();
				oldTimeSig = 0;
				cleanupLeds();

				break;
			default:
				trackToSend = buttWasPressed - 1;
				break;
			}
			calculateStartAndEndSteps();
			calculateSeqLengths();
			if (currentStep < startStep || currentStep > endStep) {
				currentStep = startStep + (currentStep % 8);
				updatePage(selectedTrack);
			}
		}
	}
}

void limitScrollOffset() {
	//////Serial.print("ispoly = ");
	//////Serial.println(isPoly[selectedTrack - 1]);
	if (isPoly[selectedTrack - 1]) {
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
	int oldKnobA = knobA;
	knobA = analogRead(A1);
	////Serial.println(analogRead(A1));

	//int oldKnobB = knobB;
	//knobB = analogRead(A0);
	bool oldFollow = follow;
	follow = digitalRead(followSwitch);
	if (follow != oldFollow) {
		digitalWrite(followLed, follow);
		viewPage = currentPage;
		//handleFollowIndicator();
		handleTopLeds();												//draw red cursor led on top
	}

	bool oldButtA = buttA;
	buttA = !digitalRead(buttApin);

	bool oldButtB = buttB;
	buttB = !digitalRead(buttBpin);

	bool oldButtC = buttC;
	buttC = !digitalRead(buttCpin);

	bool oldButtX = buttX;
	buttX = !digitalRead(buttXpin);
	SHIFT = buttX;
	digitalWrite(polyRhythmLed, polyRhythm[selectedTrack] && globalPolyRhythmEnable);
	//stepDuration = ((2048+minStepDuration) - (knobA << 1));

	if (buttA && !oldButtA) {
		if (!SHIFT) {
			if (!midiClockRunning) {
				runClock = !runClock;
			}

			if (runClock) {
				//midiClockRunning = false;
			}
		}
		else {
			masterStep = -1;
			currentStep = -1;
			clockTimer = -1;
			midiClockCounter = -1;
			//	clearTopLedsArray();
			//	handleTopLeds();

				//updatePage(selectedTrack);
		}
	}

	if (buttB) {
		if (SHIFT) {
			//storeSeq();
			//storeSeqAlt();
			//EEPROMTest();
		}
		else {
			if (!midiClockRunning) {
				int BPM = 10 + (knobA >> 2);
				int beatLength = 60000 / BPM;
				stepDuration = beatLength >> 3;
			}
			else { //if midi clock is running
				midiClockDiv = 3 + ((knobA >> 8) * 3);
			}

		}
	}

	if (buttC && !oldButtC) {
		unsigned long now = millis();

		if (SHIFT) {
			polyRhythm[selectedTrack] = !polyRhythm[selectedTrack];
			globalPolyRhythmEnable = polyRhythm[0];
		}
		else {
			if (now - tapTempoTimer < 1000 && now - tapTempoTimer>minStepDuration * 2) {
				stepDuration = now - tapTempoTimer;
			}

			tapTempoTimer = now;

			if (!runClock) {
				lastStep = currentStep;
				currentStep++;
				currentStep = currentStep % seqLength;
				handleStep();
				updatePage(selectedTrack);
			}
		}

	}

	//////////Serial.print("stepDuration ");
	//////////Serial.println(stepDuration);
	//	////////Serial.println(buttX);
}

