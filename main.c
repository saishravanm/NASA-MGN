#include <ncurses.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include "kml_generation.c"
#include "data.h"

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
int is_notif_pressed(MEVENT event, Button *button){
    if(isClick(button[0], event.x, event.y)){
        return 1;
    }
    return 0;
}

//play sound given sound file
void play_sound(const char *sound_file){
    char command[256];
    snprintf(command, sizeof(command), "aplay %s", sound_file);

    int result = system(command);

    if(result != 0){
        printw("Failed to play sound: %s\n", sound_file);
    }
    printw("played sound!");

}

//send short data burst
void short_data_burst(COUNTRY_CODE *countryCode, IDENTIFICATION *id, COORD *coords, time_t timeReceived){
    
    
        //print Country Code
            draw_button(notification_buttons[0].y,notification_buttons[0].x, sdb_text[0] ,true);
            mvprintw(notification_buttons[0].y, notification_buttons[0].x+15, "%s-%d", countryCode->code, countryCode->digits);

        // print Hex ID
            draw_button(notification_buttons[1].y,notification_buttons[1].x, sdb_text[1] ,true);
	    switch(id->type) {
	   	
		case MMSI_BNO:
            		mvprintw(notification_buttons[1].y, notification_buttons[1].x+12, "%s: %d-%d",
				       	"Maritime Mobile Service Identity (Last 6 Digits)-BNO",
				       	id->data.mmsi_bno.mmsi, id->data.mmsi_bno.bno);
			break;
		case AIRCRAFT_ADDR:
        	    	mvprintw(notification_buttons[1].y, notification_buttons[1].x+12, "%s: %d", 
					"Aircraft 24-bit Address", 
					id->data.air_addr.air_addr);
			break;
		case AIRCRAFT_OP:
        	    	mvprintw(notification_buttons[1].y, notification_buttons[1].x+12, "%s: %d-%d",
				       	"Aircraft OPER Designator-Serial No", 
					id->data.air_op.air_oper, id->data.air_op.serial_no);
			break;
		case ELT_SERIAL:
        	    	mvprintw(notification_buttons[1].y, notification_buttons[1].x+12, "%s: %d-%d",
				       	"C/S TA No [ELT Serial]",
					id->data.csta.csta_no, id->data.csta.serial_no);
			break;
		case EPIRB_SERIAL:
        	    	mvprintw(notification_buttons[1].y, notification_buttons[1].x+12, "%s: %d-%d",
				       	"C/S TA No [EPIRB_SERIAL]",
					id->data.csta.csta_no, id->data.csta.serial_no);
			break;
		case PLB:	
        	    	mvprintw(notification_buttons[1].y, notification_buttons[1].x+12, "%s: %d-%d",
				       	"C/S TA No [PLB]",
					id->data.csta.csta_no, id->data.csta.serial_no);
			break;
		case MMSI_FIXED:
	            	mvprintw(notification_buttons[1].y, notification_buttons[1].x+12, "%s: %d",
				       	"Maritime Mobile Service Identity (Last 6 Digits) [FIXED]",
					id->data.mmsi_bno.mmsi);
			break;
		case TEST:
 	           	mvprintw(notification_buttons[1].y, notification_buttons[1].x+12, "TESTING...");
			break;
		default:	
            		mvprintw(notification_buttons[1].y, notification_buttons[1].x+12, "UNKNOWN FORMAT");


	   	
	    }
            //Encoded Location
            draw_button(notification_buttons[2].y,notification_buttons[2].x, sdb_text[2] ,true);
            mvprintw(notification_buttons[2].y, notification_buttons[2].x+12, "%c-%.2f:%c-%.2f [LAT:%02d:%02d-LONG:%02d:%02d]", coords->ns, coords->lat_deg,
			   										 coords->ew, coords->long_deg,
													 coords->lat_delta_min, coords->lat_delta_sec,
													 coords->long_delta_min, coords->long_delta_sec);

            //Time Recieved 
            draw_button(notification_buttons[3].y,notification_buttons[3].x, sdb_text[3] ,true);

            struct tm* time_info = localtime(&timeReceived);
            char time_str[128];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", time_info);
            mvprintw(notification_buttons[3].y, notification_buttons[3].x+19, "%s", time_str);

            const char* dir_path = "./notifications/";
            struct stat st = {0};
            if(stat(dir_path, &st) == -1){
                if(mkdir(dir_path, 0700) != 0){
                    perror("unable to create directory");
                    return;
                }
            }


            //write data burst to file

            char file_path[256];
            snprintf(file_path, sizeof(file_path), "%s%s", dir_path, time_str);

            FILE *file = fopen(file_path, "a");
            if(file == NULL){
                perror("Unable to open file");
                return;
            }
            fprintf(file, "%s: %s-%d\n", sdb_text[0], countryCode->code, countryCode->digits);
            fprintf(file, "%s: % %c-%.2f:%c-%.2f [LAT:%02d:%02d-LONG:%02d:%02d]\n", sdb_text[1], coords->ns, coords->lat_deg,
			   								   coords->ew, coords->long_deg,
											   coords->lat_delta_min, coords->lat_delta_sec,
											   coords->long_delta_min, coords->long_delta_sec);
	    
	    

	    switch(id->type) {
	   	
		case MMSI_BNO:
            		fprintf(file, "%s: [s] %d-$d\n", sdb_text[2],
				       	"Maritime Mobile Service Identity (Last 6 Digits)-BNO",
				       	id->data.mmsi_bno.mmsi, id->data.mmsi_bno.bno);
			break;
		case AIRCRAFT_ADDR:
        	    	fprintf(file, "%s: [s] %d\n", sdb_text[2], 
					"Aircraft 24-bit Address", 
					id->data.air_addr.air_addr);
			break;
		case AIRCRAFT_OP:
        	    	fprintf(file, "%s: [s] %d-$d\n", sdb_text[2],
				       	"Aircraft OPER Designator-Serial No", 
					id->data.air_op.air_oper, id->data.air_op.serial_no);
			break;
		case ELT_SERIAL:
        	    	fprintf(file, "%s: [s] %d-$d\n", sdb_text[2],
				       	"C/S TA No [ELT Serial]",
					id->data.csta.csta_no, id->data.csta.serial_no);
			break;
		case EPIRB_SERIAL:
        	    	fprintf(file, "%s: [s] %d-$d\n", sdb_text[2],
				       	"C/S TA No [EPIRB_SERIAL]",
					id->data.csta.csta_no, id->data.csta.serial_no);
			break;
		case PLB:	
        	    	fprintf(file, "%s: [s] %d-$d\n", sdb_text[2],
				       	"C/S TA No [PLB]",
					id->data.csta.csta_no, id->data.csta.serial_no);
			break;
		case MMSI_FIXED:
	            	fprintf(file, "%s: [s] %d\n", sdb_text[2],
				       	"Maritime Mobile Service Identity (Last 6 Digits) [FIXED]",
					id->data.mmsi_bno.mmsi);
			break;
		case TEST:
 	           	fprintf(file, "%s: [s]\n", sdb_text[2], "TESTING...");
			break;
		default:	
            		fprintf(file, "%s: [s]\n", sdb_text[2], "UNKNOWN FORMAT");


	   	
	    }

            fprintf(file, "%s: %s\n", sdb_text[3], time_str);

            fclose(file);

            refresh();
    
    
    draw_button(back_button[0].y,back_button[0].x, back_text[0] ,false);

}

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




//display notification
void notification(char* frequency,int duration_seconds, int flash_count, char* sound_file, COUNTRY_CODE *countryCode, IDENTIFICATION *id, COORD *coords, time_t timeReceived){
    int delay_ms = duration_seconds * 1000 / flash_count;
    int h = true;
    
    char freq_label[1024]; 
    snprintf(freq_label, sizeof(freq_label), "%s mHz beacon found", frequency);

    char sf[1024];
    //snprintf(sf, sizeof(sf),"./sound_files/%s.wav",sound_file);

    play_sound(sf);
    for(int i = 0; i < flash_count; i++){
        draw_button(8, 24, freq_label, h);
        refresh(); 
        napms(delay_ms);
        h = !h;
    }
    clear();
    if(strcmp(frequency,"406.025") == 0){
        short_data_burst(countryCode, id, coords, timeReceived);
    }
}


//search for a beacon
void beacon_search(){
    //call mihir's search functions -> if 406.025 then return hex packet, if 121.65 then return True or False
    //if we find a 406.025
 	DATA data; //Initialize a data object
	/* Mihir code
   		The two line of code below is what you will have to edit, replace the "Test buffer" to be filled with your signal data and if the name
		is different, just change it in the data_memcpy argument. Lastly, you will need to implement the IF/ELSE statement below. 	
 	*/	
	
	char buffer[18]={0xff, 0xfe, 0x2f, 0x97, 0x0e, 0x00, 0x80, 0x01, 0x27, 0x29, 0x9b, 0x1e, 0x21, 0xf6, 0x00, 0x65, 0x79, 0x69 }; //Test buffer
	data_memcpy(&data, buffer);

	//This is just creating seperate objects which handle a readable format for printing data
	COORD coord=read_coordinates(&data);
	COUNTRY_CODE cc=read_country_code(&data);
	IDENTIFICATION id=read_identification(&data);
	time_t current_time=time(NULL); //Grabbing the current time one the system
	
    //call hex_decode on hex packet ?????? I don't think we need this anymore because you have access to all the data through the properties of the DATA struct
        //hex_decode function 1 -> should return the country code, beacon hex id, encoded location, and time recieved 
        //hex_decode function 2 -> should return the decoded latitude, longitude, country code, and beacon id, and timestamp
    
    //if 406.025 found (example) // IF/ELSE Implemented by Mihir since I do not know how you are able to detect the type of signal
    	notification("406.025",2,20,"sarsat_alert_sound", &cc, &id, &coord, current_time);
    //else if 121.65 found
    //notification("121.65",2,20,"sarsat_alert_sound");

    //then we call Taaha's KML generate function using hex_decode function 2

}


//LIST FILES IN A GIVEN DIRECTORY
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
