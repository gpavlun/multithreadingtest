/*
Subprocess Functions Source
*/
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




void *Control(void *TSharedData){

    struct D2Sshared *SharedData = (struct D2Sshared *)TSharedData;
    
    uint32_t MaxRows = SharedData->MaxRows;
    uint32_t MaxCols = SharedData->MaxCols;
    cchar_t *DisplayArray = SharedData->DisplayArray;
    struct OptionHeader *OptionHeader = SharedData->OptionHeader;

    int OptionCount = OptionHeader->OptionCount;

    unsigned int Row, Col, TRow, TCol;
    unsigned int InputCode;
    char buffer[256];
    int wbuffer[256];
    int CurrentOption;
    int SelectedOption = 0;
    int IOScan = 1;
    int tr, tc;

    pthread_t InputID;
    pthread_create(&InputID, NULL, InputScan, &InputCode);

    while(IOScan!=0){

        for(CurrentOption=0;CurrentOption<OptionCount-1;CurrentOption++){
            
            Row = MaxRows * 1/3 + 2;  // Menu starts below title
            Col = MaxCols * .5;        // Centered horizontally
            TRow = Row + CurrentOption;           // One line per option
            TCol = Col - (strlen(((OptionHeader->List)+CurrentOption)->OptionText)/2) - 1;  // Left bracket position
            
            if(CurrentOption==SelectedOption){

                buffer[0] = '[';
                narrow2wide(wbuffer,buffer,1);
                wencode(DisplayArray + TRow*MaxCols + TCol, wbuffer, 1);

            }else{

                buffer[0] = ' ';
                narrow2wide(wbuffer,buffer,1);
                wencode(DisplayArray + TRow*MaxCols + TCol, wbuffer, 1);   

            }


            TRow = Row + CurrentOption;
            TCol = Col - (strlen(((OptionHeader->List)+CurrentOption)->OptionText)/2);
            snprintf(buffer, sizeof(buffer), "%s",((OptionHeader->List)+CurrentOption)->OptionText);
            narrow2wide(wbuffer,buffer,strlen(buffer));
            wencode(DisplayArray + TRow*MaxCols + TCol, wbuffer, strlen(buffer));               


            TRow = Row + CurrentOption;
            TCol = Col + (strlen(((OptionHeader->List)+CurrentOption)->OptionText)/2);
            if(CurrentOption==SelectedOption){

                buffer[0] = ']';
                narrow2wide(wbuffer,buffer,1);
                wencode(DisplayArray + TRow*MaxCols + TCol, wbuffer, 1);

            }else{

                buffer[0] = ' ';
                narrow2wide(wbuffer,buffer,1);
                wencode(DisplayArray + TRow*MaxCols + TCol, wbuffer, 1);

            }    
        }

        switch(InputCode){
        case 1:
            // UP arrow: move to previous option
            if(SelectedOption>0) SelectedOption--;
            InputCode = 0;
            break;
        case 2:
            // DOWN arrow: move to next option
            if(SelectedOption<((OptionHeader->OptionCount)-2)) SelectedOption++;
            InputCode = 0;
            break;
        case 5:
            // ENTER key: process selection
            switch(SelectedOption){
            case 3:
                // Option 3 is the exit option
                Row = MaxRows * .5;
                Col = MaxCols * .5;
                SharedData->ExitKey = 1;
                InputCode = 0;
                break;
            default:
                // Other options: send selection message
                snprintf(buffer, sizeof(buffer), "option %d selected",SelectedOption+1);
                narrow2wide(wbuffer,buffer,strlen(buffer));
                wencode(DisplayArray + MaxCols + 1, wbuffer, strlen(buffer));   
                InputCode = 0;
                break;
            }
            break;
        }
        usleep(1000);
    }
}


void *InputScan(void *TInputCode){

    uint32_t *InputCode = (uint32_t *)TInputCode;

    char buffer[255];
    int flag, ReturnValue;
    int *UserInput;
    struct pollfd pfd;

    pfd.fd = STDIN_FILENO;
    pfd.events = POLLIN; 

    flag = 0;
    while(flag==0){
        ReturnValue = poll(&pfd, 1, 0); 
        if (ReturnValue > 0 && (pfd.revents & POLLIN)){
            read(STDIN_FILENO, buffer, 255);
        }else{
            flag = 1;
        }
    }

    *UserInput = 0;
    while(*UserInput!=10){
        *UserInput = getch();
    }
    *InputCode = 1;

    // *UserInput = 3;
    // *InputCode = 0;
    // while((*InputCode!=1)||(*InputCode!=2)||(*InputCode!=5)){ 
    //     if(*UserInput==KEY_UP){
    //         // Up arrow
    //         *InputCode = 1;
    //     }else if(*UserInput==KEY_DOWN){
    //         // Down arrow
    //         *InputCode = 2;
    //     }else if(*UserInput==10){
    //         // Enter key
    //         *InputCode = 5;
    //     }else{
    //         *UserInput = getch();
    //     }
    // }
}

