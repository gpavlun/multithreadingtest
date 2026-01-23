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


#include <fcntl.h>
#include <linux/input.h>

#include <errno.h>


int main(void)
{
    setlocale(LC_ALL, "");  
    initscr();              
    noecho();               
    raw();                  
    keypad(stdscr, TRUE);   
    curs_set(0);

    unsigned int MaxRows,MaxCols;
    getmaxyx(stdscr,MaxRows,MaxCols);
    float x = MaxCols/2;
    float y = MaxRows/2;
    float tempev;
    float oldx = x;
    float oldy = y;
    char curser = '+';


    int fd = open("/dev/input/event7", O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct input_event ev;

    while (1) {
        read(fd, &ev, sizeof(ev));

        /* Mouse movement */
        if (ev.type == EV_REL && ev.code == REL_X){
            mvprintw(2,1,"dx = %d\n", ev.value);
            tempev = ev.value;
            
            x+=tempev/6;
            
            
        }
        if (ev.type == EV_REL && ev.code == REL_Y){
            mvprintw(3,1,"dy = %d\n", ev.value);
            tempev = ev.value;
            
            y+=tempev/6;
        }

        mvprintw(1,1,"(%f,%f)", y,x);



        if(y<0){
            y=0;
        }else if(y>=MaxRows){
            y=MaxRows-1;
        }
        if(x<0){
            x=0;
        }else if(x>=MaxCols){
            x=MaxCols-1;
        }
        //clear();
        mvprintw(oldy,oldx," ");
        mvprintw(y,x,"%c",curser);
        oldx = x;
        oldy = y;
        

        /* Mouse clicks */
        if (ev.type == EV_KEY) {
            if (ev.code == BTN_LEFT){
                mvprintw(3,1,"Left %s\n", ev.value ? "down" : "up");
                if(ev.value){
                    curser = 'x';
                }else{
                    curser = '+';
                }
            }
            if (ev.code == BTN_RIGHT){
                mvprintw(4,1,"Right %s\n", ev.value ? "down" : "up");
                goto end;
            }


            if (ev.code == BTN_MIDDLE)
                mvprintw(5,1,"Middle %s\n", ev.value ? "down" : "up");
        }
        refresh();
    }
    end:
    endwin();
}
