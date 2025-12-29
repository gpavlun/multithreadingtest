#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>      // fork(), pipe(), read(), write()
#include <string.h>      // string operations
#include <locale.h>
#include <wchar.h>
#define _XOPEN_SOURCE_EXTENDED
#include <ncurses.h>     // terminal UI rendering
#include <panel.h>       // panel support for ncurses
#include <menu.h>        // menu support for ncurses

int wstrlen(int *buffer);
int wstrcpy(int *dest,int *source);

void main(void){

    setlocale(LC_ALL, "");


    initscr();          // Initialize the curses library
    noecho();           // Don't echo user input to screen
    raw();              // Disable line buffering
    keypad(stdscr, TRUE); // Enable special keys (arrows, etc)
    curs_set(0);        // Hide the cursor
    refresh();  
    
    int maxr, maxc;
    getmaxyx(stdscr, maxr, maxc);
    int buffer[512];
    int pbuffer[512];
    FILE *BackgroundFile = fopen("background.txt","r");
    int i,j,k,l,m; 
    unsigned int r, c, size;

    fgetws(buffer,512,BackgroundFile);
    j = 0;
    while(buffer[j]!='\n'){
        j++;
    }
    buffer[j+1] = 0;
                
    int len = j;
    fclose(BackgroundFile);
    while(1){
        for(l=0;l<len;l++){
            getmaxyx(stdscr, maxr, maxc);
            i=0;
            BackgroundFile = fopen("background.txt","r");
            for(i=0;i<21;i++){
                fgetws(pbuffer,512,BackgroundFile);
                j = 0;
                k = 0;
                
                while(pbuffer[j]!='\n'){
                    j++;
                }
                pbuffer[j+1] = 0;
        
                wstrcpy(&(buffer[0]),&(pbuffer[l]));
                
                if(wstrlen(buffer)>maxc){
                    mvaddnwstr(i,0,buffer,maxc);
                }else{
                    if(wstrlen(buffer)<maxc){
                        m = 0;

                        mvaddwstr(i, 0, buffer);

                        j = maxc;
                        j -= wstrlen(buffer);
                        if(j-wstrlen(pbuffer)>0){
                            mvaddwstr(i, (wstrlen(buffer)-1) + (wstrlen(pbuffer)-1)*m, pbuffer);
                            m++;
                            j -= wstrlen(buffer);
                        }
                        mvprintw(1,1,"%d",wstrlen(buffer)-1);
                        if(maxc-((wstrlen(buffer)-1) + (wstrlen(pbuffer)-1)*m)!=0){
                            mvaddnwstr(i, (wstrlen(buffer)-1) + (wstrlen(pbuffer)-1)*(m), pbuffer, maxc - (wstrlen(buffer)-1) - (wstrlen(pbuffer)-1)*(m));
                        }
                    }else{
                        mvaddnwstr(i, 0, buffer, maxc);
                    }
                }
            } 
            fclose(BackgroundFile);
            refresh();  
            usleep(125000);
        }
    }
    endwin();
}

int wstrlen(int *buffer){
    int i = 0;
    while(buffer[i]!=0){
        i++;
    }
    return i;
}
int wstrcpy(int *dest,int *source){
    int i = 0;
    while(source[i]!=0){
        dest[i] = source[i];
        i++;
    }
    dest[i] = 0; 
    return 0;
}