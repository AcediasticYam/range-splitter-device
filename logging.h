// Output function for F() macro strings.
void outln(unsigned int level, const __FlashStringHelper* out);

// Output function for strings.
void outln(unsigned int level, const char* out);

// Output function for integers.
void outln(unsigned int level, int out);

// Prefix output function. Outputs "[hhhh:mm:SS.ssss][<log level>] "
void output_prefix(unsigned int level);