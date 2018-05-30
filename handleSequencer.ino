
void handleStep() {
	for (byte track = 0; track < 16; track++) {					//repeat for every track
		int matrixCursor = currentStep + (track * 20);			//check current step for notes
		if (seqMatrix[matrixCursor > 0]){						//if there is a note
			sendWire2microBit(seqMatrix[matrixCursor]);			//send that note to microbit (ask microbit to request it.
			}
	}
}


