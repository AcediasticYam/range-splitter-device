#include <Arduino.h>
#include <WString.h>
#include "constants.h"
#include "logging.h"

// Output function for F() macro strings.
void outln(unsigned int level, const __FlashStringHelper* out) {   
  if (DEBUG_LEVEL >= level) {
    output_prefix(level);
    Serial.println(out);
  } 
}

// Output function for strings.
void outln(unsigned int level, const char* out) {   
  if (DEBUG_LEVEL >= level) {
    output_prefix(level);
    Serial.println(out);
  } 
}

// Output function for integers.
void outln(unsigned int level, int out) {   
  if (DEBUG_LEVEL >= level) {
    output_prefix(level);
    Serial.println(String(out));
  } 
}

// Prefix output function. Outputs "[hhhh:mm:SS.ssss][<log level>] "
void output_prefix(unsigned int level) {
  char timeBuffer[14];
  
  // e.g [0001:25:36.789] = 1 hour, 25 min...
  sprintf(timeBuffer, "[%.4u:%.2u:%.2u.%.3u]",       \ 
    ((unsigned int) floor(millis() / 3600000)) % 60, \ 
    ((unsigned int) floor(millis() / 60000)) % 60,   \ 
    ((unsigned int) floor(millis() / 1000)) % 60,    \ 
    millis() % 1000                                  \ 
  );

  Serial.print(timeBuffer);
  switch (level) {
    case VERBOSE:
      Serial.print(F("[VERBOSE] "));
        break;
    case INFO:
      Serial.print(F("[INFO   ] "));
        break;
    case NOTICE:
      Serial.print(F("[NOTICE ] "));
        break;
  }
}