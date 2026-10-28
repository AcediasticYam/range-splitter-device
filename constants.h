// Debugging constants. Used for logging.
const unsigned int VERBOSE = 3;
const unsigned int INFO = 2;
const unsigned int NOTICE = 1;
const unsigned int DEBUG_LEVEL = NOTICE; // Set to NOTICE for normal logging levels.

// The delay added to each iteration loop. Since the controller is an on/off toggle, we don't need
// to stress the processor with higher resolution.
const unsigned int DELAY_MS = 200;

// How often to "nudge" the stored min/max values (per loop iteration). This is meant to help with
// "drift" as the controller is used.
const unsigned int RECALIBRATE_OFFSET_INTERVAL = 30;

// How many nudges to perform until the toggle is activated again. Higher values may cause the toggles to
// be misread.
const unsigned int MAX_CALIBRATIONS_PER_PRESS = 4;

// How many times each toggle must be activated before the calibration in the EEPROM is validated.
const unsigned int CALIBRATION_UPDATE_ACTIVATION_THRESHOLD = 20;

// EEPROM beginning storage address.
const unsigned int EEPROM_STORAGE_START_ADDR = 0;

// Threshold before updating EEPROM value, current value must be +/- this constant. Lower values will cause
// excessive EEPROM writes, which will wear it out faster.
const unsigned int EEPROM_STORE_DIFF_THRESHOLD = 15;

// Used to slightly narrow the min/max from the loaded EEPROM values to capture drift from last usage.
// Higher values will prevent the loaded values from being correct initially.
const unsigned int EEPROM_LOAD_VALUE_ADJ = 4;