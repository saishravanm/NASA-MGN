#include <stdio.h>
#include <stdlib.h>
#include <dirent.h> // For directory operations
#include <string.h>
#include <stddef.h> // For NULL


//README
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

//
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
    } else {
        printf("Invalid choice.\n");
    }

    return 0;
}