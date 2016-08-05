/*
  Timed State Machine Template
  Arduino ESP8266 see https://github.com/esp8266/Arduino
  by Timothy John Turner
  5/7/16

  This is a starting point for a simple timed 'state machine' for general data acquisition use.
  Rollover is handled and each actual sample time period is avaliable (l_Period) which you can test and compensate for if needed.
  It seems to work well upto around 1000Hz on a 160Mhz ESP8266 as long as you keep your timed code brief.
  
  Software Use Disclaimer
  1. No Warranty
  1.1. "As-Is". The Software is provided "as is," with all faults, defects and errors, and without warranty of any kind.
  1.1. No Liability.
  Licensor does not warrant that the Software will be free of bugs, errors, viruses or other defects,
  and Licensor shall have no liability of any kind for the use of or inability to use the software, the software content or any associated service.

  Licenced under Creative Commons
  Attribution-NonCommercial-ShareAlike 4.0 International
  (CC BY-NC-SA 4.0)
  https://creativecommons.org/licenses/by-nc-sa/4.0/ 
*/

//ESP8266 Android and SDK 
//------------------------------------------------
extern "C" {
#include "user_interface.h"
}
#include <ESP8266WiFi.h>
//------------------------------------------------

#include "Types.h"

// State modes - create as many as needed (each number must be unique)
#define INIT_SYSTEM_STATE 0
#define STREAM_DATA_STATE 1

//spare GPIO for testing accuracy of timed loop
#define testGPIO 2

unsigned long  ul_Current_us = 0UL;
unsigned long  ul_Previous_uS = 0UL;
long l_Current = 0L;
long l_Previous = 0L;
long l_Period = 0L;
unsigned long ul_scanNumber = 0UL; // total scans

// Set timed scan loop period here....
long l_Interval = 1000L; // scan interval in usec

// The state select variable
StateType My_State;

void StateInit(StateType* my_state) {
  my_state->state = INIT_SYSTEM_STATE;
}

void setup() {
  // To minimize timing errors run ESP8266 at max speed (160MHz)
  system_update_cpu_freq(160);

  Serial.begin(74880);
  // Wait for USB Serial
  while (!Serial) {
    yield();
  }
  pinMode(testGPIO, OUTPUT);
  digitalWrite(testGPIO, LOW);
  StateInit(&My_State);
}

/**
  Core state handler 'StateHandler' is placed in an infinite loop at end of main function.
  @param current_state the next state to switch to
*/
void StateHandler(StateType* current_state) {

  // Code before the switch runs BEFORE every time any state is changed!
  // so don't put any var here that's value needs to be remembered between loops...
  int y = 0;
  // System states - must be defined in 'State modes'
  switch (current_state->state)
  {
    case INIT_SYSTEM_STATE:
      //INIT_SYSTEM_STATE code here
      ul_scanNumber = 0UL;
      l_Previous =  0L;
      ul_Previous_uS = 0UL;
      current_state->state = STREAM_DATA_STATE; // switch to next state required
      break;//End of INIT_SYSTEM_STATE code

    case STREAM_DATA_STATE:
      //STREAM_DATA_STATE code here
      ul_Current_us = micros();
      if (ul_Current_us < ul_Previous_uS) //Rollover test and fix
      {
        l_Previous = -1;
      }
      ul_Previous_uS =  ul_Current_us;
      l_Current = (unsigned long)( ul_Current_us / l_Interval );
      if (l_Current  > (l_Previous) ) //100Hz
      {
        l_Period = l_Current - l_Previous; // the current timing period 
        ul_scanNumber++;
        //Put timed code here  - code execution time must be << timing period
        
        //testing only-----------
        digitalWrite(testGPIO, HIGH); //for timing code
        delayMicroseconds(100);
        digitalWrite(testGPIO, LOW);
        //-----------------------

        l_Previous = l_Current;
      }
      current_state->state = STREAM_DATA_STATE;
      break; //End of STREAM_DATA_STATE
  }// end of state cases
  // Code here runs AFTER any exit from any state via break

};

void loop() {
  StateHandler(&My_State);
}
