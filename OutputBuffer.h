#ifndef OutputBuffer_h     // this wrapper prevents this class from being declared more than once 
#define OutputBuffer_h
//*******************************************************************************************************************************
#include <arduino.h>
#include "DataPin.h"

class OutputBuffer
{
  public:
    OutputBuffer();
    void initialize(int maxBits, int clockPin, int latchPin, int dataPin);
    unsigned int outputMemory();
    bool isInitialized();
    void clearOutputMemory();
    void sendOutputMemory();
    void sendOutputMemory(unsigned int outputMemory);
    void setOutputMemory(unsigned int outputMemory);
  private:
    unsigned int OutputMemory = 0x0000; // 16 bits, 8 for each 595 chip
    unsigned int SavedOutputMemory = 0x0000;
    int MaxBits;
    DataPin ClockPin;
    DataPin LatchPin;
    DataPin DataPin;
    bool IsInitialized = false;
};
OutputBuffer::OutputBuffer() {
//  #if DEBUG
//    Serial.println("Executing OutputBuffer::OutputBuffer()");
//  #endif

  // constructor... do not use ? was making memory do something weird whenever I tried to use it.
  
}

bool OutputBuffer::isInitialized() {
  return IsInitialized;
}

void OutputBuffer::sendOutputMemory(unsigned int outputMemory) {
  OutputBuffer::setOutputMemory(outputMemory);
  OutputBuffer::sendOutputMemory();
}

void OutputBuffer::sendOutputMemory() {
  byte OutputByte = 0xff;
  bool GoFlag = false;
  

  // #if DEBUG
  //   Serial.print("Executing OutputBuffer::sendOutputMemory(), OutputMemory = "); Serial.println(String(OutputMemory, BIN));
  // #endif

  GoFlag = SavedOutputMemory != OutputMemory;

  if(GoFlag) { // outputmemory changed
    #if DEBUG
      // if(ClockPin.number() == SR2_CLOCK_PIN) {  // output for relay buffer only
        Serial.println("OutputBuffer::sendOutputMemory() GoFlag = true.");
        Serial.print("OutputMemory      = "); Serial.println(String(OutputMemory,BIN));
        Serial.print("SavedOutputMemory = "); Serial.println(String(SavedOutputMemory,BIN));
        Serial.print("DataPin.number() = "); Serial.println(DataPin.number());
        Serial.print("ClockPin.number() = "); Serial.println(ClockPin.number());
        Serial.print("LatchPin.number() = "); Serial.println(LatchPin.number());
      // }
    #endif
    SavedOutputMemory = OutputMemory;
    if(OutputBuffer::isInitialized()) {
      // take the latchPin low so 
      // the LEDs don't change while you're sending in bits:
      ClockPin.changeState(LOW);
      LatchPin.changeState(LOW);
    
      for (int i = 1; i <= (MaxBits / 8); i++) {
    
        for(int y = 0; y < 8; y++) {  // shifts bits into OutputByte for output
          bitWrite(OutputByte, y, bitRead(OutputMemory, (i * 8) - (y + 1)));
          // #if DEBUG
          //   Serial.print("shiftOut(, OutputByte = "); Serial.print(String(OutputByte, BIN));
          //   Serial.print(", i = "); Serial.println(i);
          //   Serial.print(", y = "); Serial.println(y);
          // #endif
        }
        // #if DEBUG
        //   Serial.print("shiftOut() OutputByte = "); Serial.print(String(OutputByte, BIN));
        //   Serial.print(", i = "); Serial.println(i);
        // #endif
        // shift out the bits:
        shiftOut(DataPin.number(), ClockPin.number(), LSBFIRST, OutputByte);  
        ClockPin.changeState(LOW);
      }   
      //take the latch pin high so the LEDs will light up:
      LatchPin.changeState(HIGH);
      LatchPin.changeState(LOW);
    }
    else {
      #if DEBUG
        Serial.println("Executing OutputBuffer::sendOutputMemory() failed, IsInitialized = false");
      #endif
    }
  }
}

void OutputBuffer::setOutputMemory(unsigned int newOutputMemory) {
  OutputMemory = newOutputMemory;
}

unsigned int OutputBuffer::outputMemory() {
  return OutputMemory;
}

void OutputBuffer::initialize(int maxBits, int clockPin, int latchPin, int dataPin) {
  // #if DEBUG
  //   Serial.print("Executing OutputBuffer::initialize(). maxBits = "); Serial.println(maxBits);
  // #endif

  byte OutputByte = 0; // mem to hold output byte
  // maxBits cannot exceed the size of the output buffer memory block

  if(maxBits > (sizeof(OutputMemory) * 8)) { // if maxBits is too large it will be set to fit outputMemory's size
    MaxBits = (sizeof(OutputMemory) * 8);
  }
  else {
    MaxBits = maxBits;
  }
  
  // #if DEBUG
  //   Serial.print("Just set MaxBits = "); Serial.println(MaxBits);
  // #endif
  ClockPin.initialize(clockPin, OUTPUT);
  LatchPin.initialize(latchPin, OUTPUT);
  DataPin.initialize(dataPin, OUTPUT);

  #if DEBUG
    Serial.print("Outbuffer::Initialize() dataPin = "); Serial.println(dataPin);
    Serial.print("Outbuffer::Initialize() clockPin = "); Serial.println(clockPin);
    Serial.print("Outbuffer::Initialize() latchPin = "); Serial.println(latchPin);
  #endif

  
  IsInitialized = true; // flag has to be set for clear and send functions to work.

  // clear out bits in OutputMemory
  clearOutputMemory();

  // data to the outputs
  sendOutputMemory();
  
}

void OutputBuffer::clearOutputMemory() {
  // #if DEBUG
  //   Serial.println("Executing OutputBuffer::clearOutputMemory()");
  // #endif
  if(IsInitialized) { // if object has been initialized, set all bits in OutputMemory to LOW
    for(int i = 0; i < MaxBits; i++) bitWrite(OutputMemory, i, LOW);
  

    // #if DEBUG
    //   Serial.print("OutputMemory = "); Serial.println(String(OutputMemory, BIN));
    // #endif
  }

}



// *******************************************************************************************************************************
#endif
