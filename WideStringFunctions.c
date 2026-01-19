/*
Wide Character String Functions Source
*/
#define _XOPEN_SOURCE_EXTENDED
#include <ncurses.h>


#include "WideStringFunctions.h"
#include "SubprocessFunctions.h"

int wterminate(wchar_t *buffer){
    int i = 0;
    while(buffer[i]!='\n'){
        i++;
    }
    buffer[i] = 0;
    return i;
}
int wstrlen(wchar_t *buffer){
    int i = 0;
    while(buffer[i]!=0){
        i++;
    }
    return i;
}
int wstrcpy(wchar_t *dest, wchar_t *source){
    int i = 0;
    while(source[i]!=0){
        dest[i] = source[i];
        i++;
    }
    dest[i] = 0; 
    return 0;
}
int uencode(struct Pixel *dest, wchar_t *source, int n){
    int i = 0;

    for(i=0;i<n;i++){

        ((dest+i)->Data.chars)[0] = *(source + i);
        ((dest+i)->Data.chars)[1] = L'\0';
    }
    return 0;
}
int fuencode(struct Pixel *dest, wchar_t *source, attr_t attr, int n){
    int i = 0;

    for(i=0;i<n;i++){

        (dest+i)->Data.attr = attr;
        ((dest+i)->Data.chars)[0] = *(source + i);
        ((dest+i)->Data.chars)[1] = L'\0';

    }
    return 0;
}

//functional!
int Pfuencode(struct Pixel *dest, wchar_t *source, attr_t attr, int n, int Priority){
    int i = 0;

    for(i=0;i<n;i++){
        
        if((dest+i)->Priority <= Priority){
            if(*(source+i)==L'◆'){
                (dest+i)->Priority = 0;
                (dest+i)->Data.attr = A_NORMAL;
                ((dest+i)->Data.chars)[0] = L' ';        
            }else{
                (dest+i)->Priority = Priority;
                (dest+i)->Data.attr = attr;
                ((dest+i)->Data.chars)[0] = *(source + i);
            }
            ((dest+i)->Data.chars)[1] = L'\0';
        }
    }
    return 0;
}



int narrow2wide(wchar_t *dest, char *source, int n){
    int i = 0;
    for(i=0;i<n;i++){
        *(dest + i) = *(source + i);
    }
    return 0;
}

