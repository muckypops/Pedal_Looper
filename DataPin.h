#ifndef DataPin_h     // this wrapper prevents this class from being declared more than once 
#define DataPin_h
//*******************************************************************************************************************************


class DataPin
{
  public:
    DataPin();
    void initialize(int pin, int mode, bool initialState = LOW);
    bool currentState();
    int number();
    int mode();
    void changeState(bool newState);
    void toggleState();
  private:
    bool PinState;
    int Pin;
    int Mode;
    bool IsInitialized = false;
};
DataPin::DataPin() {
  // constructor... do not use ? was making memory do something weird whenever I tried to use it.
  
}

int DataPin::number() {
  return Pin;
}

void DataPin::initialize(int pin, int mode, bool initialState = LOW) 
{
//  #if DEBUG
//    Serial.print("Executing DataPin::initialize("); Serial.print(pin); Serial.print(","); Serial.print(mode); Serial.println(").");
//  #endif

  if(pin != -1) {
    DataPin::IsInitialized = true; // value of -1 means pin isn't being utilized.
  
    DataPin::PinState = initialState;
    DataPin::Pin = pin;
    DataPin::Mode = mode;
//    if(Mode == INPUT) {
//      pinMode(Pin, OUTPUT);
//      digitalWrite(Pin, LOW);
//    }
    pinMode(pin, mode);
    
    if(mode == OUTPUT) digitalWrite(pin, initialState);
  }
  else {
//    #if DEBUG
//      Serial.println("pin = -1, pin not initialized.");
//    #endif
  }

}

int DataPin::mode() {
  return Mode;
}

void DataPin::changeState(bool newState){
  if(IsInitialized) {
    PinState = newState;
    digitalWrite(Pin, newState);
  }
  else {
    // #if DEBUG
    //   Serial.print("IsInitialized is false, pin ");Serial.print(Pin); Serial.println(" state not changed.");
    // #endif
  }
}

void DataPin::toggleState() {
//  #if DEBUG
//    Serial.print("Executing DataPin::toggleState. PinState = "); Serial.println(PinState);
//  #endif
  if(IsInitialized) {
    PinState = !PinState;
  
  //  #if DEBUG
  //    Serial.print("Executing DataPin::toggleState. New PinState = "); Serial.println(PinState);
  //  #endif
  
    digitalWrite(Pin, PinState);
  }
  else {
//    #if DEBUG
//      Serial.println("IsInitialized is false, pin state not toggled.");
//    #endif
  }
}

bool DataPin::currentState() {
  return PinState;
}


//*******************************************************************************************************************************
#endif
