
void handleStep() {
	for (byte track = 0; track < 16; track++) {					//repeat for every track
		int matrixCursor = currentStep + (track * 20);			//check current step for notes
		if (seqMatrix[matrixCursor] > 0){						//if there is a note
			sendWire2microBitTrackAndNote(seqMatrix[matrixCursor],track);			//send that note to microbit (ask microbit to request it.
			}
	}
	
	//radioSendBuffer();
}
void radioSendBuffer() {
	sendWire2microBitTrackAndNote(100, 0);			//send that note to microbit (ask microbit to request it.
	Serial.println(" end");
}

void radioSendClockTick() {
	sendWire2microBitTrackAndNote(101, 0);			//send that note to microbit (ask microbit to request it.
}

void radioSendStartAndTempo(byte tempo) {
	sendWire2microBitTrackAndNote(102, tempo);			//send that note to microbit (ask microbit to request it.
}