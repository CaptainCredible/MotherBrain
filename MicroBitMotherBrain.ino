//#define DEBUG


#include <MIDI.h>
#include <Wire.h>
#include <I2C_Anything.h>

#define LEDPIN 14
#define interruptPin 16
#define interruptPin2 10

byte colour = 45;
int noteToSend = 123;
bool runClock = false;
bool trackOrNote = false;
byte trackToSend = 0;

byte dataPacket128[16] = { 221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236 };
unsigned int tracksBuffer16x8[8] = { 65001,65002,65003,65004,65005,65006,65007,65008 };

struct MySettings : public midi::DefaultSettings                                 //code to change if running status is disabled
{
	static const bool UseRunningStatus = false;                                     // Messes with a lot of stuff!
};

//MIDI.CREATE_DEFAULT_INSTANCE();
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial1, launchPad, MySettings);

byte scrollOffset = 0;


int seqMatrix[256] = {
	1,0,0,5,0,0,0,0,	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,
	0,2,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,
	0,0,2,0,0,0,0,0,	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,
	0,0,0,1,0,0,0,0,	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,	0,0,0,0,0,0,0,0,
};

int seqLength = 16;

byte oldSeqMatrix[320] = {
	1,2,3,4,5,0,0,0,	1,0,0,0,0,0,1,0,	127,127,127,127,
	0,0,0,0,0,0,0,0,	0,0,0,0,0,1,0,0,	127,127,127,127,
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
#define numberOfTracks 8
#define matrixTrackOffset 32



byte prevPage = 100;
byte currentPage = 0;
byte firstStepOfPage = 0;

int knobA = 0;
int knobB = 0;
bool buttA = false;
bool buttB = false;
bool buttC = false;
bool buttX = false;

byte tracksBuffer[8] = { 201,202,203,204,205,206,207,208 };
byte trackColours[8] = { 79,95,127,110,126,109,125,124 };
bool isPoly[8] = { true,true,true,true,true,true ,false ,false };

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
	0,  1,  2,  3,  4,  5,  6,  7,		-1,-1,-1,-1,-1,-1,-1,-1,
	32, 33, 34, 35, 36, 37, 38, 39,		-1,-1,-1,-1,-1,-1,-1,-1,
	64, 65, 66, 67, 68, 69, 70, 71,		-1,-1,-1,-1,-1,-1,-1,-1,
	96, 97, 98, 99,100,101,102,103,		-1,-1,-1,-1,-1,-1,-1,-1,

	128,129,130,131,132,133,134,136,	-1,-1,-1,-1,-1,-1,-1,-1,
	160,161,162,163,164,165,166,167,	-1,-1,-1,-1,-1,-1,-1,-1,
	192,193,194,195,196,197,198,199,	-1,-1,-1,-1,-1,-1,-1,-1,
	224,225,226,227,228,229,230,231,	-1,-1,-1,-1,-1,-1,-1,-1
};

byte oldLPtoMatrix[128]{
	  0,  1,  2,  3,  4,  5,  6,  7,	-1,-1,-1,-1,-1,-1,-1,-1,
	 20, 21, 22, 23, 24, 25, 26, 27,	-1,-1,-1,-1,-1,-1,-1,-1,
	 40, 41, 42, 43, 44, 45, 46, 47,	-1,-1,-1,-1,-1,-1,-1,-1,
	 60, 61, 62, 63, 64, 65, 66, 67,	-1,-1,-1,-1,-1,-1,-1,-1,

	 80, 81, 82, 83, 84, 85, 86, 87,	-1,-1,-1,-1,-1,-1,-1,-1,
	100,101,102,103,104,105,106,107,	-1,-1,-1,-1,-1,-1,-1,-1,
	120,121,122,123,124,125,126,127,	-1,-1,-1,-1,-1,-1,-1,-1,
	140,141,142,143,144,145,146,147,	-1,-1,-1,-1,-1,-1,-1,-1
};

byte topButts[8] = { 104,105,106,107,108,109,110,111 };
byte vertButts[8] = { 8,24,40,56,72,88,104,120 };


unsigned long clockTimer = 0;
int stepDuration = 800;
int lastStep = 200;
int currentStep = -1;
bool isSending = false;

byte pageMode = 0; // 0 = overview, 1 = track one 2 = track2 and so forth

const byte buttApin = 8;
const byte buttBpin = 9;
const byte buttCpin = 6;
const byte buttXpin = 7;

const byte ledApin = 4;
const byte ledBpin = 5;
bool forceUpdate = true;

unsigned long i2cTimer = 0; //for debug


void setup()
{

	
	pinMode(buttApin, INPUT_PULLUP);
	pinMode(buttBpin, INPUT_PULLUP);
	pinMode(buttCpin, INPUT_PULLUP);
	pinMode(buttXpin, INPUT_PULLUP);
	pinMode(ledApin, OUTPUT);
	pinMode(ledBpin, OUTPUT);
	pinMode(LEDPIN, OUTPUT);
	pinMode(A0, INPUT);
	pinMode(A1, INPUT);
	pinMode(interruptPin, OUTPUT);
	pinMode(interruptPin2, OUTPUT);
	Wire.begin(8);                // join i2c bus with address #8
	launchPad.begin();
	launchPad.turnThruOff();
	digitalWrite(interruptPin, HIGH); //the microbit registers falling edge
	digitalWrite(interruptPin2, HIGH); //the microbit registers falling edge
	launchPad.setHandleNoteOn(handleLPNoteOn);
	launchPad.setHandleNoteOff(handleLPNoteOff);
	launchPad.setHandleControlChange(handleLPCC);
#ifdef DEBUG
	Wire.onRequest(debugRequestEvent); // register event
#else
	Wire.onRequest(requestEvent); // register event
#endif // DEBUG

	

	clearPage();

	if (!runClock) {
		currentStep = 0;
	}
	//changePageMode(pageMode);
	for (int i = 0; i < 8; i++) {
		launchPad.sendNoteOn(vertButts[i], trackColours[i], 1);
		delay(100);
		//Serial.println(i);
	}
	//launchPad.sendNoteOff(127, 127, 10);
	updatePage(0);
}





unsigned long int timeOutStamp = 0;
//byte counter = 0;
// long (32 bit) - signed number from -2,147,483,648 to 2,147,483,647
//byte first = 201;
//byte second = 202;
//byte third = 203;
//byte fourth = 204;
//byte fifth = 205;
//byte sixth = 206;
//byte seventh = 207;
//byte eighth = 208;

unsigned long dataPacket = 1;

//uint64_t dataPacket64 = 0;

void debugLoop() {
	delay(5000);

	launchPad.sendNoteOn(1, 100, 1);
	sendTracksBuffer();
	//send32BitInt();
	//send64BitInt();
	Serial.println("Alive");
	delay(100);
	launchPad.sendNoteOn(1, 0, 1);
	checkTimeOut(); //reset interruptPin and isSending if the microbit missed the message

}


void loop() {
#ifdef DEBUG
	debugLoop();
#else
	handleClock();
	launchPad.read();
	checkTimeOut(); //reset interruptPin and isSending if the microbit missed the message
	handleKnobsAndButtons();
#endif // DEBUG
}


#define minStepDuration 20

void handleKnobsAndButtons() {
	knobA = analogRead(A1);
	knobB = analogRead(A0);
	buttA = digitalRead(buttApin);
	buttB = digitalRead(buttBpin);
	buttC = digitalRead(buttCpin);
	buttX = digitalRead(buttXpin);
	stepDuration = ((2048+minStepDuration) - (knobA << 1));
	//stepDuration = ((5048 + minStepDuration) - (knobA << 1));
}

