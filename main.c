#include <ncurses.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>



#define NUM_OPTIONS 3
#define NUM_FIELDS 4
typedef struct {
    int x, y, width, height;
} Button;

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


void list_kml(const char *path){
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
}
bool chosen_options[NUM_OPTIONS] = {
    false,
    false,
    false
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

bool isClick(Button button, int mouse_x, int mouse_y) {
return (mouse_x >= button.x && mouse_x <= button.x + button.width &&
            mouse_y >= button.y && mouse_y <= button.y + button.height);
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

void display_mm(Button* buttons) {
    for (int i = 0; i < NUM_OPTIONS; i++) {
        draw_button(buttons[i].y, buttons[i].x, menu_options[i], false);
    }
}

int check_mm_press(MEVENT event, Button* buttons) {
    
        if (isClick(buttons[0], event.x, event.y)) {
            return 0; // Return the index of the clicked button
        }
        else if (isClick(buttons[1], event.x, event.y)) {
            return 1; // Return the index of the clicked button
        }
        else if (isClick(buttons[2], event.x, event.y)) {
            return 2; // Return the index of the clicked button
        }
    
    return -1;  // Return -1 if no button was clicked
}
void send_alert(){
    for(int i = 0; i < NUM_FIELDS; i++)
    {
        draw_button(notification_buttons[i].y,notification_buttons[i].x, notification_message[i] ,true);
        refresh();
    }
    
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
    
    // Draw the menu initially
    display_mm(main_menu_buttons);
    refresh();

    // Main loop
    while (1) {
        ch = getch();
        if (ch == KEY_MOUSE && getmouse(&event) == OK) {
            if (event.bstate & BUTTON1_RELEASED) {
                int clicked_button = check_mm_press(event, main_menu_buttons);
                
                if (clicked_button == 0) {
                    clear();  // Clear the screen
                    mvprintw(0, 0, "You clicked 'Beacon Detection'!");
                    send_alert();
                    refresh();
                }if (clicked_button == 1) {
                    clear();
                    //mvprintw(0, 0, "You clicked 'Historical Data Viewer'!");
                    list_kml("./kml_files");
                    refresh();
                }if (clicked_button == 2) {
                    clear();
                    mvprintw(0, 0, "You clicked 'Notifications and Alerts'!");
                    refresh();
                }
            }
        }
    }

    endwin();  // End ncurses mode properly
    return 0;
}
