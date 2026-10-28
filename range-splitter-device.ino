#include "Joystick.h"
#include "constants.h"
#include "logging.h"
#include <EEPROM.h>

Joystick_ Joystick;

// Tracks toggle activation state, true is activated: [A0, A1].
bool lastToggleState[2] = {false, false};

// This stores the range of max/min values for each input:
// [aMin, aMax, bMin, bMax]
unsigned int toggleRange[4] = {0, 0, 0, 0};

// Toggle auto-calibration adjustment counter: [A0, A1].
// Max values are defined by MAX_CALIBRATIONS_PER_PRESS.
unsigned int toggleCalibrations[2] = {0, 0};

// Toggle press count tracker: [A0, A1]. Used for the calibration update threshold.
unsigned int toggleActivations[2] = {0, 0};

// Tracks whether calibration has occurred. Only happens after the controller is connected and
// a sufficient amount of toggle activation occur.
bool calibrationUpdated = false;

// Count of full loop() method invocations.
unsigned long loopCount = 0;

// Setup method, called at start.
void setup() {
  Serial.begin(9600);
  delay(2500); // Ensures log messages are seen.

  Joystick.begin();
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);

  // Set the calibration values stored in the EEPROM.
  unsigned int eepromValue;
  EEPROM.get(EEPROM_STORAGE_START_ADDR, eepromValue);
  toggleRange[0] = eepromValue + EEPROM_LOAD_VALUE_ADJ;

  EEPROM.get(EEPROM_STORAGE_START_ADDR + 2, eepromValue);
  toggleRange[1] = eepromValue - EEPROM_LOAD_VALUE_ADJ;

  EEPROM.get(EEPROM_STORAGE_START_ADDR + 4, eepromValue);
  toggleRange[2] = eepromValue + EEPROM_LOAD_VALUE_ADJ;
  
  EEPROM.get(EEPROM_STORAGE_START_ADDR + 6, eepromValue);
  toggleRange[3] = eepromValue - EEPROM_LOAD_VALUE_ADJ;

  if (DEBUG_LEVEL >= VERBOSE) {
    char rangeStrBuffer[64];
    sprintf(rangeStrBuffer, "EEPROM init range\tA0: %.4u to %.4u\tA1: %.4u to %.4u", \
      toggleRange[0], toggleRange[1], toggleRange[2], toggleRange[3]);
    outln(VERBOSE, rangeStrBuffer);
  }

  outln(NOTICE, F("Setup complete"));
}

// Loop method, called every tick
void loop() {
  // Read pins
  int aPin0 = analogRead(A0);
  int aPin1 = analogRead(A1);

  // Update min/max values
  if (aPin0 < toggleRange[0]) {
    toggleRange[0] = aPin0;
  }
  if (aPin0 > toggleRange[1]) {
    toggleRange[1] = aPin0;
  }
  if (aPin1 < toggleRange[2]) {
    toggleRange[2] = aPin1;
  }
  if (aPin1 > toggleRange[3]) {
    toggleRange[3] = aPin1;
  }

  // The next two if statements nudge the range inwards to account for drift from the sensors. It will only run up to a 
  // configured number of times for each toggle before the toggle is presed again. The first if statement is for toggle 0
  // and the next for toggle 1. This only occurs after the calibration is updated in the EEPROM.
  if (!calibrationUpdated && toggleCalibrations[0] <= MAX_CALIBRATIONS_PER_PRESS && loopCount % RECALIBRATE_OFFSET_INTERVAL == 0) {
    toggleRange[0]++; // nudge lower bound inward to mean
    toggleRange[1]--; // nudge upper bound inward to mean
    toggleCalibrations[0]++; // track calibration count
    outln(VERBOSE, F("A0 calibration nudge"));
  }

  if (!calibrationUpdated && toggleCalibrations[1] <= MAX_CALIBRATIONS_PER_PRESS && loopCount % RECALIBRATE_OFFSET_INTERVAL == 0) {
    toggleRange[2]++; // nudge lower bound inward to mean
    toggleRange[3]--; // nudge upper bound inward to mean
    toggleCalibrations[1]++; // track calibration count
    outln(VERBOSE, F("A1 calibration nudge"));
  }

  if (DEBUG_LEVEL >= VERBOSE) {
    char rangeStrBuffer[64];
    sprintf(rangeStrBuffer, "Input range\tA0: %.4u to %.4u\tA1: %.4u to %.4u", \
      toggleRange[0], toggleRange[1], toggleRange[2], toggleRange[3]);
    outln(VERBOSE, rangeStrBuffer);
  }

  // Calculate the mean of the lower and upper bound for A0 and A1 inputs
  unsigned int midPointA0 = toggleRange[0] + (unsigned int)((toggleRange[1] - toggleRange[0]) / 2);
  unsigned int midPointA1 = toggleRange[2] + (unsigned int)((toggleRange[3] - toggleRange[2]) / 2);
  
  // Check less than/greater than mean for A0 and if the last state does not match update the joystick value.
  if (aPin0 < midPointA0 && lastToggleState[0] == true) {
    lastToggleState[0] = false; // update internal state
    toggleCalibrations[0] = 0; // clear auto calibration counter
    Joystick.setButton(0, false); // update state via USB library
    outln(INFO, F("A0 is now OFF"));
    toggleActivations[0]++;
  }
  else if (aPin0 >= midPointA0 && lastToggleState[0] == false)
  {
    lastToggleState[0] = true;
    toggleCalibrations[0] = 0;
    Joystick.setButton(0, true);
    outln(INFO, F("A0 is now ON"));
    toggleActivations[0]++;
  }

  // Check less than/greater than mean for A0 and if the last state does not match update the joystick value.
  if (aPin1 < midPointA1 && lastToggleState[1] == true) {
    lastToggleState[1] = false; // update internal state
    toggleCalibrations[1] = 0; // clear auto calibration counter
    Joystick.setButton(1, false); // update state via USB library
    outln(INFO, F("A1 is now OFF"));
    toggleActivations[1]++;
  }
  else if (aPin1 >= midPointA1 && lastToggleState[1] == false)
  {
    lastToggleState[1] = true;
    toggleCalibrations[1] = 0;
    Joystick.setButton(1, true);
    outln(INFO, F("A1 is now ON"));
    toggleActivations[1]++;
  }

  // Update the loop counter
  loopCount++;

  // Only update calibration if the user has activated both toggles a specific amount of times and a calibration update 
  // has not occurred yet. Calibration stores in the EEPROM, which should only be updated sparingly.
  if (calibrationUpdated == false && toggleActivations[0] > CALIBRATION_UPDATE_ACTIVATION_THRESHOLD && toggleActivations[1] > CALIBRATION_UPDATE_ACTIVATION_THRESHOLD) {
    update_stored_calibration();
    calibrationUpdated = true;
  }

  delay(DELAY_MS);
}

// This method updates the calibration values stored in the EEPROM if they are substantially different from
// the current observed values.
void update_stored_calibration() {
  char diffFmtStr[] = "Change was %u";
  char diffStr[24];

  unsigned int eepromValue;
  unsigned int changeCount = 0;
  EEPROM.get(EEPROM_STORAGE_START_ADDR, eepromValue);
  if (abs(toggleRange[0] - eepromValue) >= EEPROM_STORE_DIFF_THRESHOLD) {
    outln(INFO, F("Updating min threshold of A0 in EEPROM."));
    EEPROM.put(EEPROM_STORAGE_START_ADDR, toggleRange[0]);

    sprintf(diffStr, diffFmtStr, abs(toggleRange[0] - eepromValue));
    outln(VERBOSE, diffStr);

    changeCount++;
  }

  EEPROM.get(EEPROM_STORAGE_START_ADDR + 2, eepromValue);
  if (abs(toggleRange[1] - eepromValue) >= EEPROM_STORE_DIFF_THRESHOLD ) {
    outln(INFO, F("Updating max threshold of A0 in EEPROM."));
    outln(VERBOSE, F("Updating max threshold of A0 in EEPROM."));
    EEPROM.put(EEPROM_STORAGE_START_ADDR + 2, toggleRange[1]);
    
    sprintf(diffStr, diffFmtStr, abs(toggleRange[1] - eepromValue));
    outln(VERBOSE, diffStr);

    changeCount++;
  }

  EEPROM.get(EEPROM_STORAGE_START_ADDR + 4, eepromValue);
  if (abs(toggleRange[2] - eepromValue) >= EEPROM_STORE_DIFF_THRESHOLD) {
    outln(INFO, F("Updating min threshold of A1 in EEPROM."));
    EEPROM.put(EEPROM_STORAGE_START_ADDR + 4, toggleRange[2]);
    
    sprintf(diffStr, diffFmtStr, abs(toggleRange[2] - eepromValue));
    outln(VERBOSE, diffStr);

    changeCount++;
  }

  EEPROM.get(EEPROM_STORAGE_START_ADDR + 6, eepromValue);
  if (abs(toggleRange[3] - eepromValue) >= EEPROM_STORE_DIFF_THRESHOLD) {
    outln(INFO, F("Updating max threshold of A1 in EEPROM."));
    EEPROM.put(EEPROM_STORAGE_START_ADDR + 6, toggleRange[3]);
    
    sprintf(diffStr, diffFmtStr, abs(toggleRange[3] - eepromValue));
    outln(VERBOSE, diffStr);

    changeCount++;
  }

  char str[64];
  sprintf(str, "Stored calibration check complete, %u changes.", changeCount);
  outln(NOTICE, str);
}