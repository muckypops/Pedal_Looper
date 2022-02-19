// ************************************* Constants *****************************************************
#define DEBUG 0               // turns on debugging code, set to 0 for production code, 
                              // saves a significant amount of memory when commented
#define WIPE_EEPROM 0         // set to 1 to reinitialize and test virgin EEPROM condition

#define MAX_LOOPS 8           // maximum available loops

#define BLINK_RATE 333        // default led blink rate
#define BUTTON_HOLD_MAX 1000  // max hold time for hold button operations like switching in and out of edit mode, etc...

#define SAVED_PRESET_DIRTY_FLAG 0x49  // 1001001 from the RUSH song "The Body Electric" from the album "Grace Under Pressure"
                                      // if this is not stored in byte[0] of EEPROM it means EEPROM data needs to be initialized
                                      // with default configuration data.

// LED output pins ***************************************************************************************
#define SR_LATCH_PIN A4
#define SR_CLOCK_PIN A3
#define SR_DATA_PIN A2

// Relay output pins ***************************************************************************************
#define SR2_LATCH_PIN 13 //A7
#define SR2_CLOCK_PIN 12 //A6
#define SR2_DATA_PIN 11 //A5

// Bypass and Mode Status LED Pins **********************************************************************
// these are treated separately so that I can use 2 serial output chips instead of 3.,
#define PM_STATUS_PIN A0
#define BYPASS_STATUS_PIN A1

// Button input pins ************************************************************************************
#define STOMP1_PIN 3         // Loop switch input pins
#define STOMP2_PIN 4       // when you switch to the shift register outputs
#define STOMP3_PIN 5       // use pins 3 - 12 for the inputs
#define STOMP4_PIN 6
#define STOMP5_PIN 7 //7 -1 values for pins turn off the pin when DataPin object is utilized
#define STOMP6_PIN 8 //8
#define STOMP7_PIN 9 //9
#define STOMP8_PIN 10 //10

#define PM_INPUT_PIN A5 //11        // Pedal Mode input pin
#define BYPASS_INPUT_PIN 2 //12   // Bypass Input Pin THIS WILL BE on PIN 12


// Input status indexes for data stored in ButtonStateMatrix, etc... ******************************************
#define STOMP1_IDX 0          // Loop 1 button
#define STOMP2_IDX 1          // Loop 2 button
#define STOMP3_IDX 2          // Loop 3 button
#define STOMP4_IDX 3          // Loop 4 button
#define STOMP5_IDX 4          // Loop 5 button
#define STOMP6_IDX 5          // Loop 6 button
#define STOMP7_IDX 6          // Loop 7 button
#define STOMP8_IDX 7          // Loop 8 button

#define PM_INPUT_IDX 8        // Mode button
#define BYPASS_INPUT_IDX 9    // Bypass button

// LED status indicators for data stored in LEDMatrix **************************************************
#define LOOP1_IDX 0           // Loop LEDs
#define LOOP2_IDX 1
#define LOOP3_IDX 2
#define LOOP4_IDX 3
#define LOOP5_IDX 4
#define LOOP6_IDX 5
#define LOOP7_IDX 6
#define LOOP8_IDX 7

#define PRESET1_IDX 8         // Preset LEDs
#define PRESET2_IDX 9
#define PRESET3_IDX 10
#define PRESET4_IDX 11
#define PRESET5_IDX 12
#define PRESET6_IDX 13
#define PRESET7_IDX 14
#define PRESET8_IDX 15

#define PM_STATUS_IDX 16     // Mode LED
#define BYPASS_STATUS_IDX 17  // bypass LED

// Pedal mode constants (current pedal status stored in PedalMode) *************************************
#define PM_STOMP 1
#define PM_PRESET 2
#define PM_EDIT 3
#define PM_BYPASS 4

// *******************************************************************************************************
