#ifndef Button_h     // this wrapper prevents this class from being declared more than once 
#define Button_h
//*******************************************************************************************************************************
#include "DataPin.h"

class Button
{
  public:
    Button();
    void setPin(int pin);
    bool currentState();
    unsigned long holdDuration();
  private:
    DataPin Pin;
};
Button::Button() {
  // constructor... do not use ? was making memory do something weird whenever I tried to use it.
}

unsigned long Button::holdDuration() {
  return 0;
}

void Button::setPin(int pin) 
{
//  #if DEBUG
//    Serial.print("Executing Button::setPin("); Serial.print(pin); Serial.print(","); Serial.print(initialState); Serial.println(").");
//  #endif
  
  Pin.initialize(pin, INPUT, LOW);
  // Pin.changeState(LOW);
}

bool Button::currentState() {
  return Pin.currentState();
}

//*******************************************************************************************************************************
#endif
