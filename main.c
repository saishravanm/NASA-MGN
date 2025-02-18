#include <stdio.h>
#include <ncurses.h>
#include <time.h>
#include <pthread.h>
#include "ui.h"
#include "signal_process.h"
#include "decode.h"
#include "kml_generation.h"
#include "util.h"



//TEST FUNCTION TO MAKE SURE EVERYTHING WORKS
/*
void send_data_burst(){
    time_t current_time = time(NULL);

    //Generate data burst display
    short_data_burst("US", 123456, 789012, current_time);

    //convert current_time to string for kml file name
    struct tm* time_info = localtime(&current_time);
    char time_str[128];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S.kml", time_info);
    
    //generate KML
    generate_kml(time_str,32.7767,-96.7970,"US","Beacon12345",current_time);
    printw("KML Generated!");

}
*/


//search for a beacon

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
                    //mvprintw(0, 0, "You clicked 'Beacon Detection'!");
                    beacon_search();
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
