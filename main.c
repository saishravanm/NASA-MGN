#include <ncurses.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>



#define NUM_OPTIONS 3
#define NUM_FIELDS 4

//define custom struct for a button
typedef struct {
    int x, y, width, height;
} Button;

//define the labels for the different types of buttons (per screen)
const char* menu_options[NUM_OPTIONS] = {
    "Beacon Detection",
    "Historical Data Viewer",
    "Notifications and Alerts"
};

const char* notification_message[NUM_FIELDS] = {
    "Country Code",
    "Beacon Hex ID", 
    "Encoded Location", 
    "Time Recieved"
};

const char* back_text[1] = {
    "Back"
};

//define the coordinates for the different types of buttons (per screen)
Button main_menu_buttons[NUM_OPTIONS] = {
    {10, 4, 24, 3},
    {10, 8, 24, 3},
    {10, 12, 24, 3}
};
Button notification_buttons[NUM_FIELDS] = {
    {10, 4, 24, 3},
    {10, 8, 24, 3},
    {10, 12, 24, 3},
    {10, 14, 24, 3}
};

Button back_button[1] = {
    {10,16,24,3}
};

//function to draw a box and put a label in the center
void draw_box(int width, int y, int x, const char* label) {
    mvaddch(y - 1, x - 1, '+');
    mvhline(y - 1, x, '-', width);
    mvaddch(y - 1, x + width, '+');

    mvaddch(y, x - 1, '|');
    mvaddstr(y, x + (width - strlen(label)) / 2, label); // Center the label
    mvaddch(y, x + width, '|');

    mvaddch(y + 1, x - 1, '+');
    mvhline(y + 1, x, '-', width);
    mvaddch(y + 1, x + width, '+');
}

//check if a  button is clicked
bool isClick(Button button, int mouse_x, int mouse_y) {
return (mouse_x >= button.x && mouse_x <= button.x + button.width &&
            mouse_y >= button.y && mouse_y <= button.y + button.height);
}

//draw a button 
void draw_button(int y, int x, const char* label, bool is_selected) {
    int width = 24;

    if (is_selected) {
        attron(A_REVERSE);
    }

    draw_box(width, y, x, label);

    if (is_selected) {
        attroff(A_REVERSE);
    }
}


//display the main menu
void display_mm(Button* buttons) {
    for (int i = 0; i < NUM_OPTIONS; i++) {
        draw_button(buttons[i].y, buttons[i].x, menu_options[i], false);
    }
}

//check for button presses in the main menu
int check_mm_press(MEVENT event, Button* buttons) {
    
        if (isClick(buttons[0], event.x, event.y)) {
            return 1; // Return the index of the clicked button
        }
        else if (isClick(buttons[1], event.x, event.y)) {
            return 2; // Return the index of the clicked button
        }
        else if (isClick(buttons[2], event.x, event.y)) {
            return 3; // Return the index of the clicked button
        }
    
    return -1;  // Return -1 if no button was clicked
}

//check if the back button is ever pressed
int check_back_press(MEVENT event, Button* buttons){
    if(isClick(buttons[0], event.x, event.y)){
        return 1;
    }
    return 0; 
}

void play_sound(const char *sound_file){
    char command[256];
    snprintf(command, sizeof(command), "aplay %s", sound_file);

    int result = system(command);

    if(result != 0){
        printw("Failed top lay sound: %s\n", sound_file);
    }
    printw("played sound!");

}
//send an alert with sound if a signal is found 

//send short data burst if a beacon is detected 
void short_data_burst(char* countryCode, int hexID, int encodedLocation, time_t timeReceived){
    
    
        //print Country Code
            draw_button(notification_buttons[0].y,notification_buttons[0].x, notification_message[0] ,true);
            mvprintw(notification_buttons[0].y, notification_buttons[0].x+15, ": %s", countryCode);

        // print Hex ID
            draw_button(notification_buttons[1].y,notification_buttons[1].x, notification_message[1] ,true);
            mvprintw(notification_buttons[1].y, notification_buttons[1].x+12, ": %d", hexID);

            //Encoded Location
            draw_button(notification_buttons[2].y,notification_buttons[2].x, notification_message[2] ,true);
            mvprintw(notification_buttons[2].y, notification_buttons[2].x+12, ": %d", encodedLocation);
        
            //Time Recieved 
            draw_button(notification_buttons[3].y,notification_buttons[3].x, notification_message[3] ,true);

            struct tm* time_info = localtime(&timeReceived);
            char time_str[128];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", time_info);
            mvprintw(notification_buttons[3].y, notification_buttons[3].x+19, ": %s", time_str);

            const char* dir_path = "./notifications/";
            struct stat st = {0};
            if(stat(dir_path, &st) == -1){
                if(mkdir(dir_path, 0700) != 0){
                    perror("unable to create directory");
                    return;
                }
            }



            char file_path[256];
            snprintf(file_path, sizeof(file_path), "%s%s", dir_path, time_str);

            FILE *file = fopen(file_path, "a");
            if(file == NULL){
                perror("Unable to open file");
                return;
            }
            fprintf(file, "%s: %s\n", notification_message[0], countryCode);
            fprintf(file, "%s: %d\n", notification_message[1], hexID);
            fprintf(file, "%s: %d\n", notification_message[2], encodedLocation);
            fprintf(file, "%s: %s\n", notification_message[3], time_str);

            fclose(file);

            refresh();
    
    
    draw_button(back_button[0].y,back_button[0].x, back_text[0] ,false);

}

//search for a beacon
void beacon_search(){

}

void list_files(const char *path){
    struct dirent *entry; //holds directory
    struct stat file_stat; //structure to hold file information 
    
    DIR *dp = opendir(path); 

    if(dp == NULL){
        perror("opendir");
        printw("Error opening directory: %s\n", path);
        return;
    }
        printw(" opening directory: %s\n", path);
    
    while((entry = readdir(dp)) != NULL){
        if(entry->d_name[0] == '.'){
            continue;
        }

        char full_path[1024];
        if (path[strlen(path) - 1] == '/') {
            snprintf(full_path, sizeof(full_path), "%s%s", path, entry->d_name);  // Path already ends with '/'
        } else {
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);  // Append '/'
        }
        if(stat(full_path, &file_stat) == -1){
            printw(" error retrieving file info: %s\n", entry->d_name);
            perror("stat");
            continue;
        }

        char timebuf[80];
        struct tm *tm_info = localtime(&file_stat.st_mtime);
        strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tm_info);
        printw("File: %-30s | Modified: %s\n", entry->d_name, timebuf);
    }

    closedir(dp);
    

    refresh();
    draw_button(back_button[0].y,back_button[0].x, back_text[0] ,false);


}

void send_alert(){
    time_t current_time = time(NULL);
    short_data_burst("US", 123456, 789012, current_time);
    //play_sound("./sound_files/sarsat_alert_sound.wav");
}
int main() {
    MEVENT event;
    int ch;

    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS, NULL);
    curs_set(1);
    
    //Manage which screen we're on
    int current_screen = 0;
    // Main loop
    while (1) {

        //main menu if current_screen = 0
        if(current_screen == 0){
        clear();
        display_mm(main_menu_buttons);
        }
        ch = getch();
        //check which menu option is pressed
        if (ch == KEY_MOUSE && getmouse(&event) == OK) { 
            if (event.bstate & BUTTON1_RELEASED) {

                //check whether the main menu buttons/back button is pressed
                int clicked_button = check_mm_press(event, main_menu_buttons);
                int clicked_back = check_back_press(event, back_button);

                //send back to home screen if back button is pressed
                if(clicked_back == 1){
                        refresh();
                        current_screen = 0;
                        continue;
                    }
                current_screen = clicked_button;

                //beacon detection screen
                if (current_screen == 1) {
                    clear();  // Clear the screen
                    mvprintw(0, 0, "You clicked 'Beacon Detection'!");
                    send_alert();
                    refresh();
                }
                
                //kml viewer screen
                if (current_screen == 2) {
                    clear();
                    //mvprintw(0, 0, "You clicked 'Historical Data Viewer'!");
                    list_files("./kml_files");
                    refresh();
                }
                
                //view past notifications/alerts? idk
                if (current_screen == 3) {
                    clear();
                    //mvprintw(0, 0, "You clicked 'Notifications and Alerts'!");
                    list_files("./notifications");

                    refresh();
                }
            }
        }
    }

    endwin();  // End ncurses mode properly
    return 0;
}
