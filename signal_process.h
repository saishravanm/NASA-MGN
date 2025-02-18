#ifndef SIGNAL_PROCESS_H
#define SIGNAL_PROCESS_H
#include <stdint.h>
#include <iio.h>

//returns 1 if detected, 0 if not, negative on errors
int check_frequency(uint64_t frequency);

//tries to detect sarsat signal and returns pointer to raw data
char* detect_sarsat_signal(uint64_t frequency);

//adalm pluto logic
int beacon_search(void);
#endif