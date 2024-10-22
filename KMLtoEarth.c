#include <stdio.h>
#include <stdlib.h>
#include <dirent.h> // For directory operations
#include <string.h>
#include <stddef.h> // For NULL


//README
//*Had issues with compiling, edited cpp properties, not sure if I should have included in staged changes

//This program is designed to list all KML files in the current directory, 
//prompt the user to select a KML file, 
//and then prompt the user to choose between saving the file locally or opening it in Google Earth.

//The program will then save the file locally using a file explorer dialog,
//To open via Chrome, chrome must be installed
//To open via Google Earth Pro, Google Earth Pro must be installed

//No error checking is done for the file selection (typing), so make sure you type it in correctly >:[

// Function to list KML files in the current directory
void listKMLFiles() {
    struct dirent *entry;
    DIR *dp = opendir(".");

    if (dp == NULL) {
        perror("opendir");
        return;
    }

    printf("Available KML files:\n");
    while ((entry = readdir(dp))) {
        if (strstr(entry->d_name, ".kml")) {
            printf("%s\n", entry->d_name);
        }
    }

    closedir(dp);
}

// Function to prompt the user to select a KML file
void selectKMLFile(char *filename) {
    printf("Enter the name of the KML file to select: ");
    scanf("%s", filename);
}

// Function to prompt the user to choose between saving locally or opening in Google Earth
int chooseOption() {
    int choice;
    printf("Choose an option:\n");
    printf("1. Save locally\n");
    printf("2. Open in Google Earth (Chrome)\n");
    printf("3. Open in Google Earth (Downloaded, requires GoogleEarth to be downloaded)\n");
    printf("4. Open in Google Maps, saving link to GoogleMapsLinks.txt\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);
    return choice;
}

//DONE
// Function to save the file locally using a file explorer dialog
void saveLocally(const char *filename) {
    char command[256];
    snprintf(command, sizeof(command), "explorer.exe .");
    system(command);
}

//DONE
// Function to open the file in Google Earth via CHROME
void openInChromeGoogleEarth(const char *filename) {
    char command[512];
    //INCORRECT start chrome for windows; xdg-open for linux
    //xdg-open not installed by default in WSL?, use cmd.exe from WSL
    snprintf(command, sizeof(command), "cmd.exe /C start chrome \"https://earth.google.com/web?file=%s\"", filename);
    system(command);
}

//DONE
//Function to open the file in Google Earth via downloaded application Google Earth Pro
//Explanation: The file is .kml, Google Earth Pro will open automatically if you try to open a .kml file if Earth is installed
void openInDownloadedGoogleEarth(const char *filename) {
    char command[512];
    // Use PowerShell to locate and open Google Earth Pro dynamically
    snprintf(command, sizeof(command), "explorer.exe %s", filename);
    system(command);
}

//DONE
// Function to extract coordinates from KML file
int extractCoordinates(const char *filename, char *coordinates, size_t size) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("fopen");
        return 1;
    }

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        char *start = strstr(line, "<coordinates>");
        if (start) {
            start += strlen("<coordinates>");
            char *end = strstr(start, "</coordinates>");
            if (end) {
                size_t len = end - start;
                if (len < size) {
                    strncpy(coordinates, start, len);
                    coordinates[len] = '\0';

                    //remove trailing ",0" if present
                    char *end = strstr(coordinates, ",0");
                    if (end && end[2] == '\0') {
                        *end = '\0';
                    }

                    fclose(file);
                    return 0;
                }
            }
        }
    }

    fclose(file);
    return 1;
}

//DONE
// Function to open the coordinates in Google Maps and save the link locally

//Prototype Error: requirements for Longitude and Latitude ranges, if KML coors are outside range Maps will "not found" coords
//See "Format your Coordinates": https://support.google.com/maps/answer/18539?hl=en&co=GENIE.Platform%3DDesktop
void openInGoogleMaps(const char *filename) {
    char coordinates[1024];
    if (extractCoordinates(filename, coordinates, sizeof(coordinates)) == 0) {
        char command[2048];
        char url[2048];
        snprintf(url, sizeof(url), "https://www.google.com/maps?q=%s", coordinates);
        snprintf(command, sizeof(command), "cmd.exe /C start chrome \"%s\"", url);
        system(command);

        // Save the URL to GoogleMapsLinks.txt
        FILE *file = fopen("GoogleMapsLinks.txt", "a");
        if (file == NULL) {
            perror("fopen");
            return;
        }
        fprintf(file, "%s\n", url);
        fclose(file);
    } else {
        fprintf(stderr, "Failed to extract coordinates from KML file\n");
    }
}

int main() {
    char filename[256];

    listKMLFiles();
    selectKMLFile(filename);

    int choice = chooseOption();
    if (choice == 1) {
        saveLocally(filename);
    } else if (choice == 2) {
        openInChromeGoogleEarth(filename);
    } else if (choice == 3) {
        openInDownloadedGoogleEarth(filename);
    } else if (choice == 4) {
        openInGoogleMaps(filename);
    } else {
        printf("Invalid choice.\n");
    }

    return 0;
}