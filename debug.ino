void debug() {
	for (int i = 0; i < 64; i++) {
	int	noteToSendNow = LPMAP[i];
		launchPad.sendNoteOn(noteToSendNow, 127, 1);
		delay(100);
		launchPad.sendNoteOn(noteToSendNow, 0, 1);
	}
}
