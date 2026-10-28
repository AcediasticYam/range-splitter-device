# Dual Toggle Controller Software for Arduino

This is a repository containing basic controller software built for an Eaton Fuller-style range/splitter PC controller. The controller uses Hall effect switches to monitor the displacement of the two toggles (with embedded magnets) on the shift knob and outputs the signals to an Arduino Leonardo. The software uses [ArduinoJoystickWithFFBLibrary](https://github.com/YukMingLaw/ArduinoJoystickWithFFBLibrary) to handle the USB device side of things.

## Background

The controller this software was designed for was built from an OEM shift knob used in a tractor unit (like an 18-wheeler truck) with a range/splitter transmission. The rationale for using Hall effect sensors was to preserve the "feeling" of the OEM shift knob (which in a real truck runs on pneumatics and uses ball detents), avoiding any switch resistance. There also is not a lot of room, or feasible locations, to affix mechanical switches in the knob.

## Potential Usage

This could be adapted to any controller that uses up to two on/off inputs based on a range of values (e.g. a pull knob like a parking brake). Looping the if statements could add more than two inputs if desired, and parts of the code could be reused for a larger "button box" device.

If available memory gets exhausted from additions the serial logging code can be simplified or removed, I added it because I had to do a lot of debugging of the controller itself and having multiple logging levels was useful.

### Requirements

* A DIY controller with two inputs that provide a range of analog values
 * The range between minimum and maximum values ideally should be greater than 20 (full analog range is 0-1023, 0v-5v)
* An Arduino (tested with a Leonardo, may work on others with analog inputs)
* ArduinoJoystickWithFFBLibrary 1.0 (provided in this repo) 

### Notes

The software attempts to self-calibrate by monitoring min/max values and occasionally artificially reducing the range between min/max to account for input drift. Most values are configurable in constants at the top of the file if issues are encountered. 

> [!CAUTION]
> Once per controller connection, the software will write the current calibration to the EEPROM if there is a substantial (and configurable) distance between the stored value and current value. EEPROMs are rated for 100,000 writes, so you may want to remove that functionality if you want to preserve your EEPROM.