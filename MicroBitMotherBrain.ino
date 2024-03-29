//requires nick gammons i2c anything and the board Arcore "Arduino Leonardo (MIDI)"
// REQUIRES VERSION 4.3.1 of Forty Seven Effects MIDI library.

//TODO:
// Make new method for sending tracks to microbit:
// Track number, data, data, data, data where each "data" is a note
// Track number has highest bit set, so  subtract 32768 to get actual number 
// e.g. 0b1000000000000010 is instrument 2
// how to eeprom storage ?? 
// 8 instruments * 32 steps * 2 sets = 512 individual steps, with a note cap of 6 per step thats 3072 bytes 
// atmega32u4 has 1024 bytes... that would allow us 2 notes per track. :(



//#define DEBUG
#define OLDSCHOOLUSB




#include <EEPROM.h>
#include <MIDI.h>
#include <Wire.h>
#include <I2C_Anything.h>

#ifdef OLDSCHOOLUSB
//#include <MIDIUSB_Defs.h>
//#include <MIDIUSB.h>
//MIDIUSB IS BUILT IN TO ARCORE Arduino Leonardo (MIDI) board
#else 
#include <midi_UsbTransport.h>
static const unsigned sUsbTransportBufferSize = 16;
typedef midi::UsbTransport<sUsbTransportBufferSize> UsbTransport;
UsbTransport sUsbTransport;
#endif


byte oldselectedTrack = 0;
#define LEDPIN 14
#define interruptPin 16
#define interruptPin2 10
#define ZamLed 15
#define followLed 14
#define followSwitch A2
#define polyRhythmLed A0

#define seqLedColour 3
#define followCol 32

unsigned long masterStep = 0;

bool pageWasCleared = false; // flat to keep track of whether to rewrite timesig leds

bool polyRhythm[9] = { false, false, false, false, false, false, false, false, false };
bool globalPolyRhythmEnable = polyRhythm[0];

int desiredPolyEndStep[9] = { 32,32,32,32,32,32,32,32,32 };
int desiredPolyStartStep[9] = { 0,0,0,0,0,0,0,0,0 };
int polyEndStep[9] = { 32,32,32,32,32,32,32,32,32 };
int polyStartStep[9] = { 0,0,0,0,0,0,0,0,0 };
int polyCurrentStep[9] = { 0,0,0,0,0,0,0,0 };

int polySeqLength[9] = { 32,32,32,32,32,32,32,32,32 };

int desiredEndStep = 32;
int desiredStartStep = 0;
int startStep = 0;
int endStep = 32;

int seqLength = endStep - startStep; // temporary debug seqlength, needs to be settable by user
byte polyCurrentPageThatIsPlaying[9] = { 0,0,0,0,0,0,0,0,0 };
byte polyPreviousPageThatWasPlaying[9] = { 0,0,0,0,0,0,0,0,0 };




unsigned long timeOutDeadline = 0;
bool intClock = false;
unsigned long lastMidiClockReceivedTime = 0;
byte midiClockCounter = 5;
bool hadANoteOn = false;
int USBReceiveTimeOutThresh = 4;
unsigned long prevNoteOnTime = 0;
bool waitingForTimeOut = false;

bool firstRun = true;
uint16_t isMutedInt = 0b0000000000000000;
int midiClockDiv = 6;
bool isMuted[8] = { false, false,false, false,false, false,false, false };
unsigned long tapTempoTimer = 0;
bool forceStep = false;
bool SHIFT = false;
bool internalClockSelect = false;
bool midiClockRunning = false;
#define TICK 15
#define RESTART 3

const byte NOTEON = 0x09;
const byte NOTEOFF = 0x08;
const byte MCLOCKTICK = 0x03;
byte timeSig = 0; // 0 = 4/4, 1 = 7/8, 2 = 3/3, 3 = 5/8
byte oldTimeSig = -1;

const byte pageDisplayCol = 16;

bool sentAMidiBuffer = false;

byte colour = 45;
int noteToSend = 123;
bool runClock = false;
bool trackOrNote = false;
byte trackToSend = 0;

unsigned long pageNumberClearTimer = 0;

byte dataPacket128[16] = { 221,222,223,224,225,226,227,228,229,230,231,232,233,234,235,236 };
//unsigned int tracksBuffer16x8[10] = { 65001,65002,65003,65004,65005,65006,65007,65008,65009 }; //tracks 0 - 8 then currentstep then mutes
unsigned int tracksBuffer16x8[10] = { 0,0,0,0,0,0,0,0,0,0 }; //tracks 0 - 8 then currentstep then mutes
//unsigned int tempMidiTracksBuffer16x8[9] = 
//
unsigned int midiTracksBuffer16x8[10] = { 0,0,0,0,0,0,0,0,0,0 }; //last one used for currentStep, so receivers need to be able to determine that we are not setting step number!

struct MySettings : public midi::DefaultSettings                                 //code to change if running status is disabled
{
	static const bool UseRunningStatus = false;                                     // Messes with a lot of stuff!
};

//MIDI.CREATE_DEFAULT_INSTANCE();

MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial1, launchPad, MySettings);
#ifdef OLDSCHOOLUSB

#else
MIDI_CREATE_INSTANCE(UsbTransport, sUsbTransport, UMIDI);
#endif  




byte scrollOffset = 0;
bool altMidiTrack = false;

unsigned int seqMatrix[512] = {
	0,0,0,1,0,0,0,0,	0,1,1,1,1,0,0,0,	0,1,1,1,1,1,0,0,	0,1,0,0,0,1,0,0,
	0,0,0,1,0,0,0,0,	0,0,0,0,0,1,0,0,	0,0,0,0,0,1,0,0,	0,1,0,0,0,1,0,0,
	0,0,0,1,0,0,0,0,	0,0,0,0,0,1,0,0,	0,0,0,0,0,1,0,0,	0,1,0,0,0,1,0,0,
	0,0,0,1,0,0,0,0,	0,0,0,0,0,1,0,0,	0,0,1,1,1,1,0,0,	0,1,1,1,1,1,0,0,
	0,0,0,1,0,0,0,0,	0,0,0,0,1,0,0,0,	0,0,0,0,0,1,0,0,	0,0,0,0,0,1,0,0,
	0,0,0,1,0,0,0,0,	0,0,0,1,0,0,0,0,	0,0,0,0,0,1,0,0,	0,0,0,0,0,1,0,0,
	0,0,0,1,0,0,0,0,	0,0,1,0,0,0,0,0,	0,0,0,0,0,1,0,0,	0,0,0,0,0,1,0,0,
	0,0,0,1,0,0,0,0,	0,1,1,1,1,1,1,0,	0,1,1,1,1,1,0,0,	0,0,0,0,0,1,0,0, 

	0,1,1,1,1,1,0,0,	0,1,0,0,0,0,0,0,	0,1,1,1,1,1,0,0,	0,1,1,1,1,1,0,0,
	0,1,0,0,0,0,0,0,	0,1,0,0,0,0,0,0,	0,0,0,0,0,1,0,0,	0,1,0,0,0,1,0,0,
	0,1,0,0,0,0,0,0,	0,1,0,0,0,0,0,0,	0,0,0,0,0,1,0,0,	0,1,0,0,0,1,0,0,
	0,1,1,1,1,1,0,0,	0,1,1,1,1,1,0,0,	0,0,0,0,0,1,0,0,	0,1,1,1,1,1,0,0,
	0,0,0,0,0,1,0,0,	0,1,0,0,0,1,0,0,	0,0,0,0,0,1,0,0,	0,1,0,0,0,1,0,0,
	0,0,0,0,0,1,0,0,	0,1,0,0,0,1,0,0,	0,0,0,0,0,1,0,0,	0,1,0,0,0,1,0,0,
	0,0,0,0,0,1,0,0,	0,1,0,0,0,1,0,0,	0,0,0,0,0,1,0,0,	0,1,0,0,0,1,0,0,
	0,1,1,1,1,1,0,0,	0,1,1,1,1,1,0,0,	0,0,0,0,0,1,0,0,	0,1,1,1,1,1,0,0 };

bool seqMatrixShift = false;

/*
unsigned int seqMatrix[256] = {
0,0,0,1,0,0,0,0,	0,1,1,1,1,0,0,0,	0,1,1,1,1,1,0,0,	0,1,0,0,0,1,0,0,
0,0,0,1,0,0,0,0,	0,0,0,0,0,1,0,0,	0,0,0,0,0,1,0,0,	0,1,0,0,0,1,0,0,
0,0,0,1,0,0,0,0,	0,0,0,0,0,1,0,0,	0,0,0,0,0,1,0,0,	0,1,0,0,0,1,0,0,
0,0,0,1,0,0,0,0,	0,0,0,0,0,1,0,0,	0,0,1,1,1,1,0,0,	0,1,1,1,1,1,0,0,
0,0,0,1,0,0,0,0,	0,0,0,0,1,0,0,0,	0,0,0,0,0,1,0,0,	0,0,0,0,0,1,0,0,
0,0,0,1,0,0,0,0,	0,0,0,1,0,0,0,0,	0,0,0,0,0,1,0,0,	0,0,0,0,0,1,0,0,
0,0,0,1,0,0,0,0,	0,0,1,0,0,0,0,0,	0,0,0,0,0,1,0,0,	0,0,0,0,0,1,0,0,
0,0,0,1,0,0,0,0,	0,1,1,1,1,1,1,0,	0,1,1,1,1,1,0,0,	0,0,0,0,0,1,0,0 };
*/
/*
unsigned int dummySeqMatrix[256] = {
	0,0,0,1,0,0,0,0,	0,1,1,1,1,0,0,0,	0,1,1,1,1,1,0,0,	0,1,0,0,0,1,0,0,
	0,0,0,1,0,0,0,0,	0,0,0,0,0,1,0,0,	0,0,0,0,0,1,0,0,	0,1,0,0,0,1,0,0,
	0,0,0,1,0,0,0,0,	0,0,0,0,0,1,0,0,	0,0,0,0,0,1,0,0,	0,1,0,0,0,1,0,0,
	0,0,0,1,0,0,0,0,	0,0,0,0,0,1,0,0,	0,0,1,1,1,1,0,0,	0,1,1,1,1,1,0,0,
	0,0,0,1,0,0,0,0,	0,0,0,0,1,0,0,0,	0,0,0,0,0,1,0,0,	0,0,0,0,0,1,0,0,
	0,0,0,1,0,0,0,0,	0,0,0,1,0,0,0,0,	0,0,0,0,0,1,0,0,	0,0,0,0,0,1,0,0,
	0,0,0,1,0,0,0,0,	0,0,1,0,0,0,0,0,	0,0,0,0,0,1,0,0,	0,0,0,0,0,1,0,0,
	0,0,0,1,0,0,0,0,	0,1,1,1,1,1,1,0,	0,1,1,1,1,1,0,0,	0,0,0,0,0,1,0,0
};
*/



byte numberOfPages = 4;

/*
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
*/
#define numberOfTracks 8
#define matrixTrackOffset 32



byte prevPage = 100;
byte currentPage = 0;
byte pageSelect = 0;
byte viewPage = 0;
byte firstStepOfPage = 0;
bool follow = false;


int knobA = 0;
int knobB = 0;
bool buttA = false;
bool buttB = false;
bool buttC = false;
bool buttX = false;

byte tracksBuffer[8] = { 201,202,203,204,205,206,207,208 };
byte trackColours[8] = { 79,95,127,110,126,109,125,124 };
bool isPoly[9] = { true,true,true,true,true,true ,true ,false, false }; // this equates to midi channel 1,2,3,4,5,6,7 as poly and 8+9 as mono channels sharing same int in the buffer //the last false is there so USBmidi can look it up as alt channel

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
int stepDuration = 125;
int tripletStepDuration = (stepDuration * 4) / 3;
int lastStep = 200;
int currentStep = -1;
bool isSending = false;



byte selectedTrack = 0; // 0 = overview, 1 = track one 2 = track2 and so forth

const byte buttApin = 8;
const byte buttBpin = 9;
const byte buttCpin = 6;
const byte buttXpin = 7;

const byte ledApin = 4;
const byte ledBpin = 5;
bool forceUpdate = true;

byte currentPageThatIsPlaying = 0;
byte previousPageThatWasPlaying = 0;

unsigned long i2cTimer = 0; //for debug

bool topLedWasSet[8] = { false,false,false,false, false,false,false,false, }; //array to keep track of whether anything has writen to a specific led this time around
bool oldTopLedWasSet[8] = { false,false,false,false, false,false,false,false, }; //array to keep track of whether anything has writen to a specific led last time around
bool greenPageLedWasSet[4]{ false, false, false, false }; //array to track what green led to turn off

void setup()
{
	pinMode(buttApin, INPUT_PULLUP);
	pinMode(buttBpin, INPUT_PULLUP);
	pinMode(buttCpin, INPUT_PULLUP);
	pinMode(buttXpin, INPUT_PULLUP);
	pinMode(ledApin, OUTPUT);
	pinMode(ledBpin, OUTPUT);
	pinMode(LEDPIN, OUTPUT);
	//pinMode(A0, INPUT);
	pinMode(A1, INPUT);
	pinMode(interruptPin, OUTPUT);
	pinMode(interruptPin2, OUTPUT);
	pinMode(ZamLed, OUTPUT);
	pinMode(polyRhythmLed, OUTPUT);
	pinMode(followSwitch, INPUT_PULLUP);
	pinMode(followLed, OUTPUT);

	Wire.begin(8);                // join i2c bus with address #8
	launchPad.begin();
	launchPad.turnThruOff();
	digitalWrite(interruptPin, HIGH); //the microbit registers falling edge
	digitalWrite(interruptPin2, HIGH); //the microbit registers falling edge
	launchPad.setHandleNoteOn(handleLPNoteOn);
	launchPad.setHandleNoteOff(handleLPNoteOff);
	launchPad.setHandleControlChange(handleLPCC);
#ifdef OLDSCHOOLUSB
#else
	UMIDI.setHandleNoteOn(preHandleUSBNoteOn);
	UMIDI.setHandleNoteOff(HandleUsbNoteOff);
	UMIDI.setHandleStart(resetSeq);
	UMIDI.setHandleClock(handleUsbMidiClockTicks);
	UMIDI.setHandleContinue(USBContinue);
	UMIDI.setHandleTimeCodeQuarterFrame(timeCodeQuarterFrame);
	UMIDI.setHandleStop(handleUSBstop);
	UMIDI.setHandleSystemReset(resetSeq);
	UMIDI.turnThruOff();
#endif // OLDSCHOOLUSB






#ifdef DEBUG
	Wire.onRequest(debugRequestEvent); // register event
#else
	Wire.onRequest(requestEvent); // register event
#endif // DEBUG

	if (!runClock) {
		currentStep = 0;
	}
	//changeselectedTrack(selectedTrack);

	//EEPROM.write(1023, 123);
	//delay(4000);
	if (EEPROM.read(1023) == 123) { //look for magic number that means we have stored something in EEPROM
		
		digitalWrite(polyRhythmLed, HIGH);
		recallSeq();
	}

	/*
	if (EEPROM.read(1022) == 122) { //look for magic number that means we have stored something in EEPROM
		Serial.println("found 122 at 1022");
		digitalWrite(polyRhythmLed, HIGH);
		//recallSeq(1);
	}
	*/


	delay(1000);
	clearPage();
	//updatePage(0);
	digitalWrite(polyRhythmLed, LOW);
	forceUpdate = true;
	updatePage(0);
	firstRun = false;

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

}




void loop() {
#ifdef DEBUG
	debugLoop();
#else
	//handlePageNumDisplayTimeouts();

	handleClock();
	launchPad.read();

#ifdef OLDSCHOOLUSB
	if (firstRun) {
		usbmidiprocessing(); //OLD USB MIDI PROCESSING
		firstRun = false;
	}
	usbmidiprocessing(); //OLD USB MIDI PROCESSING

#else 
	UMIDI.read();
	if (waitingForTimeOut) { //if we are waiting to see if there are any more messages for this step
		if (millis() > timeOutDeadline) { //if we timed out
			sendUsbMidiPackage();
			waitingForTimeOut = false;
		}
	}
#endif // oldScoolMidi
	checkTimeOut(); //reset interruptPin and isSending if the microbit missed the message
	handleKnobsAndButtons();
	handleRunClockActivation();
	if (firstRun) {
	}
#endif // DEBUG
	//Serial.println(oldselectedTrack);
}

bool numIsDisplayed = false;

void handlePageNumDisplayTimeouts() {
	if (numIsDisplayed) {
		if (millis() > pageNumberClearTimer) {
			numIsDisplayed = false;
			forceUpdate = true;
			updatePage(currentPage);
		}
	}
}