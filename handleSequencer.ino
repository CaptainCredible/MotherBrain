void handleClock() {
	if (runClock) {
		if (millis() > clockTimer + stepDuration) {
			clockTimer = millis();
			lastStep = currentStep;
			currentStep++;
			currentStep = currentStep % seqLength;
			handleStep();
			updatePage(pageMode);
		}
	}
}

void handleStep() {
	for (byte track = 0; track < numberOfTracks; track++) {					//repeat for every track 0-15
		int matrixCursor = currentStep + (track * matrixTrackOffset);			//check current step for notes
		tracksBuffer16x8[track] = seqMatrix[matrixCursor];											//	sendWire2microBitTrackAndNote(seqMatrix[matrixCursor],track);			//send that note to microbit (ask microbit to request it.
		//if (seqMatrix[matrixCursor] > 0){						//if there is a note
			
		//	}
	}
	sendTracksBuffer();
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