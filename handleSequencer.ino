void handleTimeSig() {
	if (currentStep % 8 >= 8 - timeSig) {
		currentStep += timeSig;
	}
}

void calculateStartAndEndStep() {
	startStep = desiredStartStep;
	if (desiredStartStep >= desiredEndStep) {
		endStep = startStep + 8;
	}
	else {
		endStep = desiredEndStep;
	}
	/*
	Serial.print("desiredStartStep = ");
	Serial.print(desiredStartStep);
	Serial.print("   desiredEndStep = ");
	Serial.println(desiredEndStep);

	Serial.print("actualStartStep = ");
	Serial.print(startStep);
	Serial.print("   actualEndStep = ");
	Serial.println(endStep);
	*/


}

void handleClock() {
	if (runClock) {
		unsigned long now = millis();
		//Serial.print("clocktimer = ");
		//Serial.println(clockTimer);
		//Serial.print("        Now = ");
		//Serial.println(now);
		if (now >= clockTimer + stepDuration) {
			int diff = now - (clockTimer + stepDuration); // find out if we overshot so we can avoid drifting and instead have just a spot of jitter
			if (diff > 5 || diff < 0) {									//avoid adjusting for diff when there are other delays causing problems
				diff = 0;
			}
			clockTimer = now - diff;					  //Set clocktimer to what it should have been
			lastStep = currentStep;
			currentStep++;

			calculateStartAndEndStep();

			if (currentStep >= endStep) {
				currentStep = startStep;
			}
			handleTimeSig();
			//currentStep = currentStep % endStep;
			//currentStep = currentStep % seqLength;
			if (currentStep >> 3 != currentPageThatIsPlaying) {
				previousPageThatWasPlaying = currentPageThatIsPlaying;
				currentPageThatIsPlaying = currentStep >> 3;
			}
			handleStep();
			updatePage(pageMode);
			//Serial.print("diff = ");
			//Serial.println(diff);
		}
	}
}

void handleStep() {
	for (byte track = 0; track < numberOfTracks; track++) {					//repeat for every track 0-15
		int matrixCursor = currentStep + (track * matrixTrackOffset);			//check current step for notes
		//if (isMuted[track]) {
		if (bitRead(isMutedInt, track)) {
			tracksBuffer16x8[track] = 0;
		}
		else {
			if (isPoly[track]) {

				//code to invert notes sent
				uint16_t intermedium = seqMatrix[matrixCursor];
				for (int i = 0; i < 16; i++) {
					if (bitRead(intermedium, i)) {
						bitSet(tracksBuffer16x8[track], 15 - i);
					}
					else {
						bitClear(tracksBuffer16x8[track], 15 - i);
					}
				}

				//tracksBuffer16x8[track] = intermedium;	//added a test to see if its muted				//	sendWire2microBitTrackAndNote(seqMatrix[matrixCursor],track);			//send that note to microbit (ask microbit to request it.
			}
			else {
				tracksBuffer16x8[track] = (seqMatrix[matrixCursor]);	//added a test to see if its muted				//	sendWire2microBitTrackAndNote(seqMatrix[matrixCursor],track);			//send that note to microbit (ask microbit to request it.
			}
		}

	}
	tracksBuffer16x8[8] = currentStep; //slot number eight is where we send the current step number
	tracksBuffer16x8[9] = isMutedInt;  //slot 9 is where the mutes are stored
	sendTracksBuffer();
	////Serial.println(tracksBuffer16x8[8]);

}

void clearTracksBuffer() {
	for (int i = 0; i < 16; i++) {
		tracksBuffer[i] = 0;
	}
}

void radioSendClockTick() {
	//sendWire2microBitTrackAndNote(101, 0);			//send that note to microbit (ask microbit to request it.
}

void radioSendStartAndTempo(byte tempo) {
	//sendWire2microBitTrackAndNote(102, tempo);			//send that note to microbit (ask microbit to request it.
}

void storeSeq() {
	clearVertButts();
	for (int i = 0; i < 256; i++) {
		uint8_t MSB = seqMatrix[i] >> 8;
		uint8_t LSB = seqMatrix[i] && 0b0000000011111111;
		int writeCursor = i * 2;
		EEPROM.write(writeCursor, MSB);
		EEPROM.write(writeCursor + 1, LSB);
	}
	for (int i = 0; i < 8; i++) {
		launchPad.sendNoteOn(vertButts[i], trackColours[i], 1);
		delay(100);
		//////Serial.println(i);
	}
	EEPROM.write(1000, 123);
	Serial.println("wrote 123");
	Serial.print("Read:  ");
	Serial.println(EEPROM.read(1000));
	updateVertButts();
}

void recallSeq() {
	clearVertButts();
	int loadingBar = 0;
	for (int i = 0; i < 256; i++) {
		
		int readCursor = i * 2;
		uint8_t MSB = EEPROM.read(readCursor);
		uint8_t LSB = EEPROM.read(readCursor + 1);
		uint16_t seqValToWrite = (MSB << 8) + LSB;
		seqMatrix[i] = seqValToWrite;
		//delay(10);
		//if (i >> 5 > loadingBar) {
		//	loadingBar = i >> 5;
		//	launchPad.sendNoteOn(vertButts[i - 1], trackColours[i - 1], 1);

		//}
	}
	updateVertButts();

}

