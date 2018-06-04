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
	for (byte track = 0; track < 8; track++) {					//repeat for every track
		int matrixCursor = currentStep + (track * 20);			//check current step for notes
		if (seqMatrix[matrixCursor] > 0){						//if there is a note
			tracksBuffer[track] = seqMatrix[matrixCursor];											//	sendWire2microBitTrackAndNote(seqMatrix[matrixCursor],track);			//send that note to microbit (ask microbit to request it.
			}
	}
	sendTracksBuffer64();
	clearTracksBuffer();
	//Serial.print("stepDuration =");
	//Serial.println(stepDuration);
	//radioSendBuffer();
}




void clearTracksBuffer() {
	for (int i = 0; i < 8; i++) {
		tracksBuffer[i] = 0;
	}
}

void radioSendClockTick() {
	sendWire2microBitTrackAndNote(101, 0);			//send that note to microbit (ask microbit to request it.
}

void radioSendStartAndTempo(byte tempo) {
	sendWire2microBitTrackAndNote(102, tempo);			//send that note to microbit (ask microbit to request it.
}