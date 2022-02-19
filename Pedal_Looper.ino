#include <EEPROM.h>
#include "Pedal_Looper.h" // basically has program constants in it
#include "LED.h"
#include "OutputBuffer.h"
#include "DataPin.h"
#include "Button.h"
#include "PedalData.h"

struct ButtonPress {
  bool ButtonDown;  // to indicate if button was pressed or released
  bool LongPress;   // long button press
  bool ModePress;   // mode button press
  bool BypassPress; // bypass button press
  bool Changed;     // dirty flag
  int ButtonIDX;    // IDX of button pressed
  void copyBPData(ButtonPress bpToCopy);
  void initialize();
};

void ButtonPress::copyBPData(ButtonPress bpToCopy) {
  ButtonPress::ButtonDown  = bpToCopy.ButtonDown;
  ButtonPress::LongPress   = bpToCopy.LongPress;
  ButtonPress::ModePress   = bpToCopy.ModePress;
  ButtonPress::BypassPress = bpToCopy.BypassPress;
  ButtonPress::ButtonIDX   = bpToCopy.ButtonIDX;
  ButtonPress::Changed     = bpToCopy.Changed;
}

void ButtonPress::initialize() {
  ButtonPress::ButtonDown  = false;
  ButtonPress::LongPress   = false;
  ButtonPress::ModePress   = false;
  ButtonPress::BypassPress = false;
  ButtonPress::ButtonIDX   = -1;
  ButtonPress::Changed     = false;
}

bool BlinkToggle = LOW;       // set on or off to make blinking LEDs blink by calling toggleLEDBlinkState()
bool ButtonHoldToggle = LOW;  // used to determine if BUTTON_HOLD_MAX has been reached, set with setButtonHoldMaxFlag()

PedalData CurrentPedalState;  // memory for current pedal state
PedalData SavedPedalState;    // memory for previous loop iteration pedal state


// Miscellaneous global variables **********************************************************************
unsigned int ChangedButtonMatrix = 0; // stores changed button identity(s)

int ChangedButtonIDX = 0;
int SavedChangedButtonIDX = 0;
bool DoNothingFlag = LOW;
bool ButtonPressedFlag = LOW;
// End global definitions ******************************************************************************

// LED objects for all LED outputs.
LED LEDs[(MAX_LOOPS * 2) + 2];  // LED object for each Loop (8), Preset (8) plus 2 for Mode and Bypass indicators.

// DataPin objects for all input pins
Button InputButton[MAX_LOOPS + 2]; // one for each loop plus 2 for Mode and Bypass


OutputBuffer LEDOutputBuffer;  // output buffer for LEDs
OutputBuffer RelayOutputBuffer; // output buffer for relays


void setup() {
  #if DEBUG 
    Serial.begin(9600); 
    Serial.println("Starting setup()");
  #endif

  initializePedalStateMemory(); // gets saved configuration from EEPROM, loads defaults if none saved.

//  // Initializes Mode and Bypass status LEDs. ************
  LEDs[PM_STATUS_IDX].setPin(PM_STATUS_PIN);
  LEDs[BYPASS_STATUS_IDX].setPin(BYPASS_STATUS_PIN);
// *****************************************

  // Initializes all inputs ******************
  InputButton[STOMP1_IDX].setPin(STOMP1_PIN);
  InputButton[STOMP2_IDX].setPin(STOMP2_PIN);
  InputButton[STOMP3_IDX].setPin(STOMP3_PIN);
  InputButton[STOMP4_IDX].setPin(STOMP4_PIN);
  InputButton[STOMP5_IDX].setPin(STOMP5_PIN);
  InputButton[STOMP6_IDX].setPin(STOMP6_PIN);
  InputButton[STOMP7_IDX].setPin(STOMP7_PIN);
  InputButton[STOMP8_IDX].setPin(STOMP8_PIN);

  InputButton[PM_INPUT_IDX].setPin(PM_INPUT_PIN);
  InputButton[BYPASS_INPUT_IDX].setPin(BYPASS_INPUT_PIN);
  // *****************************************

  
  doLEDStartupAnimation();  // cycles through all of the LEDs for show
                            // could even display 8 bit (or even 16) for stored numeric data
                            // such as version number, EEPROM write count, whatever...

  CurrentPedalState.resetButtonStateMatrix(); // sets ButtonStateMatrix to 0's, needed for 
                                              // first loop() iteration
  
  #if DEBUG
//    Serial.print("CurrentPedalState.getButtonStateMatrix() = "); Serial.println(String(CurrentPedalState.getButtonStateMatrix(), BIN));
//    Serial.print("SavedPedalState.getButtonStateMatrix() = "); Serial.println(String(SavedPedalState.getButtonStateMatrix(), BIN));
    Serial.println("Ending setup()");
  #endif
}


void loop() {
  static unsigned long ButtonHoldStartMillis = millis(); // used to detect longpress

  static ButtonPress CurrentBP;
  static ButtonPress SavedBP;

  // querying the buttons to see if any have changed state and how...
  SavedBP.copyBPData(CurrentBP);
  getCurrentButtonPressData(&CurrentBP);
  // at this point you know if a button's state has changed since last time through,
  // which button it is, whether it was pressed or released and whether bypass or mode buttons (special cases)
  // have been freshly pressed...
  // now check for Longpress condition and set CurrentBP.LongPress appropriately... 

  // now you need to check what mode the box is in and 
  // call setLongPressFlag accordingly
  if(CurrentPedalState.getPedalMode() == PM_PRESET ||     // if in preset or
     CurrentPedalState.getPedalMode() == PM_EDIT)         // edit mode
    setLongPressFlag(&CurrentBP, &ButtonHoldStartMillis); // get the longpress flag set
  else                                                    // otherwise
    CurrentBP.LongPress = false;                          // cancel the longpress flag


  #if DEBUG
    if(CurrentBP.LongPress) Serial.println("Ding! Long Mode press detected.");
  #endif

  // ok now we know if we have a long mode press...
  // if so... correct pedal mode must be selected....
  // mode tap must be accounted for here...
  //************************************************   
  if(CurrentBP.LongPress && CurrentPedalState.getPedalMode() != PM_BYPASS) {
    changePedalMode();    // sets correct PedalMode in CurrentPedalState if not in bypass
  }

  switch(CurrentPedalState.getPedalMode()) {
    case PM_PRESET:
      if(!CurrentBP.LongPress) {
        if(CurrentBP.ButtonDown && CurrentBP.Changed && CurrentBP.ModePress) { 
          #if DEBUG
            Serial.println("Mode tap detected in PRESET mode.");
          #endif
          changePedalMode(true);    // accounts for mode tap in preset mode
                                    // passing in true tells it that it's a short
                                    // press and not a long press
                                    // long press takes it to edit instead of stomp
        }
      }
      break;
    case PM_STOMP:
      if(!CurrentBP.LongPress) {
        #if DEBUG
          Serial.println("Mode tap detected in STOMP mode.");
        #endif
        if(CurrentBP.ButtonDown && CurrentBP.Changed && CurrentBP.ModePress) { 
          changePedalMode();    // accounts for mode tap in stomp mode
        }
      }
      break;
  }

  // now figure out if you have a bypass press?
  if(CurrentBP.BypassPress) {
    // i have a bypass press, do something
    #if DEBUG
      Serial.println("Bypass press detected.");
    #endif

    CurrentPedalState.toggleBypassMode();  // needs tweaking, just testing
  }

  // tapping bypass at any time should put the box into bypass mode...
  // while in bypass mode none of the buttons (other than bypass)
  // should work, the signal should go straight from input to output
  // without going into any loops
  // 
  // while in bypass mode a bypass tap will take the box back into stomp mode
  // with previously selected stomps still selected
  // if in stomp when entering bypass 
  // box will go to preset mode if in preset or edit
  // when entering bypass.  if entering bypass from edit mode, 
  // cancel edit and go back to selected preset when coming out of bypass.

  // TODO: *****************************************************************
  // now if you are in preset mode and mode button is being released but 
  // not after a longpress, switch to stomp mode

  // THIS CODE DOESN'T WORK RIGHT **************************
  // if(CurrentPedalState.getPedalMode() == PM_PRESET) { // pedal mode preset
  //   if(!CurrentBP.ButtonDown) {                       // button released
  //     if(CurrentBP.Changed) {                          // make sure button is released this loop
  //       if(getTimeInterval(millis(), ButtonHoldStartMillis) < BUTTON_HOLD_MAX) // make sure we're not exceeding a long press
  //         CurrentPedalState.setPedalMode(PM_STOMP);     // set mode to stomp - needs testing
  //     }
  //   }
  // }
  // *********************************************************

  switch(CurrentPedalState.getPedalMode()) {
    case PM_STOMP:    doStompboxMode(); break;
    case PM_PRESET:   doPresetMode();   break;
    case PM_EDIT:     doEditMode();     break;
    case PM_BYPASS:   doBypassMode();   break;
  }

  updateAllOutputs();

  //delay(100);

}

bool setLongPressFlag(ButtonPress *BP, unsigned long* ButtonHoldStartMillis) {
  static bool ModeLongPressFlag = false;

  // #if DEBUG
  //   // Serial.println("Executing setLongPressFlag()");
  // #endif
  
  if(BP->ModePress) {               // pressed Mode Button..,
    *ButtonHoldStartMillis = millis();     // reset longpress counter
  }

  BP->LongPress = true; // initialize flag to be set....
  
  if(BP->ButtonDown) {                                            // button down?

    BP->LongPress = false;                                       // cancel longpress

    if(BP->ModePress) {
      ModeLongPressFlag = true;               // set ModeLongPressFlag true if it's mode button
      *ButtonHoldStartMillis = millis();     // reset longpress counter 
    }
    else {                                      // if not mode press
      if(!ModeLongPressFlag) {                  // and if ModeLongPressFlag not set (redundant)
        *ButtonHoldStartMillis = millis();      // reset button hold counter. }
      }
    }

    // #if DEBUG
    //   // if(BP->ModePress)
    //   //   Serial.println("Mode button press detected.");
    //   // else
    //   //   Serial.println("Other button press detected.");
    // #endif
    
  }
  else {                                                      
    if(BP->Changed) {                                           // maybe button up then...
      if(BP->ButtonIDX == PM_INPUT_IDX) {       // mode button up, cancel longpress.
        // #if DEBUG
        //   // Serial.println("Mode button release detected.");
        // #endif

        BP->LongPress = false;
        *ButtonHoldStartMillis = millis();      // reset hold counter
        ModeLongPressFlag = false;                  // reinitialize flag
      }
      else {
        // #if DEBUG
        //   // Serial.println("Other button release detected.");
        // #endif

        if(!ModeLongPressFlag) {                                    // check ModeLongPressFlag:
          BP->LongPress = ModeLongPressFlag;                        // cancel LongPress if ModeLongPressFlag not set
          *ButtonHoldStartMillis = millis();                    // also reset counter
        }
      }
    }                                                         // ignore other buttons being released.
    else {                                                    // if no change in button state
      if(!ModeLongPressFlag)                                      // long press flag not set
        *ButtonHoldStartMillis = millis();                    // reset buttonhold counter
      BP->LongPress = ModeLongPressFlag;                          
      // #if DEBUG
      //   // Serial.println("No button state change detected.");
      // #endif                                                          // do nothing
    }                                                        // 
  }

  // #if DEBUG
  //   // Serial.print("ModeLongPressFlag = "); Serial.println(ModeLongPressFlag);
  //   // Serial.print("BP->LongPress = "); Serial.println(BP->LongPress);
  // #endif

  if(BP->LongPress && ModeLongPressFlag) { // if not cancelled yet, check the buttonholdstart time against current time 
    BP->LongPress = (getTimeInterval(millis(), *ButtonHoldStartMillis) > BUTTON_HOLD_MAX); // set longpress flag;
  }
  
  if(BP->BypassPress) { // finally, if we have a bypass press, cancel longpress.
    BP->LongPress = false;
    ModeLongPressFlag = false;
  }

  if(BP->LongPress) {    // if after ALL of these disqualifiers you are still longpress, reset flags and counters
    *ButtonHoldStartMillis = millis();
    ModeLongPressFlag = false;
  }

  // #if DEBUG
  //   // if(BP->LongPress)
  //   //   Serial.println("BP->LongPress is true.");
  //   // Serial.println("Exiting setLongPressFlag()");
  // #endif



  //return BP.LongPress; // changed to pointer
}




void getCurrentButtonPressData(ButtonPress *BP) {
  BP->initialize();

  // querying the buttons to see if any have changed state and how...
  saveOldButtonStates();         // save button states from previous loop iteration in SavedButtonStateMatrix
  getNewButtonStates();          // get current button states and stores in ButtonStateMatrix

  ChangedButtonMatrix = updateChangedButtonMatrix();

  if(ChangedButtonMatrix) {  // returns true if a button has changed state (any bits set to 1)
    ChangedButtonIDX = getChangedButtonIDX(); // determine which button changed 
                                              // (could be more than one but just take first one it comes across)

    BP->Changed = true;

    BP->ButtonIDX = ChangedButtonIDX;          // save changed button idx with BP  
    BP->LongPress = false; // initialize flag

    BP->ButtonDown = setButtonDownFlag(ChangedButtonIDX); // determine if it is now down or up
    
    if(BP->ButtonDown) {
      switch(ChangedButtonIDX) {
        case PM_INPUT_IDX:      BP->ModePress   = true;   break;
        case BYPASS_INPUT_IDX:  BP->BypassPress = true;   break;
        default:                BP->ModePress   = false;
                                BP->BypassPress = false;  break;
      }
    }
  }
  //return BP;  // turned it into a pointer...
}


int updateChangedButtonMatrix() {
  return (CurrentPedalState.getButtonStateMatrix() ^ SavedPedalState.getButtonStateMatrix()); // determines which inputs have changed state
                                                                                              // by setting corresponding bit to 1 in return value
                                                                                              // and all others to 0 using XOR (^) operator
}

bool setButtonDownFlag(int ButtonIDX) {
  static bool newFlag;             
  
  // determine whether it turned on or off and set newFlag
  newFlag = CurrentPedalState.getButtonState(ButtonIDX);
  
  #if DEBUG // tells whether button pressed or release was detected.
    Serial.print("button "); Serial.print(ButtonIDX); 
    if(newFlag) Serial.println(" pressed.");
    else        Serial.println(" released.");
  #endif

  return newFlag;  // set flag true if a button changed state and is ON
}

void initializePedalStateMemory() {

  getAllSavedData();    // initializes pedal state data
}

void getAllSavedData() {
  // make sure if presets haven't ever been saved to EEPROM that you load memory with default config info for first run and save to EEPROM as well
  // check first byte of memory (address 0) for SAVED_PRESET_DIRTY_FLAG to determine if this is needed
  // use EEPROM object.  EEPROM.h is already added so go to town with it.

  SavedDataFormat InputHolder;
  //  // clear out any presets that may be in memory first
  
  #if WIPE_EEPROM
    EEPROM.write(0, 0xff); // conditionally sets virgin EEPROM condition to true...
    Serial.println("Reflowering EEPROM...");
  #endif

  if(EEPROM.read(0) != SAVED_PRESET_DIRTY_FLAG) { // virgin EEPROM condition, save default configuration data to EEPROM (this should only run once ever.
    #if DEBUG
      Serial.println("Detected virgin EEPROM data, saving default configuration to EEPROM.");
      Serial.println("Deflowering EEPROM...");
    #endif
  
    EEPROM.write(0, SAVED_PRESET_DIRTY_FLAG);
    setDefaultPedalState();
    saveCurrentPedalState(false);
  }

  //pull data from EEPROM and save to CurrentPedalState
  #if DEBUG
    Serial.println("Pulling CurrentPedalState from EEPROM...");
  #endif
  EEPROM.get(1, InputHolder);

  CurrentPedalState.setState(InputHolder);

  #if DEBUG
    Serial.print("CurrentPedalState.getLEDMatrix() = "); Serial.println(String(CurrentPedalState.getLEDMatrix(), BIN));
    Serial.print("CurrentPedalState.getLoopStateMatrix() = "); Serial.println(String(CurrentPedalState.getLoopStateMatrix(), BIN));
  #endif
  
}

void saveCurrentPedalState(bool PresetsOnly) {
  #if DEBUG
    Serial.print("Saving CurrentPedalState.state() to EEPROM, PresetsOnly = "); Serial.println(PresetsOnly);
  #endif
  
  if(PresetsOnly) {
    // save only preset data
    #if DEBUG
      Serial.println("Saving presets only to EEPROM...");
      Serial.print("sizeof(CurrentPedalState.state()) = "); Serial.println(sizeof(CurrentPedalState.state()));
      Serial.print("sizeof(CurrentPedalState.state().PresetMatrix) = "); Serial.println(sizeof(CurrentPedalState.state().PresetMatrix));
    #endif
    EEPROM.put(sizeof(CurrentPedalState.state()) - sizeof(CurrentPedalState.state().PresetMatrix),CurrentPedalState.state().PresetMatrix);
  }
  else {
    #if DEBUG
      Serial.println("Saving all CurrentPedalState.state() data");
    #endif
    EEPROM.put(1, CurrentPedalState.state()); // save all CurrentPedalState data, skipping byte 0 which is the EEPROM dirty flag
  }
  
}

void setDefaultPedalState() {

  CurrentPedalState.setPedalMode(PM_PRESET);
  CurrentPedalState.setCurrentPresetIDX(0);
  CurrentPedalState.setButtonState(0x0000);
  CurrentPedalState.setLoopStateMatrix(0x00);
  CurrentPedalState.setLEDMatrix(0x00000000);
  // loop to clear CurrentPedalState.PresetMatrix values
  for(int i = 0; i < (sizeof(CurrentPedalState.state().PresetMatrix) / sizeof(CurrentPedalState.state().PresetMatrix[0])); i++) CurrentPedalState.state().PresetMatrix[i] = 0;
  
}

int getChangedButtonIDX() {
  static int IDX;

  // enumerate bits in ChangedButtonMatrix until you find the first one that is HIGH
  for(IDX=0; IDX < (sizeof(ChangedButtonMatrix)*8); IDX++) if(bitRead(ChangedButtonMatrix, IDX)) break;
  
  return IDX; // returns the index of the least significant HIGH bit in ChangedButtonMatrix
}

void toggleLEDBlinkState() {
  // toggles blink flag on and off at rate defined by BLINK_RATE

  static unsigned long CurrentMillis = 0;
  static unsigned long BlinkPreviousMillis = 0;

  CurrentMillis = millis();
  
  if(getTimeInterval(CurrentMillis, BlinkPreviousMillis) > BLINK_RATE) {
    BlinkToggle = !BlinkToggle;
    BlinkPreviousMillis = CurrentMillis;
  }
  
}

unsigned long getTimeInterval(unsigned long CurrentTime, unsigned long PreviousTime) {
  // this function compensates for the overflow condition that can occur when millis() goes over 50 days

  static unsigned long Interval = 0x00000000;

  if(CurrentTime < PreviousTime) Interval = ((CurrentTime + 0xffffffff) - PreviousTime);
  else                           Interval = (CurrentTime - PreviousTime);
  
  return Interval;
}

void saveOldButtonStates() {
  SavedPedalState.setButtonState(CurrentPedalState.getButtonStateMatrix());
}

void getNewButtonStates() {
  
  // get current state of all inputs and store in ButtonStateMatrix

  CurrentPedalState.setButtonState(STOMP1_IDX, digitalRead(STOMP1_PIN));
  CurrentPedalState.setButtonState(STOMP2_IDX, digitalRead(STOMP2_PIN));
  CurrentPedalState.setButtonState(STOMP3_IDX, digitalRead(STOMP3_PIN));
  CurrentPedalState.setButtonState(STOMP4_IDX, digitalRead(STOMP4_PIN));
  CurrentPedalState.setButtonState(STOMP5_IDX, digitalRead(STOMP5_PIN));
  CurrentPedalState.setButtonState(STOMP6_IDX, digitalRead(STOMP6_PIN));
  CurrentPedalState.setButtonState(STOMP7_IDX, digitalRead(STOMP7_PIN));
  CurrentPedalState.setButtonState(STOMP8_IDX, digitalRead(STOMP8_PIN));

  CurrentPedalState.setButtonState(PM_INPUT_IDX, digitalRead(PM_INPUT_PIN));
  CurrentPedalState.setButtonState(BYPASS_INPUT_IDX, digitalRead(BYPASS_INPUT_PIN));

}


void doBypassMode() {
// do bypass stuff here
  CurrentPedalState.setLEDState(BYPASS_STATUS_IDX, BlinkToggle);
}

void doEditMode() {
  CurrentPedalState.setLEDState(PRESET1_IDX, HIGH); // testing
  
  CurrentPedalState.setLEDState(LOOP1_IDX, BlinkToggle && CurrentPedalState.getButtonState(STOMP1_IDX));
  CurrentPedalState.setLEDState(LOOP2_IDX, BlinkToggle && CurrentPedalState.getButtonState(STOMP2_IDX));
  CurrentPedalState.setLEDState(LOOP3_IDX, BlinkToggle && CurrentPedalState.getButtonState(STOMP3_IDX));
  CurrentPedalState.setLEDState(LOOP4_IDX, BlinkToggle && CurrentPedalState.getButtonState(STOMP4_IDX));
  CurrentPedalState.setLEDState(LOOP5_IDX, BlinkToggle && CurrentPedalState.getButtonState(STOMP5_IDX));
  CurrentPedalState.setLEDState(LOOP6_IDX, BlinkToggle && CurrentPedalState.getButtonState(STOMP6_IDX));
  CurrentPedalState.setLEDState(LOOP7_IDX, BlinkToggle && CurrentPedalState.getButtonState(STOMP7_IDX));
  CurrentPedalState.setLEDState(LOOP8_IDX, BlinkToggle && CurrentPedalState.getButtonState(STOMP8_IDX));

  CurrentPedalState.setLoopState(LOOP1_IDX, BlinkToggle && CurrentPedalState.getButtonState(STOMP1_IDX));
  CurrentPedalState.setLoopState(LOOP2_IDX, BlinkToggle && CurrentPedalState.getButtonState(STOMP2_IDX));
  CurrentPedalState.setLoopState(LOOP3_IDX, BlinkToggle && CurrentPedalState.getButtonState(STOMP3_IDX));
  CurrentPedalState.setLoopState(LOOP4_IDX, BlinkToggle && CurrentPedalState.getButtonState(STOMP4_IDX));
  CurrentPedalState.setLoopState(LOOP5_IDX, BlinkToggle && CurrentPedalState.getButtonState(STOMP5_IDX));
  CurrentPedalState.setLoopState(LOOP6_IDX, BlinkToggle && CurrentPedalState.getButtonState(STOMP6_IDX));
  CurrentPedalState.setLoopState(LOOP7_IDX, BlinkToggle && CurrentPedalState.getButtonState(STOMP7_IDX));
  CurrentPedalState.setLoopState(LOOP8_IDX, BlinkToggle && CurrentPedalState.getButtonState(STOMP8_IDX));

  CurrentPedalState.setLEDState(PM_STATUS_IDX, BlinkToggle);
  CurrentPedalState.setLEDState(BYPASS_STATUS_IDX, HIGH);
  
}

void doPresetMode() {
  CurrentPedalState.setLEDState(LOOP1_IDX, HIGH); // testing
  CurrentPedalState.setLEDState(LOOP2_IDX, HIGH); 
  CurrentPedalState.setLEDState(LOOP3_IDX, HIGH); 
  CurrentPedalState.setLEDState(LOOP4_IDX, HIGH); 
  CurrentPedalState.setLEDState(LOOP5_IDX, HIGH); // testing
  CurrentPedalState.setLEDState(LOOP6_IDX, HIGH); 
  CurrentPedalState.setLEDState(LOOP7_IDX, HIGH); 
  CurrentPedalState.setLEDState(LOOP8_IDX, HIGH); 

  CurrentPedalState.setLoopState(LOOP1_IDX, HIGH); // testing
  CurrentPedalState.setLoopState(LOOP2_IDX, HIGH); 
  CurrentPedalState.setLoopState(LOOP3_IDX, HIGH); 
  CurrentPedalState.setLoopState(LOOP4_IDX, HIGH); 
  CurrentPedalState.setLoopState(LOOP5_IDX, HIGH); // testing
  CurrentPedalState.setLoopState(LOOP6_IDX, HIGH); 
  CurrentPedalState.setLoopState(LOOP7_IDX, HIGH); 
  CurrentPedalState.setLoopState(LOOP8_IDX, HIGH); 

  CurrentPedalState.setLEDState(PRESET1_IDX, HIGH); // testing
  CurrentPedalState.setLEDState(PRESET2_IDX, LOW); 
  CurrentPedalState.setLEDState(PRESET3_IDX, LOW); 
  CurrentPedalState.setLEDState(PRESET4_IDX, LOW); 
  CurrentPedalState.setLEDState(PRESET5_IDX, LOW); 
  CurrentPedalState.setLEDState(PRESET6_IDX, LOW); 
  CurrentPedalState.setLEDState(PRESET7_IDX, LOW); 
  CurrentPedalState.setLEDState(PRESET8_IDX, LOW); 

  CurrentPedalState.setLEDState(PM_STATUS_IDX, HIGH); // testing
  CurrentPedalState.setLEDState(BYPASS_STATUS_IDX, HIGH);
  
}

void doStompboxMode() {
  
  CurrentPedalState.setLEDState(LOOP1_IDX, CurrentPedalState.getButtonState(STOMP1_IDX));
  CurrentPedalState.setLEDState(LOOP2_IDX, CurrentPedalState.getButtonState(STOMP2_IDX));
  CurrentPedalState.setLEDState(LOOP3_IDX, CurrentPedalState.getButtonState(STOMP3_IDX));
  CurrentPedalState.setLEDState(LOOP4_IDX, CurrentPedalState.getButtonState(STOMP4_IDX));
  CurrentPedalState.setLEDState(LOOP5_IDX, CurrentPedalState.getButtonState(STOMP5_IDX));
  CurrentPedalState.setLEDState(LOOP6_IDX, CurrentPedalState.getButtonState(STOMP6_IDX));
  CurrentPedalState.setLEDState(LOOP7_IDX, CurrentPedalState.getButtonState(STOMP7_IDX));
  CurrentPedalState.setLEDState(LOOP8_IDX, CurrentPedalState.getButtonState(STOMP8_IDX));

  CurrentPedalState.setLoopState(LOOP1_IDX, CurrentPedalState.getButtonState(STOMP1_IDX));
  CurrentPedalState.setLoopState(LOOP2_IDX, CurrentPedalState.getButtonState(STOMP2_IDX));
  CurrentPedalState.setLoopState(LOOP3_IDX, CurrentPedalState.getButtonState(STOMP3_IDX));
  CurrentPedalState.setLoopState(LOOP4_IDX, CurrentPedalState.getButtonState(STOMP4_IDX));
  CurrentPedalState.setLoopState(LOOP5_IDX, CurrentPedalState.getButtonState(STOMP5_IDX));
  CurrentPedalState.setLoopState(LOOP6_IDX, CurrentPedalState.getButtonState(STOMP6_IDX));
  CurrentPedalState.setLoopState(LOOP7_IDX, CurrentPedalState.getButtonState(STOMP7_IDX));
  CurrentPedalState.setLoopState(LOOP8_IDX, CurrentPedalState.getButtonState(STOMP8_IDX));

  CurrentPedalState.setLEDState(PRESET1_IDX, LOW);
  CurrentPedalState.setLEDState(PRESET2_IDX, LOW);
  CurrentPedalState.setLEDState(PRESET3_IDX, LOW);
  CurrentPedalState.setLEDState(PRESET4_IDX, LOW);
  CurrentPedalState.setLEDState(PRESET5_IDX, LOW);
  CurrentPedalState.setLEDState(PRESET6_IDX, LOW);
  CurrentPedalState.setLEDState(PRESET7_IDX, LOW);
  CurrentPedalState.setLEDState(PRESET8_IDX, LOW);

  CurrentPedalState.setLEDState(PM_STATUS_IDX, LOW);
  CurrentPedalState.setLEDState(BYPASS_STATUS_IDX, HIGH);

}

// void checkPedalMode() {
//   static int SavePedalMode = CurrentPedalState.getPedalMode();
//   static bool PedalModeInput = LOW;
  
//   PedalModeInput = bitRead(CurrentPedalState.getButtonStateMatrix(), PM_INPUT_IDX);   // checks for mode button

//   if(PedalModeInput && (CurrentPedalState.getPedalMode() == SavePedalMode))  changePedalMode();
//   if(!PedalModeInput && (CurrentPedalState.getPedalMode() != SavePedalMode)) SavePedalMode = CurrentPedalState.getPedalMode();
// }

void changePedalMode() {
  changePedalMode(false);
}

void changePedalMode(bool shortPress) {
  
  #if DEBUG
    Serial.println("setting pedal mode in SavedPedalState.");
  #endif
  SavedPedalState.setPedalMode(CurrentPedalState.getPedalMode());

  #if DEBUG
    Serial.println("setting pedal mode in CurrentPedalState.");
  #endif
  switch(CurrentPedalState.getPedalMode()) {
    case PM_STOMP:    CurrentPedalState.setPedalMode(PM_PRESET);  break;
    case PM_PRESET: if(shortPress) {
                      CurrentPedalState.setPedalMode(PM_STOMP);   break;      // this will go to stompbox unless button is held for BUTTON_HOLD_MAX.... need code!
                    }
                    else {
                      CurrentPedalState.setPedalMode(PM_EDIT);    break;      // this will go to stompbox unless button is held for BUTTON_HOLD_MAX.... need code!
                    }

    case PM_EDIT:     CurrentPedalState.setPedalMode(PM_STOMP);   break;  // this will go back to preset mode with 3 second hold. need code.
    case PM_BYPASS:   CurrentPedalState.toggleBypassMode();       break;  //toggles between bypass mode and whatever mode it was entered from
  }
  
}

void setRelays() {
  // turn relays on or off here...
  #if DEBUG
    //CurrentPedalState.setLoopStateMatrix(0xf0);
    Serial.print("Executing setRelays(), (unsigned int)CurrentPedalState.getLoopStateMatrix() = "); 
    Serial.println(String((unsigned int)CurrentPedalState.getLoopStateMatrix(), BIN));
  #endif
  RelayOutputBuffer.sendOutputMemory((unsigned int)CurrentPedalState.getLoopStateMatrix());
}

void setIndicators() {
  LEDOutputBuffer.sendOutputMemory((unsigned int)CurrentPedalState.getLEDMatrix());
  
  LEDs[PM_STATUS_IDX]    .changeState(bitRead(CurrentPedalState.getLEDMatrix(), PM_STATUS_IDX));
  LEDs[BYPASS_STATUS_IDX].changeState(bitRead(CurrentPedalState.getLEDMatrix(), BYPASS_STATUS_IDX));
}

void updateAllOutputs() {
  updateAllOutputs(false);
}

void updateAllOutputs(bool LEDsOnly) {
  if(!LEDOutputBuffer.isInitialized()) {
    LEDOutputBuffer.initialize(16, SR_CLOCK_PIN, SR_LATCH_PIN, SR_DATA_PIN); // 16 bit buffer for leds
  }
  if(!RelayOutputBuffer.isInitialized()) {
    RelayOutputBuffer.initialize(8, SR2_CLOCK_PIN, SR2_LATCH_PIN, SR2_DATA_PIN); // 8 bit buffer for relays
  }

  toggleLEDBlinkState();         // set flag for blinking LEDs

  if(!LEDsOnly) { 
    // if not LEDs only, update all other outputs here (relays and stuff)
    setRelays();
  }

  setIndicators();
}

void doLEDStartupAnimation() {
  int StartupBlinkRateMultiple = 3;
  int StartupBlinkRate = (BLINK_RATE/StartupBlinkRateMultiple);

  for(int y = 0; y < 1; y++) {  // blink all LEDs once
    for(int i = 0; i < ((MAX_LOOPS * 2) +2); i++) CurrentPedalState.setLEDState(i, HIGH);
    updateAllOutputs(true);
    delay(StartupBlinkRate);
    for(int i = 0; i < ((MAX_LOOPS * 2) +2); i++) CurrentPedalState.setLEDState(i, LOW);
    updateAllOutputs(true);
    delay(StartupBlinkRate);
  }

  
  
  // cycle through all leds and blink once
  for(int i = 0; i < ((MAX_LOOPS * 2) +2); i++) {
    CurrentPedalState.setLEDState(i, HIGH); updateAllOutputs(false); delay(StartupBlinkRate);
    CurrentPedalState.setLEDState(i, LOW);  updateAllOutputs(false); delay(StartupBlinkRate);
  }
  #if DEBUG
    Serial.println("Entering sequenced flash loop for relays.");
  #endif
  for(int i = 0; i < MAX_LOOPS; i++) {
    CurrentPedalState.setLoopState(i, HIGH); updateAllOutputs(false); delay(StartupBlinkRate);
    CurrentPedalState.setLoopState(i, LOW);  updateAllOutputs(false); delay(StartupBlinkRate);
  }



  // for(int y = 0; y < 2; y++) {  // blink all LEDs 2 times and leave all on...
  //   for(int i = 0; i < ((MAX_LOOPS * 2) +2); i++) CurrentPedalState.setLEDState(i, HIGH);
  //   for(int i = 0; i < MAX_LOOPS; i++)            CurrentPedalState.setLoopState(i, HIGH);
  //   updateAllOutputs(false);
  //   delay(StartupBlinkRate);
  // }



  // #if DEBUG
  //   Serial.println("Exiting doLEDStartupAnimation()");
  // #endif
  
  
}
