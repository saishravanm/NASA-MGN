#include "ui.h"
#include <ncurses.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "decode.h"
#include "data.h"
#include "util.h"

const char* menu_options[NUM_OPTIONS] = {
    "Beacon Detection",
    "Historical Data Viewer",
    "Notifications and Alerts"
};

const char* sdb_text[NUM_FIELDS] = {
    "Country Code",
    "Beacon Hex ID",
    "Encoded Location",
    "Time Recieved"
};

const char* notification_text[1] = {
    "Beacon Found!"
};

const char* back_text[1] = {
    "Back"
};

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



void ui_init(void){
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS, NULL);
}

void ui_cleanup(void){
    endwin();
}

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

bool isClick(Button button, int mouse_x, int mouse_y) {
return (mouse_x >= button.x && mouse_x <= button.x + button.width &&
            mouse_y >= button.y && mouse_y <= button.y + button.height);
}

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
int is_notif_pressed(MEVENT event, Button *button){
    if(isClick(button[0], event.x, event.y)){
        return 1;
    }
    return 0;
}

//display notification
void notification(char* frequency,int duration_seconds, int flash_count, char* sound_file, COUNTRY_CODE *countryCode, IDENTIFICATION *id, COORD *coords, time_t timeReceived){
    int delay_ms = duration_seconds * 1000 / flash_count;
    int h = true;
    
    char freq_label[1024]; 
    snprintf(freq_label, sizeof(freq_label), "%s mHz beacon found", frequency);

    char sf[1024];
    //snprintf(sf, sizeof(sf),"./sound_files/%s.wav",sound_file); 

    pthread_t sound_tid;
    pthread_create(&sound_tid, NULL,sound_thread, NULL);

    //play_sound(sf);
    for(int i = 0; i < flash_count; i++){
        draw_button(8, 24, freq_label, h);
        refresh(); 
        napms(delay_ms);
        h = !h;
    }
    pthread_join(sound_tid, NULL);
    clear();
    if(strcmp(frequency,"406.025") == 0){
        short_data_burst(countryCode, id, coords, timeReceived);
    }
}
