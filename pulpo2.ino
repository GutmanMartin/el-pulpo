/*
  el pulpo recive cualquier note on y note off, y lo pasa como clock
  luego envía los mensajes por canal 1
  tenés que enviarle los mensajes por un canal que no sea el 1, porque sino loopmidi hace feedback

*/


// Change values with //* 


// LIBRARIES

#include <MIDI.h> // by Francois Best
MIDI_CREATE_DEFAULT_INSTANCE(); 


// BUTTONS
const int NButtons = 8; //*  total number of push buttons
const int buttonPin[NButtons] = {2, 3, 4, 5, 6, 7, 8 ,9 };
int buttonPState[NButtons] = {};        // stores the button previous value

// SWITCHES
const int velocitySwitchPin = 10;
const int otherSwitchPin = 12;

bool isVelocitySwitch = false;
bool isOtherSwitch = false;


// POTENTIOMETERS
const int MAX_POT_STATE = 900;
const int NPots = 6; //* total numbers of pots (slide & rotary)
const int potPin[NPots] = {A5, A4, A3, A2, A1, A0}; //* Analog pins of each pot connected straight to the Arduino i.e 4 pots, {A3, A2, A1, A0};
                                          // have nothing in the array if 0 pots {}

int potPState[NPots] = {0}; // Previous state of the pot; delete 0 if 0 pots
int potVelocityState[NPots] = {127,127,127,127,127,127}; // Previous state of the pot; delete 0 if 0 pots


// MIDI
byte midiCh = 1; //** MIDI channel to be used; You can add more if you need to reorganize or have a billion buttons/pots
byte note = 36; //** First note to be used for digital buttons, then go up chromatically in scale according to the sequence in your "buttonPin" array

byte cc = 2; //** First MIDI CC to be used for pots on Analog Pins in order of the "potPin" array; then goes up by 1

// clock
const int clockPin = 13;
const int delayForClock = 100;
unsigned long timeForTurningClockOff = 0;
const int clockInputPin = 11;


// SETUP
void setup() { 
  Serial.begin(115200); //**  Baud Rate 31250 for MIDI class compliant jack | 115200 for Hairless MIDI  
  // Buttons
  // Initialize buttons with pull up resistors
  for (int i = 0; i < NButtons; i++) {
    pinMode(buttonPin[i], INPUT_PULLUP);
  }
  pinMode(velocitySwitchPin, INPUT_PULLUP);
  pinMode(otherSwitchPin, INPUT_PULLUP);

  pinMode(clockInputPin, INPUT);
  pinMode(clockPin, OUTPUT);
  digitalWrite(clockPin, LOW);
}


// LOOP
void loop() {
    
  switches();
  recieveClock();
  if (digitalRead(!isOtherSwitch)){
    buttons();
    potentiometers();
    
    digitalWrite(clockPin, LOW);
  }
}

void switches() {
  isVelocitySwitch = digitalRead(velocitySwitchPin);
  isOtherSwitch = digitalRead(isOtherSwitch);
}



void recieveClock() {
  if (digitalRead(clockInputPin)) {
    timeForTurningClockOff = millis() + delayForClock;
  }

  if (millis() < timeForTurningClockOff) {
    digitalWrite(clockPin, HIGH);
  } else if (MIDI.read()) {
    // Check if it's a Note On or Note Off message
    if (MIDI.getType() == midi::NoteOn) {
      // Note On event
      digitalWrite(clockPin, HIGH);
    }
  }
}

// BUTTONS
void buttons() {
  for (int i = 0; i < NButtons; i++) {
    bool buttonState = digitalRead(buttonPin[i]);
    if (buttonPState[i] != buttonState) {
      if (buttonState == HIGH) {
        MIDI.sendNoteOn(note + (NButtons - i -1), 127, midiCh); // note, velocity, channel  
      } else {
        MIDI.sendNoteOn(note + (NButtons - i -1), 0, midiCh); // note, velocity, channel
      }
      buttonPState[i] = buttonState;
    }
  }
}


// POTENTIOMETERS
void potentiometers() {
  for (int i = 0; i < NPots; i++) { // Loops through all the potentiometers
    int potState = constrain(map(analogRead(potPin[i]), 0, MAX_POT_STATE, 0, 127), 0, 127);
    
    if (potPState[i] != potState) {
      if (potState > 0 && !isVelocitySwitch) MIDI.sendControlChange(note + NButtons + (NPots - i -1), potState, midiCh);  //MIDI.sendPolyPressure(note + NButtons + (NPots - i -1), potState, midiCh);
      if (potPState[i] < 1 && potState > 1){
        if(isVelocitySwitch) {
          MIDI.sendNoteOn(note + NButtons + (NPots - i -1), potState, midiCh); 
          potVelocityState[i] = potState;
        } else {
          MIDI.sendNoteOn(note + NButtons + (NPots - i -1), potVelocityState[i], midiCh);
        }
      } else if (potPState[i] > 1 && potState < 1){

        MIDI.sendNoteOn(note + NButtons + (NPots - i -1), 0, midiCh);
      }
       potPState[i] = potState;
    }
  }
}