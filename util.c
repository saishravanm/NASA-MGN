#include <pthread.h>
#include <ncurses.h> 
#include <sys/stat.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "util.h"
#include "ui.h"
//play sound given sound file
void play_sound(const char *sound_file)
{
    char command[256];
    snprintf(command, sizeof(command), 
             "aplay ./sound_files/sarsat_alert_sound.wav > /dev/null 2>&1");

    int result = system(command);
    if (result != 0) {
        printw("Failed to play sound: %s\n", sound_file);
    }
}

void list_files(const char *path)
{
    struct dirent *entry;
    struct stat file_stat;
    DIR *dp = opendir(path);

    if(dp == NULL) {
        perror("opendir");
        printw("Error opening directory: %s\n", path);
        return;
    }
    printw("Opening directory: %s\n", path);

    while((entry = readdir(dp)) != NULL) {
        
        if(entry->d_name[0] == '.') {
            continue;
        }

        char full_path[1024];
        if (path[strlen(path) - 1] == '/') {
            snprintf(full_path, sizeof(full_path), "%s%s", path, entry->d_name);
        } else {
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        }
        if(stat(full_path, &file_stat) == -1) {
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

    draw_button(back_button[0].y, back_button[0].x, back_text[0], false);
}

void* sound_thread(void *arg)
{
    play_sound("asd");
    return NULL;
}