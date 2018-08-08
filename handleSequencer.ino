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
		tracksBuffer16x8[track] = seqMatrix[matrixCursor];											//	sendWire2microBitTrackAndNote(seqMatrix[matrixCursor],track);			//send that note to microbit (ask microbit to request it.
		//if (seqMatrix[matrixCursor] > 0){						//if there is a note
			
		//	}
	}
	tracksBuffer16x8[8] = currentStep; //slot number eight is where we send the current step number
	////Serial.println(tracksBuffer16x8[8]);
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