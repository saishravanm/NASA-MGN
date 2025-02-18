#ifndef UTIL_H
#define UTIL_H

#include <pthread.h>

void play_sound(const char *sound_file);
void list_files (const char *path);
void* sound_thread(void *arg);


#endif