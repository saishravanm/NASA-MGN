#ifndef KML_GENERATION_H
#define KML_GENERATION_H

#include <time.h>

void generate_kml(const char *filename, double latitude, double longitude, const char *country_code, const char *beacon_id, time_t timestamp);

#endif