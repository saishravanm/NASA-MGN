#include <stdio.h>
#include <time.h>

void generate_kml(const char *filename, double latitude, double longitude, const char *country_code, const char *beacon_id, time_t timestamp) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Error opening file");
        return;
    }
    
    char time_str[50];
    struct tm *timeinfo = localtime(&timestamp);
    strftime(time_str, sizeof(time_str), "%Y-%m-%dT%H:%M:%SZ", timeinfo);

    fprintf(file, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(file, "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n");
    fprintf(file, "  <Document>\n");
    fprintf(file, "    <name>Beacon Locations</name>\n");
    fprintf(file, "    <Placemark>\n");
    fprintf(file, "      <name>Beacon ID: %s</name>\n", beacon_id);
    fprintf(file, "      <description>Country Code: %s\nTime: %s</description>\n", country_code, time_str);
    fprintf(file, "      <Point>\n");
    fprintf(file, "        <coordinates>%f,%f,0</coordinates>\n", longitude, latitude);
    fprintf(file, "      </Point>\n");
    fprintf(file, "    </Placemark>\n");
    fprintf(file, "  </Document>\n");
    fprintf(file, "</kml>\n");

    fclose(file);
}
/*
int main() {
    double latitude = 32.7767;
    double longitude = -96.7970;
    const char *country_code = "US";
    const char *beacon_id = "Beacon12345";

    time_t current_time;
    time(&current_time);

    generate_kml("beacon_locations.kml", latitude, longitude, country_code, beacon_id, current_time);
    printf("KML file generated successfully.\n");

    return 0;
}

*/

