
#include <MIDI.h>
#include <Wire.h>
//#include <I2C_Anything.h>

#define LEDPIN 14
#define interruptPin 16
#define interruptPin2 10
byte colour = 45;
int noteToSend = 123;
bool runClock = false;
bool trackOrNote = false;
byte trackToSend = 0;


//MIDI.CREATE_DEFAULT_INSTANCE();
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, launchPad);

byte scrollOffset = 0;

byte seqMatrix[320]{
	1,0,0,0,0,0,0,0,	1,0,0,0,0,0,1,0,	127,127,127,127,
	0,1,0,0,0,0,0,0,	0,0,0,0,0,1,0,0,	127,127,127,127,
	0,0,1,0,0,0,0,0,	0,1,0,0,1,0,0,0,	127,127,127,127,
	0,0,0,1,0,0,0,0,	0,0,0,1,0,0,0,0,	127,127,127,127,
	0,0,0,0,1,0,0,0,	0,0,1,0,0,0,0,0,	127,127,127,127,
	0,0,0,0,0,1,0,0,	0,0,0,0,0,0,0,0,	127,127,127,127,
	0,0,0,0,0,0,1,0,	0,0,0,0,0,0,0,0,	127,127,127,127,
	0,0,0,0,0,0,0,1,	0,0,0,0,0,0,0,0,	127,127,127,127,
	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	127,127,127,127,
	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	127,127,127,127,
	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	127,127,127,127,
	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	127,127,127,127,
	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	127,127,127,127,
	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	127,127,127,127,
	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	127,127,127,127,
	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	127,127,127,127,
};
byte prevPage = 100;
byte currentPage = 0;
byte firstStepOfPage = 0;

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

byte LPtoMatrix[128]{
	  0,  1,  2,  3,  4,  5,  6,  7,	-1,-1,-1,-1,-1,-1,-1,-1,
	 20, 21, 22, 23, 24, 25, 26, 27,	-1,-1,-1,-1,-1,-1,-1,-1,
	 40, 41, 42, 43, 44, 45, 46, 47,	-1,-1,-1,-1,-1,-1,-1,-1,
	 60, 61, 62, 63, 64, 65, 66, 67,	-1,-1,-1,-1,-1,-1,-1,-1,

	 80, 81, 82, 83, 84, 85, 86, 87,	-1,-1,-1,-1,-1,-1,-1,-1,
	100,101,102,103,104,105,106,107,	-1,-1,-1,-1,-1,-1,-1,-1,
	120,121,122,123,124,125,126,127,	-1,-1,-1,-1,-1,-1,-1,-1,
	140,141,142,143,144,145,146,147,	-1,-1,-1,-1,-1,-1,-1,-1
};

byte topButts[8] = {104,105,106,107,108,109,110,111};

unsigned long clockTimer = 0;
int stepDuration = 800;
int lastStep = 200;
int currentStep = -1;
int seqLength = 16;
bool isSending = false;

void setup()
{
	Wire.begin(8);                // join i2c bus with address #8
	launchPad.begin();
	pinMode(LEDPIN, OUTPUT);
	launchPad.turnThruOff();
	Wire.onRequest(requestEvent); // register event
	pinMode(interruptPin, OUTPUT);
	digitalWrite(interruptPin, HIGH); //the microbit registers falling edge
	pinMode(interruptPin2, OUTPUT);
	digitalWrite(interruptPin2, HIGH); //the microbit registers falling edge
	launchPad.setHandleNoteOn(handleLPNoteOn);
	launchPad.setHandleNoteOff(handleLPNoteOff);
	launchPad.setHandleControlChange(handleLPCC);
	//digitalWrite(LEDPIN, HIGH);
	if (!runClock) {
		currentStep = 0;
		updatePage();
	}
	
}


void handleClock() {
	if(runClock){
	if (millis() > clockTimer + stepDuration) {
		clockTimer = millis();
		lastStep = currentStep;
		currentStep++;
		currentStep = currentStep % seqLength;
		updatePage();
		//handleCursor();
		handleStep();
		
		//Serial.println(currentStep);
	}
	}
}


unsigned long int timeOutStamp = 0;





void loop()
{
	handleClock();
	launchPad.read();
	checkTimeOut(); //reset interruptPin and isSending if the microbit missed the message

}



