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
#include "OptionInitializer.h"
#include "WideStringFunctions.h"
#include "MusicFunctions.h"

void *Display(void *SharedData){
    setlocale(LC_ALL, "");  
    initscr();              
    noecho();               
    raw();                  
    keypad(stdscr, TRUE);   
    curs_set(0);
    unsigned int MaxRows,MaxCols;
    getmaxyx(stdscr,MaxRows,MaxCols);
    unsigned int OldMaxRows = MaxRows;
    unsigned int OldMaxCols = MaxCols;
    unsigned int Row,Col;

    struct M2Dshared *M2DSharedData = (struct M2Dshared *)SharedData;
    struct D2Sshared *D2SSharedData = (struct D2Sshared *)malloc(sizeof(struct D2Sshared));
    //pthread_mutex_init(&(D2SSharedData->lock), NULL);
    cchar_t *DisplayArray = (cchar_t *)calloc(MaxRows*MaxCols,sizeof(cchar_t));
    D2SSharedData->DisplayArray = DisplayArray;
    D2SSharedData->MaxRows = MaxRows;
    D2SSharedData->MaxCols = MaxCols;
    D2SSharedData->BackStart = 0;
    D2SSharedData->KillMusic = 0;

    for(Row=0;Row<MaxRows;Row++){
        for(Col=0;Col<MaxCols;Col++){
            getmaxyx(stdscr,MaxRows,MaxCols);
            if(OldMaxRows==MaxRows&&OldMaxCols==MaxCols){
                setcchar(DisplayArray + Row*MaxCols+Col,L" ",A_NORMAL,0,NULL);
            }else{
                goto terminate;
            }
        }
    }
    getmaxyx(stdscr,MaxRows,MaxCols);
    if(OldMaxRows==MaxRows&&OldMaxCols==MaxCols){
        setcchar(DisplayArray + (MaxRows/2)*MaxCols+MaxCols/2,L"A",A_NORMAL,0,NULL);
    }else{
        goto terminate;
    }
    
    pthread_t InputID;
    pthread_create(&InputID, NULL, InputScan, &(M2DSharedData->KillMain));

    pthread_t BackgroundID;
    pthread_create(&BackgroundID, NULL, Background, D2SSharedData);
    
    pthread_t TitleID;
    if(M2DSharedData->PlayTitle) pthread_create(&TitleID, NULL, TitleSequence, D2SSharedData);

    pthread_t MusicID;
    if(M2DSharedData->PlayMusic){
        D2SSharedData->BackStart = 1;
        pthread_create(&MusicID, NULL, Music, D2SSharedData); 
    } 
       

    while(M2DSharedData->KillDisplay==0&&M2DSharedData->KillMain==0){
        for(Row=0;Row<MaxRows;Row++){
            for(Col=0;Col<MaxCols;Col++){

                getmaxyx(stdscr,MaxRows,MaxCols);
                if(OldMaxRows==MaxRows&&OldMaxCols==MaxCols){
                    mvadd_wch(Row,Col,(DisplayArray + Row*MaxCols+Col));
                }else{
                    goto terminate;
                }  
            }
        }

        refresh();

        usleep(10000);

    }





    terminate:
    
    pthread_cancel(InputID);
    pthread_join(InputID, NULL);

    pthread_cancel(BackgroundID);
    pthread_join(BackgroundID, NULL);

    if(M2DSharedData->PlayTitle){
        pthread_cancel(TitleID);
        pthread_join(TitleID, NULL);
    }
    if(M2DSharedData->PlayMusic){
        D2SSharedData->KillMusic = 1;
        pthread_join(MusicID,NULL);
    }

    free(D2SSharedData);
    free(DisplayArray);
    endwin();
    pthread_exit(NULL);
    return NULL;
}



void PrintFile(struct Parameters *Parameters){

    setlocale(LC_ALL, "");

    uint32_t MaxRows = Parameters->MaxRows;
    uint32_t MaxCols = Parameters->MaxCols;
    uint32_t MaxFRows = Parameters->MaxFRows;

    cchar_t *DisplayArray = Parameters->DisplayArray;
   
    wchar_t buffer[1024];
    
    uint32_t FRow,FLen; 

    FILE *File;

    File = fopen(Parameters->Filename,"r");

    fgetws(buffer,1024,File);
    FLen = wterminate(buffer);

    fclose(File);

    File = NULL;
    
    if(MaxCols>FLen&&MaxRows>MaxFRows){
        File = fopen(Parameters->Filename,"r");

        for(FRow=0;FRow<MaxFRows;FRow++){
            
            fgetws(buffer,1024,File);    
            wterminate(buffer);
            uencode(DisplayArray + (Parameters->FRow + FRow)*MaxCols + MaxCols/2 - wstrlen(buffer)/2, buffer, wstrlen(buffer));
        
        } 
 
        fclose(File);
    }
    
}

void *TitleSequence(void *TSharedData){
    struct D2Sshared *SharedData = (struct D2Sshared *)TSharedData;
    struct Parameters *Parameters = (struct Parameters *)malloc(sizeof(struct Parameters));

    Parameters->MaxRows = SharedData->MaxRows;
    Parameters->MaxCols = SharedData->MaxCols;
    Parameters->DisplayArray = SharedData->DisplayArray;
    Parameters->lock = SharedData->lock;
    Parameters->MaxFRows = 6;

    strcpy(Parameters->Filename, "title.txt");
    Parameters->FRow = Parameters->MaxRows/2 - Parameters->MaxFRows/2;
    PrintFile(Parameters);
    sleep(1);

    DisplayFill(SharedData->DisplayArray,SharedData->MaxRows,SharedData->MaxCols);
    usleep(125000);
    DisplayClear(SharedData->DisplayArray,SharedData->MaxRows,SharedData->MaxCols);

    strcpy(Parameters->Filename, "Stitle.txt");
    Parameters->FRow = Parameters->MaxRows/2 - Parameters->MaxFRows/2;
    PrintFile(Parameters);
    sleep(1);
    int i;
    int glide = Parameters->FRow;
    for(i=0;i<glide;i++){
        DisplayClear(SharedData->DisplayArray,SharedData->MaxRows,SharedData->MaxCols);
        Parameters->FRow = glide - i;
        PrintFile(Parameters);    
        usleep(125000);
    }
    SharedData->BackStart = 1;
    pthread_exit(NULL);
}




/*
Function: Background
This function generates the animated background the scrolls
behind the main menu. Though not yet implemented, I intend
to include a conflict area that will prevent it from writing
into the space occupied by the options list
*/
void *Background(void *TSharedData){

    struct D2Sshared *SharedData = (struct D2Sshared *)TSharedData;
    setlocale(LC_ALL, "");

    uint32_t MaxRows = SharedData->MaxRows;
    uint32_t MaxCols = SharedData->MaxCols;
    cchar_t *DisplayArray = SharedData->DisplayArray;
   
    while(SharedData->BackStart==0){
        usleep(250000);
    }

    wchar_t buffer[1024];
    
    uint32_t CRows,len,BRow,BLen; 
    int32_t CompareValue,nCopy;

    wchar_t timer[8];
    FILE *BackgroundFile;

    BackgroundFile = fopen("background.txt","r");
    fgetws(buffer,1024,BackgroundFile);

    BLen = wterminate(buffer);

    fclose(BackgroundFile);

    while(1){
        if(MaxRows>21){
            CRows = 21;
        }else{
            CRows = MaxRows;
        }
        for(len=0;len<(BLen+2);len++){
            BackgroundFile = fopen("background.txt","r");
            for(BRow=0;BRow<CRows;BRow++){

                fgetws(buffer,1024,BackgroundFile);    
                wterminate(buffer);

                uencode(DisplayArray + BRow*MaxCols, buffer+len, wstrlen(buffer)>MaxCols ? MaxCols : wstrlen(buffer));
                
                nCopy = 0;    
                CompareValue = MaxCols-(wstrlen(buffer+len))-(wstrlen(buffer))*(nCopy+1);
                while(CompareValue>0){

                    uencode(DisplayArray + BRow*MaxCols + (wstrlen(buffer+len)) + (wstrlen(buffer))*nCopy, buffer, wstrlen(buffer));
                    nCopy++;
                    CompareValue = MaxCols-(wstrlen(buffer+len))-(wstrlen(buffer))*(nCopy+1);

                }
                if(MaxCols-((wstrlen(buffer+len)) + (wstrlen(buffer))*nCopy)!=0){

                    uencode(DisplayArray + BRow*MaxCols + (wstrlen(buffer+len)) + (wstrlen(buffer))*nCopy, buffer, MaxCols - (wstrlen(buffer+len)) - (wstrlen(buffer))*nCopy);
    
                }

                swprintf(timer,8,L"%d",len);
                uencode(DisplayArray + MaxCols + 1, timer,wstrlen(timer));
            } 
            fclose(BackgroundFile);
            usleep(125000);
        }
    }
}




void *InputScan(void *TInputCode){

    sleep(1);

    uint32_t *InputCode = (uint32_t *)TInputCode;

    char buffer[255];
    int flag, ReturnValue;
    int UserInput;
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

    UserInput = 0;
    while(UserInput!=10){
        UserInput = getch();
    }
    *InputCode = 1;
    pthread_exit(NULL);
}



void DisplayClear(cchar_t *DisplayArray, uint32_t MaxRows, uint32_t MaxCols){
    uint32_t Row,Col;
    for(Row=0;Row<MaxRows;Row++){
        for(Col=0;Col<MaxCols;Col++){
            setcchar(DisplayArray + Row*MaxCols+Col,L" ",A_NORMAL,0,NULL);
        }
    }    
}
void DisplayFill(cchar_t *DisplayArray, uint32_t MaxRows, uint32_t MaxCols){
    uint32_t Row,Col;
    for(Row=0;Row<MaxRows;Row++){
        for(Col=0;Col<MaxCols;Col++){
            setcchar(DisplayArray + Row*MaxCols+Col,L"█",A_NORMAL,0,NULL);
        }
    }     
}

