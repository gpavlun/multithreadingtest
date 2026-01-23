#define _XOPEN_SOURCE_EXTENDED
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <string.h>
#include <locale.h>
#include <ncurses.h>
#include <wchar.h>

#include <pthread.h>

#include "SubprocessFunctions.h"
#include "WideStringFunctions.h"


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
