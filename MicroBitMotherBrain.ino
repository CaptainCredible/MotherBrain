
#include <MIDI.h>
#include <Wire.h>

#define LEDPIN 10
#define interruptPin 16
byte colour = 45;
byte noteToSend = 123;

//MIDI.CREATE_DEFAULT_INSTANCE();
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, launchPad);


byte seqMatrix[320]{
	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	127,127,127,127,
	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	127,127,127,127,
	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	127,127,127,127,
	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	127,127,127,127,
	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	127,127,127,127,
	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	127,127,127,127,
	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	127,127,127,127,
	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	127,127,127,127,
	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	127,127,127,127,
	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	127,127,127,127,
	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	127,127,127,127,
	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	127,127,127,127,
	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	127,127,127,127,
	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	127,127,127,127,
	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	127,127,127,127,
	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	127,127,127,127,
};

byte LPMAP[64]{
	  0,  1,  2,  3,  4,  5,  6,  7,
	 16, 17, 18, 19, 20, 21, 22, 23,
	 32, 33, 34, 35, 36, 37, 38, 39,
	 48, 49, 50, 51, 52, 53, 54, 55,
	 64, 65, 66, 67, 68, 69, 70, 71,
	 80, 81, 82, 83, 84, 85, 86, 87,
	 96, 97, 98, 99,100,101,102,103,
	112,113,114,115,116,117,118,119,
};

void setup()
{
	Wire.begin(8);                // join i2c bus with address #8
	launchPad.begin();
	pinMode(LEDPIN, OUTPUT);
	launchPad.turnThruOff();
	Wire.onRequest(requestEvent); // register event
	pinMode(interruptPin, OUTPUT);
	launchPad.setHandleNoteOn(handleLPNoteOn);
	launchPad.setHandleNoteOff(handleLPNoteOff);
	launchPad.setHandleControlChange(handleLPCC);
	//digitalWrite(LEDPIN, HIGH);
}
unsigned long clockTimer = 0;
int stepDuration = 500;
int lastStep = 200;
int currentStep = -1;
int seqLength = 16;
bool isSending = false;

void handleClock() {
	if (millis() > clockTimer + stepDuration) {
		lastStep = currentStep;
		handleCursor();
		handleStep();
		currentStep++;
		currentStep = currentStep % seqLength;
	}
}


unsigned long int timeOutStamp = 0;

void sendWire2microBit(byte note) {
	noteToSend = note;
	digitalWrite(interruptPin, HIGH);
	isSending = true;
	timeOutStamp = millis();
}



void loop()
{
	debug();
}

#define timeOut  10
void checkTimeOut() {
	if (millis() - timeOutStamp > timeOut) {
		digitalWrite(interruptPin, LOW);
		isSending = false;
	}
}

void requestEvent() {
	Wire.write(noteToSend);
	digitalWrite(interruptPin, LOW);
	isSending = false;
}