#define _XOPEN_SOURCE_EXTENDED
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <string.h>
#include <locale.h>
#include <ncurses.h>
#include <poll.h>
#include <wchar.h>

#include <sys/mman.h>
#include <pthread.h>

#include "SubprocessFunctions.h"
#include "OptionInitialize.h"
#include "WideStringFunctions.h"
#include "MusicFunctions.h"

void main(void){
    struct D2Sshared d;
    d.KillMusic = 0;
    pthread_t MusicID;
    pthread_create(&MusicID, NULL, Music, &d); 
    pthread_join(MusicID, NULL);
}