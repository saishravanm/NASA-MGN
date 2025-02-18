#ifndef UI_H
#define UI_H

#include <stdbool.h>
#include <time.h>
#include <curses.h>

#define NUM_OPTIONS 3
#define NUM_FIELDS 4

extern const char* menu_options[NUM_OPTIONS];
extern const char* sdb_text[NUM_FIELDS];
extern const char* notification_text[1];
extern const char* back_text[1];

typedef struct {
    int x, y, width, height;
} Button;

extern Button main_menu_buttons[NUM_OPTIONS];
extern Button notification_buttons[NUM_FIELDS];
extern Button back_button[1];

void ui_init(void);
void ui_cleanup(void);

void draw_box(int width, int y, int x, const char* label);
void draw_button(int y, int x, const char* label, bool is_selected);
bool isClick(Button button, int mouse_x, int mouse_y);

void display_mm(Button* buttons);
int check_mm_press(MEVENT event, Button* buttons);
int check_back_press(MEVENT event, Button* buttons);
int is_notif_pressed(MEVENT event, Button *button);
void notification(char* frequency,int duration_seconds, int flash_count, char* sound_file, COUNTRY_CODE *countryCode, IDENTIFICATION *id, COORD *coords, time_t timeReceived);

#endif
