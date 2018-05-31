
void handleStep() {
	//Serial.print("step is ");
	//Serial.println(currentStep);
	for (byte track = 0; track < 16; track++) {					//repeat for every track

		int matrixCursor = currentStep + (track * 20);			//check current step for notes
		//Serial.print(matrixCursor);
		//Serial.print("-");
		//Serial.print(seqMatrix[matrixCursor]);
		//Serial.print("   ");
		if (seqMatrix[matrixCursor] > 0){						//if there is a note
			sendWire2microBitTrackAndNote(track,seqMatrix[matrixCursor]);			//send that note to microbit (ask microbit to request it.
			Serial.print("i sent that and it was = ");
			Serial.println(seqMatrix[matrixCursor]);
			}
	}
	//Serial.println(" end");
}




