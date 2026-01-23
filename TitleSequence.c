#define _XOPEN_SOURCE_EXTENDED
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ncurses.h>
#include <wchar.h>
#include <sys/mman.h>
#include <pthread.h>

#include "SubprocessFunctions.h"
#include "WideStringFunctions.h"

/*
================================================================================
| Function: TitleSequence                 | TL;DR                              |
|-----------------------------------------|------------------------------------|
|     The Purpose of this function is to  | Converts key presses to commands   |
| Generate a little title sequence that   |                                    |
| plays whenever the game starts. It is   |                                    |
| run when the "-title" tag is present.   |                                    |
|                                         |                                    |
|                                         |                                    |
|                                         |                                    |
|                                         |                                    |
================================================================================
*/
void *TitleSequence(void *TSharedData){
    struct D2Sshared *SharedData = (struct D2Sshared *)TSharedData;
    struct Parameters *Parameters = (struct Parameters *)calloc(1,sizeof(struct Parameters));

    Parameters->MaxRows = SharedData->MaxRows;
    Parameters->MaxCols = SharedData->MaxCols;
    Parameters->DisplayArray = SharedData->DisplayArray;
    Parameters->LineDelay = 0;
    int MaxFRows;

    MaxFRows = getrows("title.txt");
    strcpy(Parameters->Filename, "title.txt");
    Parameters->FRow = Parameters->MaxRows/2 - MaxFRows/2;
    PrintFile(Parameters, 5);
    sleep(1);

    DisplayFill(SharedData->DisplayArray,SharedData->MaxRows,SharedData->MaxCols);
    usleep(125000);
    DisplayClear(SharedData->DisplayArray,SharedData->MaxRows,SharedData->MaxCols);

    MaxFRows = getrows("Stitle.txt");
    strcpy(Parameters->Filename, "Stitle.txt");
    Parameters->FRow = Parameters->MaxRows/2 - MaxFRows/2;
    PrintFile(Parameters, 5);
    sleep(1);

    int i;
    int glide = Parameters->FRow;
    for(i=0;i<12;i++){
        DisplayClear(SharedData->DisplayArray,SharedData->MaxRows,SharedData->MaxCols);
        Parameters->FRow = glide - i;
        PrintFile(Parameters, 5);    
        usleep(75000);
    }

    Parameters->LineDelay = 37500;
    strcpy(Parameters->Filename, "mountains.txt");
    usleep(25000);
    PrintBackdrop(Parameters);

    free(Parameters);

    SharedData->MenuStart = 1;
 
    pthread_exit(NULL);
}
