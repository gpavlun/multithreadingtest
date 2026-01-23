#define _XOPEN_SOURCE_EXTENDED
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <string.h>
#include <ncurses.h>
#include <wchar.h>

#include <pthread.h>
#include <linux/input.h>
#include <fcntl.h>

#include "SubprocessFunctions.h"

#define BITS_PER_LONG   (sizeof(unsigned long) * 8)
#define NBITS(x)        ((((x) - 1) / BITS_PER_LONG) + 1)
#define TEST_BIT(bit, array) ((array[(bit) / BITS_PER_LONG] >> ((bit) % BITS_PER_LONG)) & 1)

int is_mouse(int fd) {
    unsigned long evbit[NBITS(EV_MAX)];
    unsigned long relbit[NBITS(REL_MAX)];
    unsigned long keybit[NBITS(KEY_MAX)];

    memset(evbit, 0, sizeof(evbit));
    memset(relbit, 0, sizeof(relbit));
    memset(keybit, 0, sizeof(keybit));

    if (ioctl(fd, EVIOCGBIT(0, sizeof(evbit)), evbit) < 0)
        return 0;

    if (!TEST_BIT(EV_REL, evbit))
        return 0;

    ioctl(fd, EVIOCGBIT(EV_REL, sizeof(relbit)), relbit);
    ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keybit)), keybit);

    return TEST_BIT(REL_X, relbit) && TEST_BIT(REL_Y, relbit) && TEST_BIT(BTN_LEFT, keybit);
}


void *Rodent(void *TRodentData){

    char path[64];

    char *paths = malloc(sizeof(path));
    int npaths = 0;

    for (int i = 0; i < 32; i++) {
        snprintf(path, sizeof(path), "/dev/input/event%d", i);

        int fd = open(path, O_RDONLY);

        if(fd>=0&&is_mouse(fd)){
            strcpy(paths+npaths, path);
            npaths++;
            paths = realloc(paths,sizeof(path)*npaths);
        }
        close(fd);
    }

    while(npaths==0){
        sleep(1);
    }

    struct RodentData *RodentData = (struct RodentData *)TRodentData;
    
    float x = (RodentData->SharedData->MaxCols)/2;
    float y = (RodentData->SharedData->MaxRows)/2;
    float tempev;
    float oldx = x;
    float oldy = y;
    RodentData->Paths = paths;

    int *FileDescriptor;
    FileDescriptor = malloc(sizeof(int)*npaths);
    struct input_event ev;
    int currentpath = 0;

    for(currentpath=0; currentpath<npaths; currentpath++){
        *(FileDescriptor + currentpath) = open(RodentData->Paths + currentpath, O_RDONLY | O_NONBLOCK);
    }
    currentpath = 0;
    
    int err;
    while(1){
        
        usleep(1000);
        err = read(*(FileDescriptor+currentpath), &ev, sizeof(ev));
        currentpath++;
        if(!(currentpath<npaths)){
            currentpath=0;
            
        }
        if((err==0)||(err==-1)){
            continue;
        }

        if (ev.type == EV_REL && ev.code == REL_X){
            tempev = ev.value;
            x+=tempev/4;
        }
        if (ev.type == EV_REL && ev.code == REL_Y){
            tempev = ev.value;
            y+=tempev/7;
        }
        if(y<0){
            y=0;
        }else if(y>=(RodentData->SharedData->MaxRows)){
            y=(RodentData->SharedData->MaxRows)-1;
        }
        if(x<0){
            x=0;
        }else if(x>=(RodentData->SharedData->MaxCols)){
            x=(RodentData->SharedData->MaxCols)-1;
        }
        
        (RodentData->SharedData->DisplayArray + ((int)oldy)*(RodentData->SharedData->MaxCols) + ((int)oldx))->Cursor = 0;
        (RodentData->SharedData->DisplayArray + ((int)y)*(RodentData->SharedData->MaxCols) + ((int)x))->Cursor = 1;
        oldx = x;
        oldy = y;
        



        if (ev.type == EV_KEY) {
            if (ev.code == BTN_LEFT){
                if(ev.value){
                    RodentData->Cursor.chars[0] = 'L';
                }else{
                    RodentData->Cursor.chars[0] = '+';
                }
                RodentData->Cursor.chars[1] = '\0';
            }
            if (ev.code == BTN_RIGHT){
                if(ev.value){
                    RodentData->Cursor.chars[0] = 'R';
                }else{
                    RodentData->Cursor.chars[0] = '+';
                }
                RodentData->Cursor.chars[1] = '\0';
            }
            if (ev.code == BTN_MIDDLE){
                if(ev.value){
                    RodentData->Cursor.chars[0] = 'M';
                }else{
                    RodentData->Cursor.chars[0] = '+';
                }
                RodentData->Cursor.chars[1] = '\0';
            }
                
        }
    }
}



