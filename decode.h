#ifndef DECODE_H
#define DECODE_H

#include <time.h> 
#include "data.h"

//takes raw 406Mhz data and decodes it
void decode_and_display(const char *buffer);

//generates short data burst to print the data
void short_data_burst(COUNTRY_CODE *countryCode, IDENTIFICATION *id, COORD *coords, time_t timeReceived);
#endif