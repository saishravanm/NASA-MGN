#include <ncurses.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include "kml_generation.c"
#include "data.h"
#include <iio.h>
#include <stdbool.h>
#include <math.h>

#define NUM_OPTIONS 3
#define NUM_FIELDS 4
#define TARGET_FREQUENCY 2121650000ULL  // Frequency to check (121.65 MHz)
#define SAMPLE_COUNT 1024              // Number of samples to capture
#define SIGNAL_DETECTION_THRESHOLD 500 // Threshold for signal detection
#define SARSAT_FREQUENCY 406025000ULL  // 406.025 MHz
#define OUTPUT_SIZE 18                 // Output size for char[18]


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
		case AIRCRAFT_ADDR:
        	    	mvprintw(notification_buttons[1].y, notification_buttons[1].x+12, "%s: %d", 
					"Aircraft 24-bit Address", 
					id->data.air_addr.air_addr);
		case AIRCRAFT_OP:
        	    	mvprintw(notification_buttons[1].y, notification_buttons[1].x+12, "%s: %d-%d",
				       	"Aircraft OPER Designator-Serial No", 
					id->data.air_op.air_oper, id->data.air_op.serial_no);
		case ELT_SERIAL:
        	    	mvprintw(notification_buttons[1].y, notification_buttons[1].x+12, "%s: %d-%d",
				       	"C/S TA No [ELT Serial]",
					id->data.csta.csta_no, id->data.csta.serial_no);
		case EPIRB_SERIAL:
        	    	mvprintw(notification_buttons[1].y, notification_buttons[1].x+12, "%s: %d-%d",
				       	"C/S TA No [EPIRB_SERIAL]",
					id->data.csta.csta_no, id->data.csta.serial_no);
		case PLB:	
        	    	mvprintw(notification_buttons[1].y, notification_buttons[1].x+12, "%s: %d-%d",
				       	"C/S TA No [PLB]",
					id->data.csta.csta_no, id->data.csta.serial_no);
		case MMSI_FIXED:
	            	mvprintw(notification_buttons[1].y, notification_buttons[1].x+12, "%s: %d",
				       	"Maritime Mobile Service Identity (Last 6 Digits) [FIXED]",
					id->data.mmsi_bno.mmsi);
		case TEST:
 	           	mvprintw(notification_buttons[1].y, notification_buttons[1].x+12, "TESTING...");
		default:	
            		mvprintw(notification_buttons[1].y, notification_buttons[1].x+12, "UNKNOWN FORMAT");


	   	
	    }
            //Encoded Location
            draw_button(notification_buttons[2].y,notification_buttons[2].x, sdb_text[2] ,true);
            mvprintw(notification_buttons[2].y, notification_buttons[2].x+12, "%c-%f:%c-%f [LAT:%02d:%02d-LONG:%02d:%02d]", coords->ns, coords->lat_deg,
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
            fprintf(file, "%s: % %c-%f:%c-%f [LAT:%02d:%02d-LONG:%02d:%02d]\n", sdb_text[1], coords->ns, coords->lat_deg,
			   								   coords->ew, coords->long_deg,
											   coords->lat_delta_min, coords->lat_delta_sec,
											   coords->long_delta_min, coords->long_delta_sec);
            fprintf(file, "%s: %d\n", sdb_text[2], hexID);
	    switch(id->type) {
	   	
		case MMSI_BNO:
            		fprintf(notification_buttons[1].y, notification_buttons[1].x+12, "%s: %d-%d",
				       	"Maritime Mobile Service Identity (Last 6 Digits)-BNO",
				       	id->data.mmsi_bno.mmsi, id->data.mmsi_bno.bno);
		case AIRCRAFT_ADDR:
        	    	fprintf(notification_buttons[1].y, notification_buttons[1].x+12, "%s: %d", 
					"Aircraft 24-bit Address", 
					id->data.air_addr.air_addr);
		case AIRCRAFT_OP:
        	    	fprintf(notification_buttons[1].y, notification_buttons[1].x+12, "%s: %d-%d",
				       	"Aircraft OPER Designator-Serial No", 
					id->data.air_op.air_oper, id->data.air_op.serial_no);
		case ELT_SERIAL:
        	    	fprintf(notification_buttons[1].y, notification_buttons[1].x+12, "%s: %d-%d",
				       	"C/S TA No [ELT Serial]",
					id->data.csta.csta_no, id->data.csta.serial_no);
		case EPIRB_SERIAL:
        	    	fprintf(notification_buttons[1].y, notification_buttons[1].x+12, "%s: %d-%d",
				       	"C/S TA No [EPIRB_SERIAL]",
					id->data.csta.csta_no, id->data.csta.serial_no);
		case PLB:	
        	    	fprintf(notification_buttons[1].y, notification_buttons[1].x+12, "%s: %d-%d",
				       	"C/S TA No [PLB]",
					id->data.csta.csta_no, id->data.csta.serial_no);
		case MMSI_FIXED:
	            	fprintf(notification_buttons[1].y, notification_buttons[1].x+12, "%s: %d",
				       	"Maritime Mobile Service Identity (Last 6 Digits) [FIXED]",
					id->data.mmsi_bno.mmsi);
		case TEST:
 	           	fprintf(notification_buttons[1].y, notification_buttons[1].x+12, "TESTING...");
		default:	
            		fprintf(notification_buttons[1].y, notification_buttons[1].x+12, "UNKNOWN FORMAT");


	   	
	    }

            fprintf(file, "%s: %s\n", sdb_text[3], time_str);

            fclose(file);

            refresh();
    
    
    draw_button(back_button[0].y,back_button[0].x, back_text[0] ,false);

}

//TEST FUNCTION TO MAKE SURE EVERYTHING WORKS
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
int check_frequency(struct iio_context *context, uint64_t frequency) {
    struct iio_device *receiver_device, *phy_device;
    struct iio_channel *i_channel, *q_channel, *lo_channel;
    struct iio_buffer *sample_buffer;
    ssize_t buffer_bytes;
    char *data_pointer, *data_end;
    ptrdiff_t step_size;

    // Find the RX device
    receiver_device = iio_context_find_device(context, "cf-ad9361-lpc");
    if (!receiver_device) {
        fprintf(stderr, "Unable to find RX device\n");
        return -1;  // Error finding RX device
    }

    // Find the PHY device to configure hardware settings
    phy_device = iio_context_find_device(context, "ad9361-phy");
    if (!phy_device) {
        fprintf(stderr, "Unable to find PHY device\n");
        return -2;  // Error finding PHY device
    }

    // Set the LO frequency to the desired frequency
    lo_channel = iio_device_find_channel(phy_device, "altvoltage0", true);
    if (!lo_channel) {
        fprintf(stderr, "Unable to find LO channel\n");
        return -3;  // Error finding LO channel
    }

    if (iio_channel_attr_write_longlong(lo_channel, "frequency", frequency) < 0) {
        fprintf(stderr, "Failed to set LO frequency to %llu Hz\n", frequency);
        return -4;  // Error setting LO frequency
    }

    // Enable the RX voltage channels for I (voltage0) and Q (voltage1)
    i_channel = iio_device_find_channel(receiver_device, "voltage0", false);
    q_channel = iio_device_find_channel(receiver_device, "voltage1", false);
    if (!i_channel || !q_channel) {
        fprintf(stderr, "Unable to find RX channels\n");
        return -5;  // Error finding RX channels
    }
    iio_channel_enable(i_channel);
    iio_channel_enable(q_channel);

    // Create a buffer to receive samples from the RX device
    sample_buffer = iio_device_create_buffer(receiver_device, SAMPLE_COUNT, false);
    if (!sample_buffer) {
        fprintf(stderr, "Unable to create RX buffer\n");
        return -6;  // Error creating buffer
    }

    // Fill the buffer with received samples
    buffer_bytes = iio_buffer_refill(sample_buffer);
    if (buffer_bytes < 0) {
        fprintf(stderr, "Error refilling RX buffer: %zd\n", buffer_bytes);
        iio_buffer_destroy(sample_buffer);
        return -7;  // Error refilling buffer
    }

    // Process the samples and calculate the magnitude
    step_size = iio_buffer_step(sample_buffer);
    data_end = iio_buffer_end(sample_buffer);
    double total_magnitude = 0;
    int16_t i_sample_value, q_sample_value;
    for (data_pointer = iio_buffer_first(sample_buffer, i_channel); data_pointer < data_end; data_pointer += step_size) {
        int16_t *sample_pointer = (int16_t*)data_pointer;
        i_sample_value = sample_pointer[0];  // I sample
        q_sample_value = sample_pointer[1];  // Q sample
        double sample_magnitude = sqrt((double)i_sample_value * i_sample_value + (double)q_sample_value * q_sample_value);
        total_magnitude += sample_magnitude;
    }

    // Calculate the average magnitude
    double average_magnitude = total_magnitude / SAMPLE_COUNT;
    printf("Average magnitude: %f\n", average_magnitude);

    // Determine if signal is detected based on the threshold
    if (average_magnitude > SIGNAL_DETECTION_THRESHOLD) {
        iio_buffer_destroy(sample_buffer);
        return 1;  // Signal detected
    }

    // Clean up
    iio_buffer_destroy(sample_buffer);
    return 0;  // No signal detected
}
char* detect_sarsat_signal(struct iio_context *context, uint64_t frequency) {
    struct iio_device *receiver_device, *phy_device;
    struct iio_channel *i_channel, *q_channel, *lo_channel;
    struct iio_buffer *sample_buffer;
    ssize_t buffer_bytes;
    static char output[OUTPUT_SIZE];

    // Find the RX device
    receiver_device = iio_context_find_device(context, "cf-ad9361-lpc");
    if (!receiver_device) {
        fprintf(stderr, "Unable to find RX device\n");
        strncpy(output, "Error: RX device", OUTPUT_SIZE);
        return output;
    }

    // Find the PHY device to configure hardware settings
    phy_device = iio_context_find_device(context, "ad9361-phy");
    if (!phy_device) {
        fprintf(stderr, "Unable to find PHY device\n");
        strncpy(output, "Error: PHY device", OUTPUT_SIZE);
        return output;
    }

    // Set the LO frequency to the desired SARSAT frequency
    lo_channel = iio_device_find_channel(phy_device, "altvoltage0", true);
    if (!lo_channel) {
        fprintf(stderr, "Unable to find LO channel\n");
        strncpy(output, "Error: LO channel", OUTPUT_SIZE);
        return output;
    }

    if (iio_channel_attr_write_longlong(lo_channel, "frequency", frequency) < 0) {
        fprintf(stderr, "Failed to set LO frequency to %llu Hz\n", frequency);
        strncpy(output, "Error: Set LO freq", OUTPUT_SIZE);
        return output;
    }

    // Enable the RX voltage channels for I (voltage0) and Q (voltage1)
    i_channel = iio_device_find_channel(receiver_device, "voltage0", false);
    q_channel = iio_device_find_channel(receiver_device, "voltage1", false);
    if (!i_channel || !q_channel) {
        fprintf(stderr, "Unable to find RX channels\n");
        strncpy(output, "Error: RX channels", OUTPUT_SIZE);
        return output;
    }
    iio_channel_enable(i_channel);
    iio_channel_enable(q_channel);

    // Create a buffer to receive samples from the RX device
    sample_buffer = iio_device_create_buffer(receiver_device, SAMPLE_COUNT, false);
    if (!sample_buffer) {
        fprintf(stderr, "Unable to create RX buffer\n");
        strncpy(output, "Error: Create buffer", OUTPUT_SIZE);
        return output;
    }

    // Fill the buffer with received samples
    buffer_bytes = iio_buffer_refill(sample_buffer);
    if (buffer_bytes < 0) {
        fprintf(stderr, "Error refilling RX buffer: %zd\n", buffer_bytes);
        iio_buffer_destroy(sample_buffer);
        strncpy(output, "Error: Refill buffer", OUTPUT_SIZE);
        return output;
    }

    // Check if buffer is empty (no valid data)
    if (buffer_bytes == 0) {
        fprintf(stderr, "Empty buffer received\n");
        iio_buffer_destroy(sample_buffer);
        strncpy(output, "Error: Empty buffer", OUTPUT_SIZE);
        return output;
    }

    // Example: Return raw signal data as "RAW_SIGNAL_XXXX" placeholder
    strncpy(output, "RAW_SIGNAL_DATA", OUTPUT_SIZE);  // Replace with actual signal data

    // Clean up
    iio_buffer_destroy(sample_buffer);
    return output;
}

//search for a beacon
//to prevent any performance issues, the program only searches for each beacon once, the final plan is to have them continuously searching for the beacon and Sarsat in a thread
void beacon_search(){
   struct iio_context *context;

    // Initialize the IIO context using USB connection
    context = iio_create_context_from_uri("usb:");
    if (!context) {
        fprintf(stderr, "Unable to create IIO context\n");
        return;
    }
   int beacon_check = check_frequency(context, TARGET_FREQUENCY);
   if (beacon_check == 1) {
        printf("Signal detected at %llu Hz\n", TARGET_FREQUENCY);
    } else if (beacon_check == 0) {
        printf("No signal detected at %llu Hz\n", TARGET_FREQUENCY);
    } else {
        // Print the error code returned from check_frequency
        printf("Error: code %d\n", beacon_check);
    }
    char *sarsat_result = detect_sarsat_signal(context, SARSAT_FREQUENCY);
     if (strcmp(sarsat_result, "RAW_SIGNAL_DATA") != 0) {
        printf("SARSAT signal detected: %s\n", sarsat_result);

        // Prepare the DATA structure to hold the decoded SARSAT signal
        DATA data;
        data_memcpy(&data, sarsat_result);  // Use data_memcpy to decode the SARSAT signal

        // Extract necessary fields from the DATA structure
        COUNTRY_CODE cc = read_country_code(&data);     // Get the country code
        COORD coords = read_coordinates(&data);         // Get the coordinates
        time_t current_time = time(NULL);               // Capture the current time

        // Send the short data burst with the decoded information
        short_data_burst(&cc, NULL, &coords, current_time);
    } else {
        printf("No SARSAT signal detected.\n");
    }
    iio_context_destroy(context);
}

//display notification
void notification(char* frequency,int duration_seconds, int flash_count, char* sound_file,char* countryCode, int hexID, int encodedLocation, time_t timeReceived){
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
        send_data_burst();
    }
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
                    send_data_burst();
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
