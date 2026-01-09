/*
Wide Character String Functions Source
*/
#define _XOPEN_SOURCE_EXTENDED
#include <ncurses.h>


#include "WideStringFunctions.h"

int wterminate(wchar_t *buffer){
    int i = 0;
    while(buffer[i]!='\n'){
        i++;
    }
    buffer[i] = 0;
    return i-1;
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
int uencode(cchar_t *dest, wchar_t *source, int n){
    int i = 0;

    for(i=0;i<n;i++){
        ((dest+i)->chars)[0] = *(source + i);
        ((dest+i)->chars)[1] = L'\0';
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

