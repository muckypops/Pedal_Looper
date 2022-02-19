#ifndef PedalData_h     // this wrapper prevents this class from being declared more than once 
#define PedalData_h
//*******************************************************************************************************************************
//#include "LED.h"
struct SavedDataFormat {
    int PedalMode;
    int CurrentPresetIDX;
    int ButtonStateMatrix;
    byte LoopStateMatrix;   // needs set/get
    unsigned long LEDMatrix;  
    unsigned int PresetMatrix[MAX_LOOPS]; // this must always remain the last member of this struct for the EEPROM code to work...
                                          // NEEDS TO BE FIXED!!!!!
                                          // needs set/get
};

class PedalData {
  public:
    PedalData();
    SavedDataFormat state();
    void resetButtonStateMatrix();
    void setState(SavedDataFormat newState);
    int getPedalMode();
    void setPedalMode(int newPedalMode);
    int getLEDState(int idx);
    void setLEDState(int idx, bool newValue);
    int getCurrentPresetIDX();
    void setCurrentPresetIDX(int newIDX);
    int getButtonStateMatrix();
    void setLEDMatrix(unsigned long newLEDMatrix);
    unsigned long getLEDMatrix();
    byte getLoopStateMatrix();
    void setLoopState(int idx, bool newLoopState);
    void setLoopStateMatrix(byte newLoopStateMatrix);
    bool getButtonState(int idx);
    void setButtonState(int idx, bool newButtonState);
    void setButtonState(int newButtonStateMatrix);
    void toggleLEDState(int idx);
    void toggleBypassMode();
  private:
    SavedDataFormat State;      // allows access to old PedalData struct for data storage in EEPROM, needs to be removed 
};


PedalData::PedalData() {
  // constructor... do not use ? was making memory do something weird whenever I tried to use it.
}

void PedalData::toggleBypassMode() {
  static int SavedPedalMode;
  #if DEBUG
    Serial.println("Executing PedalData::toggleBypassMode()");
  #endif

  if(PedalData::getPedalMode() == PM_BYPASS) {  // if i'm in bypass
    PedalData::setPedalMode(SavedPedalMode);    // flip to saved pedalmode from when i switched to bypass  

  }
  else {                                        // if not in bypass
    SavedPedalMode = PedalData::getPedalMode(); // Save current pedal mode
    if(SavedPedalMode == PM_EDIT) SavedPedalMode = PM_PRESET; // switch saved pedal mode from edit to preset if necessary...
    PedalData::setPedalMode(PM_BYPASS);         // switch to bypass
  }

}

unsigned long PedalData::getLEDMatrix() {
  return PedalData::State.LEDMatrix;
}

SavedDataFormat PedalData::state() {
  return State;
}

void PedalData::setLEDMatrix(unsigned long newLEDMatrix) {
  PedalData::State.LEDMatrix = newLEDMatrix;
}

byte PedalData::getLoopStateMatrix() {
  return PedalData::State.LoopStateMatrix;
}

void PedalData::setLoopStateMatrix(byte newLoopStateMatrix) {
  PedalData::State.LoopStateMatrix = newLoopStateMatrix;
}

void PedalData::resetButtonStateMatrix() {
  PedalData::State.ButtonStateMatrix = 0x0000;
  //for(int i = 0; i < (MAX_LOOPS + 2); i++) bitWrite(PedalData::State.ButtonStateMatrix, i, LOW);
  // #if DEBUG 
  //   Serial.println("Executed PedalData::resetButtonStateMatrix():"); 
  //   Serial.print("New value of PedalData::State.ButtonStateMatrix is "); Serial.println(String(PedalData::State.ButtonStateMatrix, BIN));
  // #endif
}

void PedalData::setState(SavedDataFormat newState){

  PedalData::State.PedalMode = newState.PedalMode;
  PedalData::State.CurrentPresetIDX   = newState.CurrentPresetIDX;
  PedalData::State.ButtonStateMatrix  = newState.ButtonStateMatrix;
  PedalData::State.LoopStateMatrix    = newState.LoopStateMatrix;
  PedalData::State.LEDMatrix          = newState.LEDMatrix;
  for(int i = 0; i < MAX_LOOPS; i++)  PedalData::State.PresetMatrix[i] = newState.PresetMatrix[i];
  
  
}


int PedalData::getCurrentPresetIDX() {
  return PedalData::State.CurrentPresetIDX;
}
void PedalData::setCurrentPresetIDX(int newIDX) {
  PedalData::State.CurrentPresetIDX = newIDX;
}

void PedalData::setButtonState(int newButtonStateMatrix) {
  PedalData::State.ButtonStateMatrix = newButtonStateMatrix;
}

void PedalData::setButtonState(int idx, bool newButtonState) {
  bitWrite(PedalData::State.ButtonStateMatrix, idx, newButtonState);
}

bool PedalData::getButtonState(int idx) {
  return bitRead(PedalData::State.ButtonStateMatrix, idx);
}

int PedalData::getButtonStateMatrix() {
  return PedalData::State.ButtonStateMatrix;
}

void PedalData::setLoopState(int idx, bool newLoopState) {
  if(idx < (sizeof(PedalData::State.LoopStateMatrix) * 8)) {
    bitWrite(PedalData::State.LoopStateMatrix, idx, newLoopState);
  }
}

void PedalData::setLEDState(int idx, bool newValue) {
  if(idx < (sizeof(PedalData::State.LEDMatrix) * 8)) {
    bitWrite(PedalData::State.LEDMatrix, idx, newValue);
  }
  #if DEBUG
//    Serial.print("Executing PedalData::setLEDState(idx = "); Serial.print(idx); Serial.print(", newValue = "); Serial.println(newValue);
//    Serial.print("PedalData::State.LEDMatrix = "); Serial.println(String(PedalData::State.LEDMatrix, BIN));
  #endif
}

void PedalData::toggleLEDState(int idx) {
  bitWrite(PedalData::State.LEDMatrix, idx, !bitRead(PedalData::State.LEDMatrix, idx));
}

int PedalData::getPedalMode() {
  return PedalData::State.PedalMode;
}

void PedalData::setPedalMode(int newPedalMode) {
  switch(newPedalMode) {
    case PM_BYPASS:
    case PM_STOMP:
    case PM_PRESET:
    case PM_EDIT:
      #if DEBUG
        Serial.print("Pedal mode changed to "); Serial.print(newPedalMode); Serial.println(" in PedalData::setPedalMode()");
      #endif
      PedalData::State.PedalMode = newPedalMode;
      break;
    default:
      #if DEBUG
        Serial.print("Invalid pedal mode "); Serial.print(newPedalMode); Serial.println(" passed to PedalData::setPedalMode()");
      #endif
      break;
  }
}

//*******************************************************************************************************************************
#endif
