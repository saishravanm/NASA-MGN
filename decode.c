#include "decode.h"
#include <stdio.h>
#include <ncurses.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "ui.h"
#include "kml_generation.h"

void decode_and_display(const char *buffer)
{

}

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


