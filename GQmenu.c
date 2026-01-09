#define _XOPEN_SOURCE_EXTENDED
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>      // fork(), pipe(), read(), write()
#include <string.h>      // string operations
#include <signal.h>      // signal handling
#include <poll.h>        
#include <ncurses.h>     // terminal UI rendering
// #include <panel.h>       // panel support for ncurses
// #include <menu.h>        // menu support for ncurses
//#include <fcntl.h>       // file control (O_NONBLOCK)
#include <sys/wait.h>    // waitpid() for process management
#include <locale.h>
#include <fcntl.h>       // file control (O_NONBLOCK)
#include <wchar.h>
#include <sys/mman.h>
#include <pthread.h>

#include "SubprocessFunctions.h"

void MainMenu(char PlayMusic, char PlayTitle);

int main(int ArgCount, char *ArgList[]){
    char PlayMusic = 0;
    char PlayTitle = 0;

    for(int i=0;i<ArgCount;i++){
        if(strcmp(ArgList[i],"-music")==0){
            PlayMusic = 1;
        }else if(strcmp(ArgList[i],"-title")==0){
            PlayTitle = 1;
        }
    }
    
    MainMenu(PlayMusic,PlayTitle);
    return 0;
}

void MainMenu(char PlayMusic, char PlayTitle){
    
    struct M2Dshared *SharedData = (struct M2Dshared *)malloc(sizeof(struct M2Dshared));

    SharedData->KillMain = 0;
    SharedData->KillDisplay = 0;
    SharedData->PlayMusic = PlayMusic;
    SharedData->PlayTitle = PlayTitle;

    pthread_t DisplayID;

    while(SharedData->KillMain==0){
        SharedData->KillDisplay = 0;
        pthread_create(&DisplayID, NULL, Display, SharedData);
        pthread_join(DisplayID, NULL);
    }
    free(SharedData);
}
