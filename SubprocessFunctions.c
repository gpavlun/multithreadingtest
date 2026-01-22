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
#include <linux/input.h>
#include <fcntl.h>

#include "SubprocessFunctions.h"
#include "OptionInitializer.h"
#include "WideStringFunctions.h"
#include "MusicFunctions.h"

void *Display(void *SharedData){

    struct M2Dshared *M2DSharedData = (struct M2Dshared *)SharedData;
    char RodentPath[256];
    RodentPath[0] = 0;
    //if(M2DSharedData->UseRodent){
        //printf("enter mouse file path, should look like \"/dev/input/event7\"\n");
        //scanf("%s",RodentPath);
    //}

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

    

    struct D2Sshared *D2SSharedData = (struct D2Sshared *)malloc(sizeof(struct D2Sshared));
    //pthread_mutex_init(&(D2SSharedData->lock), NULL);
    struct Pixel *DisplayArray = (struct Pixel *)calloc(MaxRows*MaxCols,sizeof(struct Pixel));
    D2SSharedData->DisplayArray = DisplayArray;
    D2SSharedData->MaxRows = MaxRows;
    D2SSharedData->MaxCols = MaxCols;
    D2SSharedData->BackStart = 0;
    D2SSharedData->MenuStart = 0;
    D2SSharedData->KillMusic = 0;
    D2SSharedData->OptionHeader = (struct OptionHeader *)InitializeOptions("MainOptions.txt");

    getmaxyx(stdscr,MaxRows,MaxCols);
    int height;
    if((8+getrows("Stitle.txt"))>((D2SSharedData->OptionHeader->OptionCount)*3-1)){
        height = (8+getrows("Stitle.txt"));
    }else{
        height = (D2SSharedData->OptionHeader->OptionCount)*3-1;
    }
    
    if(MaxRows<(height*2)||MaxCols<87){
        M2DSharedData->KillMain = 2;
        goto window_error;
    }

    struct MultiShare *MultiShare;
    MultiShare = (struct MultiShare *)malloc(sizeof(struct MultiShare));
    MultiShare->MainData = M2DSharedData;
    MultiShare->SubData = D2SSharedData;

    for(Row=0;Row<MaxRows;Row++){
        for(Col=0;Col<MaxCols;Col++){
            getmaxyx(stdscr,MaxRows,MaxCols);
            if(OldMaxRows==MaxRows&&OldMaxCols==MaxCols){
                ((DisplayArray + Row*MaxCols+Col)->Data).chars[0] = L' ';
                ((DisplayArray + Row*MaxCols+Col)->Data).chars[1] = 0;
                ((DisplayArray + Row*MaxCols+Col)->Priority) = 0;
                ((DisplayArray + Row*MaxCols+Col)->Cursor) = 0;
            }else{
                goto terminate;
            }
        }
    }

    if(OldMaxRows==MaxRows&&OldMaxCols==MaxCols){
        setcchar(&((DisplayArray + (MaxRows/2)*MaxCols+MaxCols/2)->Data),L"A",A_NORMAL,0,NULL);
    }else{
        goto terminate;
    }
    
    pthread_t InputID;
    pthread_create(&InputID, NULL, InputScan, MultiShare);

    struct RodentData *RodentData = (struct RodentData *)calloc(1,sizeof(struct RodentData));
    RodentData->SharedData = D2SSharedData;
    RodentData->Cursor.attr = A_NORMAL;
    RodentData->Cursor.chars[0] = '+';
    RodentData->Cursor.chars[1] = '\0';
    RodentData->Paths = RodentPath;

    pthread_t RodentID;
    if(M2DSharedData->UseRodent){
        pthread_create(&RodentID, NULL, Rodent, RodentData);
    }
    
    pthread_t TitleID;
    if(M2DSharedData->PlayTitle){
        pthread_create(&TitleID, NULL, TitleSequence, D2SSharedData);
    }else{
        D2SSharedData->MenuStart = 1;
    }

    pthread_t MenuID;
    pthread_create(&MenuID, NULL, Menu, D2SSharedData);

    pthread_t BackgroundID;
    if(M2DSharedData->PlayBackground){
        pthread_create(&BackgroundID, NULL, Background, D2SSharedData);
    }
    
    pthread_t MusicID;
    if(M2DSharedData->PlayMusic){
        pthread_create(&MusicID, NULL, Music, D2SSharedData); 
    } 




    while(M2DSharedData->KillDisplay==0&&M2DSharedData->KillMain==0){
        for(Row=0;Row<MaxRows;Row++){
            for(Col=0;Col<MaxCols;Col++){

                getmaxyx(stdscr,MaxRows,MaxCols);
                if(OldMaxRows==MaxRows&&OldMaxCols==MaxCols){

                    if((DisplayArray + Row*MaxCols+Col)->Cursor){
                        mvadd_wch(Row,Col,&(RodentData->Cursor));
                    }else{
                        mvadd_wch(Row,Col,&((DisplayArray + Row*MaxCols+Col)->Data));
                    }
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

    pthread_cancel(MenuID);
    pthread_join(MenuID, NULL);

    if(M2DSharedData->UseRodent){
        pthread_cancel(RodentID);
        pthread_join(RodentID, NULL);
    }
    if(M2DSharedData->PlayBackground){
        pthread_cancel(BackgroundID);
        pthread_join(BackgroundID, NULL);
    }
    if(M2DSharedData->PlayTitle){
        pthread_cancel(TitleID);
        pthread_join(TitleID, NULL);
    }
    if(M2DSharedData->PlayMusic){
        D2SSharedData->KillMusic = 1;
        pthread_join(MusicID,NULL);
    }

    free(MultiShare);

    window_error:
    free(D2SSharedData);
    free(DisplayArray);

    M2DSharedData->MaxRows = MaxRows;
    M2DSharedData->MaxCols = MaxCols;

    endwin();
    pthread_exit(NULL);
    return NULL;
}











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
            //printf("Mouse: %s\n", paths);
        }
        close(fd);
    }

    struct RodentData *RodentData = (struct RodentData *)TRodentData;
    
    float x = (RodentData->SharedData->MaxCols)/2;
    float y = (RodentData->SharedData->MaxRows)/2;
    float tempev;
    float oldx = x;
    float oldy = y;
    RodentData->Paths = paths;

//"/dev/input/event7"
    int fd;
    struct input_event ev;
    int currentpath = 0;

    fd = open(RodentData->Paths + currentpath, O_RDONLY);
    while (1) {
        
        while(fd < 0){
            continue;
        }

        read(fd, &ev, sizeof(ev));

        currentpath++;
        if(!(currentpath<npaths)){
            currentpath=0;
        }

        /* Mouse movement */
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
        //clear();
        
        (RodentData->SharedData->DisplayArray + ((int)oldy)*(RodentData->SharedData->MaxCols) + ((int)oldx))->Cursor = 0;
        (RodentData->SharedData->DisplayArray + ((int)y)*(RodentData->SharedData->MaxCols) + ((int)x))->Cursor = 1;
        oldx = x;
        oldy = y;
        

        /* Mouse clicks */
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

    //getch();


    free(Parameters);

    //PrintFile(Parameters);


    //SharedData->BackStart = 1;

    SharedData->MenuStart = 1;


    
    pthread_exit(NULL);
}


/*
================================================================================
| Function: Menu                          | TL;DR                              |
|-----------------------------------------|------------------------------------|
|     The Purpose of this function is to  | Draws the title and buttons        |
| handle the interactable part of the     |                                    |
| main menu. It will only generate the    |                                    |
| static parts of the menu once, then the |                                    |
| buttons will be continuously redrawn to |                                    |
| show which option is currently being    |                                    |
| being selected. The options come from   |                                    |
| the InitailizeOptions function and are  |                                    |
| from the "options.txt" file. The title  |                                    |
| images are from the "title.txt" files.  |                                    |
|                                         |                                    |
================================================================================
*/
void *Menu(void *TShared){
    
    struct D2Sshared *SharedData = (struct D2Sshared *)TShared;
    struct OptionHeader *OptionHeader = SharedData->OptionHeader;
    SharedData->SelectedOption = 0;
    uint32_t MaxRows = SharedData->MaxRows;
    uint32_t MaxCols = SharedData->MaxCols;
    uint32_t Length;
    struct Pixel *DisplayArray = SharedData->DisplayArray;
    struct Option *Option;
    attr_t attr;

    while(SharedData->MenuStart==0){
        usleep(125000);
    }
    SharedData->BackStart = 1;

    struct Parameters *Parameters = (struct Parameters *)calloc(1,sizeof(struct Parameters));
    Parameters->MaxRows = SharedData->MaxRows;
    Parameters->MaxCols = SharedData->MaxCols;
    Parameters->DisplayArray = SharedData->DisplayArray;

    strcpy(Parameters->Filename, "mountains.txt");
    PrintBackdrop(Parameters);



    strcpy(Parameters->Filename, "Stitle.txt");
    Parameters->FRow = Parameters->MaxRows/2 - getrows("Stitle.txt") - 8;
    PrintFile(Parameters, 5); 

    int i;
    Parameters->LineDelay = 37500;
    for(i=0;i<OptionHeader->OptionCount;i++){
        strcpy(Parameters->Filename, "box.txt");
        Parameters->FRow = (Parameters->MaxRows/2+i*3)- getrows("box.txt")/2;
        PrintFile(Parameters, 5);
    }
    
    
    while(1){
        for(i=0;i<OptionHeader->OptionCount;i++){
            if(i==SharedData->SelectedOption){
                attr = A_BOLD;
            }else{
                attr = A_NORMAL;
            }
            Option = ((OptionHeader->List) + i);
            Length = wstrlen(Option->OptionText);
            Pfuencode((DisplayArray + (MaxRows/2 + i*3)*MaxCols + (MaxCols/2 - Length/2)), Option->OptionText, attr, Length, 5);
        }
        usleep(10000);
    }
    return NULL;
}





/*
================================================================================
| Function: Background                    | TL;DR                              |
|-----------------------------------------|------------------------------------|
|     The Purpose of this function is to  | Draws the animated background      |
| handle the dynaminc background of the   |                                    |
| main menu. It generates a scene from a  |                                    |
| a file callled "background" and it      |                                    |
| scrolls behind the static parts of the  |                                    |
| menu. The background has many conflicts |                                    |
| with the static parts of the menu from  |                                    |
| the menu function. This is yet to be    |                                    |
| resolved.                               |                                    |
|                                         |                                    |
================================================================================
*/
void *Background(void *TSharedData){
    
    struct D2Sshared *SharedData = (struct D2Sshared *)TSharedData;
    setlocale(LC_ALL, "");

    uint32_t MaxRows = SharedData->MaxRows;
    uint32_t MaxCols = SharedData->MaxCols;
    struct Pixel *DisplayArray = SharedData->DisplayArray;

    while(SharedData->BackStart==0){
        usleep(250000);
    }

    wchar_t buffer[1024];
    
    uint32_t len,BRow,BLen; 
    int32_t CompareValue,nCopy;

    wchar_t timer[8];
    FILE *BackgroundFile;

    BackgroundFile = fopen("background.txt","r");
    fgetws(buffer,1024,BackgroundFile);

    BLen = wterminate(buffer);

    fclose(BackgroundFile);

    while(1){

        for(len=0;len<(BLen+2);len++){
        
            BackgroundFile = fopen("background.txt","r");
            
            BRow = 0;
            while(fgetws(buffer,1024,BackgroundFile)!=NULL&&BRow!=MaxRows){

                wterminate(buffer);

                Pfuencode(DisplayArray + BRow*MaxCols, buffer+len, A_NORMAL, wstrlen(buffer)>MaxCols ? MaxCols : wstrlen(buffer), 0);
                
                nCopy = 0;    
                CompareValue = MaxCols-(wstrlen(buffer+len))-(wstrlen(buffer))*(nCopy+1);
                while(CompareValue>0){

                    Pfuencode(DisplayArray + BRow*MaxCols + (wstrlen(buffer+len)) + (wstrlen(buffer))*nCopy, buffer, A_NORMAL, wstrlen(buffer), 0);
                    nCopy++;
                    CompareValue = MaxCols-(wstrlen(buffer+len))-(wstrlen(buffer))*(nCopy+1);

                }
                if(MaxCols-((wstrlen(buffer+len)) + (wstrlen(buffer))*nCopy)!=0){

                    Pfuencode(DisplayArray + BRow*MaxCols + (wstrlen(buffer+len)) + (wstrlen(buffer))*nCopy, buffer, A_NORMAL, MaxCols - (wstrlen(buffer+len)) - (wstrlen(buffer))*nCopy, 0);
    
                }

                swprintf(timer,8,L"%d",len);
                Pfuencode(DisplayArray + MaxCols + 1, timer, A_NORMAL, wstrlen(timer),0);
                BRow++;
            } 
            fclose(BackgroundFile);
            usleep(125000);
        }
    }
}
/*
================================================================================
| Function: InputHandler                  | TL;DR                              |
|-----------------------------------------|------------------------------------|
|     The Purpose of this function is to  | Converts key presses to commands   |
| handle the user key presses from the    |                                    |
| InputScan function. It will give        |                                    |
| instructions based on the key presses.  |                                    |
|                                         |                                    |
| Currently, unimplemented                |                                    |
|                                         |                                    |
|                                         |                                    |
================================================================================
*/
void *InputHandler(void *TSharedData){
    return NULL;
}

/*
================================================================================
| Function: InputScan                     | TL;DR                              |
|-----------------------------------------|------------------------------------|
|     The Purpose of this function is to  | Reads button presses from the user |
| handle the keyboard inputs from the     |                                    |
| user. The function clears out any       |                                    |
| queued inputs from the stdin buffer and |                                    |
| then reads the first newly pressed      |                                    |
| button. This input will be handled by   |                                    |
| the input handler function in the       |                                    |
| future, but is currently handled within |                                    |
================================================================================
*/
void *InputScan(void *TSharedData){

    usleep(125000);

    struct MultiShare *SharedData = (struct MultiShare *)TSharedData;

    char buffer[255];
    int flag, ReturnValue;
    int UserInput;
    struct pollfd pfd;

    pfd.fd = STDIN_FILENO;
    pfd.events = POLLIN; 

    //wchar_t timer[8];

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
    while(!((UserInput==10||UserInput==13)&&SharedData->SubData->SelectedOption==2)){
        UserInput = getch();

        // swprintf(timer,8,L"        ");
        // swprintf(timer,8,L"%d",UserInput);
        // Pfuencode((SharedData->SubData->DisplayArray + SharedData->SubData->MaxCols + 1), timer, A_NORMAL, wstrlen(timer),0);
        switch(UserInput){
            case 258:
                if(SharedData->SubData->SelectedOption<(SharedData->SubData->OptionHeader->OptionCount-1)){
                    (SharedData->SubData->SelectedOption)++;
                }
                break;
            case 259:
                if(SharedData->SubData->SelectedOption>0){
                    (SharedData->SubData->SelectedOption)--;
                }
                break;
        }
    }
    SharedData->MainData->KillMain = 1;
    pthread_exit(NULL);
}






void DisplayClear(struct Pixel *DisplayArray, uint32_t MaxRows, uint32_t MaxCols){
    uint32_t Row,Col;
    for(Row=0;Row<MaxRows;Row++){
        for(Col=0;Col<MaxCols;Col++){
            (DisplayArray + Row*MaxCols+Col)->Priority = 0;
            setcchar(&((DisplayArray + Row*MaxCols+Col)->Data),L" ",A_NORMAL,0,NULL);
        }
    }    
}
void DisplayFill(struct Pixel *DisplayArray, uint32_t MaxRows, uint32_t MaxCols){
    uint32_t Row,Col;
    for(Row=0;Row<MaxRows;Row++){
        for(Col=0;Col<MaxCols;Col++){
            setcchar(&((DisplayArray + Row*MaxCols+Col)->Data),L"█",A_NORMAL,0,NULL);
        }
    }     
}
int getrows(char *Name){
    FILE *File = fopen(Name,"r");
    wchar_t Buffer[1024];
    int Rows = 0;
    while(fgetws(Buffer, 1024,File)!=NULL){
        Rows++;
    }
    fclose(File);
    return Rows;
}
int getcols(char *Name){
    FILE *File = fopen(Name,"r");
    wchar_t Buffer[1024];

    if(fgetws(Buffer,1024,File)==NULL){
        return -1;
    }
    
    fclose(File);
    return wterminate(Buffer);
}
void PrintFile(struct Parameters *Parameters, int Priority){

    setlocale(LC_ALL, "");

    uint32_t MaxCols = Parameters->MaxCols;

    struct Pixel *DisplayArray = Parameters->DisplayArray;
   
    wchar_t buffer[1024];
    
    uint32_t FRow;

    FILE *File;

    File = fopen(Parameters->Filename,"r");

    fgetws(buffer,1024,File);
    wterminate(buffer);

    fclose(File);

    File = fopen(Parameters->Filename,"r");

    FRow = 0;
    while(fgetws(buffer,1024,File)!=NULL){

        wterminate(buffer);
        Pfuencode(DisplayArray + (Parameters->FRow + FRow)*MaxCols + MaxCols/2 - wstrlen(buffer)/2, buffer, A_NORMAL, wstrlen(buffer), Priority);
        FRow++;
        usleep(Parameters->LineDelay);

    } 

    fclose(File);

}
void PrintBackdrop(struct Parameters *Parameters){
    setlocale(LC_ALL, "");

    struct Pixel *DisplayArray = Parameters->DisplayArray;
   
    wchar_t buffer[1024];
    
    uint32_t FileRows = getrows(Parameters->Filename);
    uint32_t FileCols = getcols(Parameters->Filename);

    FILE *File = fopen(Parameters->Filename,"r");

    int Rdiff,Cdiff;
    int Rstart = 0;
    int Cstart = 0;

    int Cend = Parameters->MaxCols;
    int Rend = FileRows - 1;

    if(Parameters->MaxRows<FileRows){
        Rdiff = FileRows - Parameters->MaxRows;
        Rend = Parameters->MaxRows;
    }else{
        Rdiff = 0;
        Rstart = (Parameters->MaxRows - FileRows)/2;
        Rend = FileRows + Rstart;
    }
    if(Parameters->MaxCols<FileCols){
        Cdiff = FileCols - Parameters->MaxCols;
        Cend = Parameters->MaxCols;
    }else{
        Cdiff = 0;
        Cstart = (Parameters->MaxCols - FileCols)/2;
        Cend = FileCols;
    }

    for(int i=0;i<(Rdiff/2);i++){
        fgetws(buffer,1024,File);
    }

    for(int j=Rstart;j<Rend;j++){
        if(Parameters->MaxCols<FileCols){
            fgetws(buffer,1024,File);
            Pfuencode(DisplayArray + j*(Parameters->MaxCols) + Cstart,&(buffer[Cdiff/2]), A_NORMAL, Cend, 1);
        }else{
            fgetws(buffer,1024,File);
            Pfuencode(DisplayArray + j*(Parameters->MaxCols) + Cstart,buffer, A_NORMAL, Cend,1);            
        }
        usleep(Parameters->LineDelay);
    }
}
