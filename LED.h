#ifndef LED_h     // this wrapper prevents this class from being declared more than once 
#define LED_h
//*******************************************************************************************************************************
#include "DataPin.h"

class LED
{
  public:
    LED();
    void setPin(int pin, bool initialState = LOW);
    int getPinNumber();
    bool currentState();
    void changeState(bool newState);
    void toggleState();
  private:
    DataPin Pin;
};
LED::LED() {
  // constructor... do not use ? was making memory do something weird whenever I tried to use it.
}

void LED::setPin(int pin, bool initialState = LOW) 
{
  // #if DEBUG
  //   Serial.print("Executing LED::setPin() for pin "); Serial.println(pin);
  // #endif

  Pin.initialize(pin, OUTPUT, initialState);
}

int LED::getPinNumber() {
  return Pin.number();
}

void LED::changeState(bool newState){
  Pin.changeState(newState);
}

void LED::toggleState() {
//  #if DEBUG
//    Serial.println("Executing LED::toggleState.");
//  #endif
  Pin.toggleState();
}

bool LED::currentState() {
  return Pin.currentState();
}

//*******************************************************************************************************************************
#endif
