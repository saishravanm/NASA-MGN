#include <ncurses.h> 
#include <string.h> 

#define NUM_OPTIONS 3
const char* menu_options[NUM_OPTIONS] = {
    "Beacon Detection",
    "Historical Data Viewer", 
    "Notifications and Alerts"
};

typedef struct {
    int x, y, width, height;
} Button;


void draw_box(int width, int y, int x, const char* label){
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

bool isClick(Button button, int mouse_x, int mouse_y){
    if(mouse_x >= button.x && mouse_x <= button.x + button.width && mouse_y >= button.y && mouse_y <= button.y + button.height){
        return true;
    }
    return false;
}

void draw_button(int y, int x, const char* label, bool is_selected){
    int width = 24; 

    if(is_selected == true){
        attron(A_REVERSE);
    }

    draw_box(width,y, x, label);

    if(!is_selected == false){
        attroff(A_REVERSE);
    }

}
int main() {
    Button buttons[NUM_OPTIONS] = {
        {10, 4, 24, 3},
        {10, 8, 24, 3},
        {10, 12, 24, 3},

    };
    MEVENT event;
    int highlight = 0;
    int ch;

    initscr();
    noecho();
    cbreak();
    keypad(stdscr, true);
    mousemask(ALL_MOUSE_EVENTS, NULL);
    curs_set(1);

    // Draw all buttons initially
    for (int i = 0; i < NUM_OPTIONS; i++) {
        draw_button(buttons[i].y, buttons[i].x, menu_options[i], i == highlight);
    }

    refresh();

    // Main loop
    while (1) {
        ch = getch();
        switch (ch) {
            case KEY_MOUSE:
                if (getmouse(&event) == OK) {
                    if (event.bstate & BUTTON1_CLICKED) {
                        for (int i = 0; i < NUM_OPTIONS; i++) {
                            if (isClick(buttons[i], event.x, event.y)) {
                                // Unhighlight prev
                                draw_button(buttons[highlight].y, buttons[highlight].x, menu_options[highlight], false);

                                // Highlight new
                                highlight = i;
                                draw_button(buttons[highlight].y, buttons[highlight].x, menu_options[highlight], true);
                                refresh();  // Update the screen

                                break;
                            }
                        }
                    }
                }
                break;
        }
    }

    endwin();  // End ncurses mode properly
    return 0;
}
