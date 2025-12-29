// Includes for system calls, standard library, and ncurses UI library
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>      // fork(), pipe(), read(), write()
#include <string.h>      // string operations
#include <signal.h>      // signal handling
#include <poll.h>        // poll() for non-blocking I/O
#define _XOPEN_SOURCE_EXTENDED
#include <ncurses.h>     // terminal UI rendering
#include <panel.h>       // panel support for ncurses
#include <menu.h>        // menu support for ncurses
#include <fcntl.h>       // file control (O_NONBLOCK)
#include <sys/wait.h>    // waitpid() for process management
#include <locale.h>
#include <wchar.h>

// Data structure for menu options container
// Holds the menu title and a list of available options
struct OptionHeader{
    int OptionCount;        // Number of options in the list
    char Title[64];         // Menu title/header text
    struct Option *List;    // Pointer to array of options
};

// Individual menu option
struct Option{
    char OptionText[32];    // Text displayed for this option
};

// Function declarations
void Control(struct OptionHeader *OptionHeader, int WP);
void Background(int WP);
void Display(void);
void MainMenu(void);
int InputScan(void);
int wstrlen(int *buffer);
int wstrcpy(int *dest,int *source);
struct OptionHeader *InitializeOptions(FILE *OptionsFile);
void IOProcess(int WP);

// Entry point - initiates the menu system
void main(void){
    MainMenu();
}

// MainMenu: Forks the display process and waits for it to complete
// The display process handles all ncurses rendering and spawns background/control subprocesses
void MainMenu(void){
    // Fork a child process for the display subroutine
    int DisplayID = fork();
    if(DisplayID==0){
        // Child process: run the display handler
        Display();
        // Kill child after display exits
        kill(getpid(),SIGKILL);
    }
    
    // Parent process: wait for display child to finish
    int stats;
    waitpid(DisplayID, &stats, 0);
}
// Display: Main ncurses rendering loop
// Initializes the terminal UI and spawns background and control processes
// Reads from pipes and renders all updates to the screen
void Display(void){
    setlocale(LC_ALL, "");
    //wchar_t block[] = L"\u2588";
    // Initialize ncurses screen
    initscr();          // Initialize the curses library
    noecho();           // Don't echo user input to screen
    raw();              // Disable line buffering
    keypad(stdscr, TRUE); // Enable special keys (arrows, etc)
    curs_set(0);        // Hide the cursor
    
    clear();            // Clear the screen
    refresh();          // Update the display
    usleep(200000);          // Brief delay for ncurses initialization

    int flags;

    // === Create Background Process & Pipe ===
    // This pipe carries updates from the background process to display
    int BackgroundPipe[2];
    pipe(BackgroundPipe);

    // Set both ends of background pipe to non-blocking mode
    // This prevents the display from hanging if no data is available
    flags = fcntl(BackgroundPipe[0], F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(BackgroundPipe[0], F_SETFL, flags);
    flags = fcntl(BackgroundPipe[1], F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(BackgroundPipe[1], F_SETFL, flags);

    // Fork background process
    int BackgroundID = fork();
    if(BackgroundID==0){
        // Child: run background task
        Background(BackgroundPipe[1]);
        kill(getpid(),SIGKILL);
    }
    // Parent continues with display setup

    // === Load Menu Options from File ===
    // Read options.txt to populate the menu structure
    FILE *OptionsFile = fopen("options.txt","r");
    if(OptionsFile==NULL){
        // Exit if options file not found
        exit(0);
    }
 
    // Parse and initialize menu options
    struct OptionHeader *OptionHeader;
    OptionHeader = InitializeOptions(OptionsFile);
    fclose(OptionsFile);

    // === Create Control Process & Pipe ===
    // This pipe carries menu updates from the control process to display
    int ControlPipe[2];
    pipe(ControlPipe);

    // Set control pipe to non-blocking mode
    flags = fcntl(ControlPipe[0], F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(ControlPipe[0], F_SETFL, flags);
    flags = fcntl(ControlPipe[1], F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(ControlPipe[1], F_SETFL, flags);

    // Fork control process - handles user input
    int ControlID = fork();
    if(ControlID==0){
        // Child: handle menu control and input
        Control(OptionHeader, ControlPipe[1]);
        kill(getpid(),SIGKILL);
    }
    // Parent continues with display rendering loop


    char buffer[512];  // Temporary buffer for pipe data
    int wbuffer[512];
    unsigned int r, c, size;  // Row, column, message size
    int err;            // Read error status

    int maxr,maxc;      // Terminal dimensions
    int i = 0;
    
    // === Draw Menu Box Once (Static) ===
    // Calculate menu position once
    getmaxyx(stdscr, maxr, maxc);
    r = maxr * 1/3;   // Menu positioned at 1/3 down the screen
    c = maxc * .5;    // Horizontally centered


    // Draw menu box border (only once)
    mvwhline(stdscr, r - 3, c - (strlen(OptionHeader->Title)/2) - 3, ACS_HLINE, strlen(OptionHeader->Title) + 4);
    mvwhline(stdscr, r + 2 + 4 , c - (strlen(OptionHeader->Title)/2) - 3, ACS_HLINE, strlen(OptionHeader->Title) + 4);
    mvwvline(stdscr, r - 3, c - (strlen(OptionHeader->Title)/2) - 3, ACS_VLINE, 4 + 6);
    mvwvline(stdscr, r - 3, c - (strlen(OptionHeader->Title)/2) + strlen(OptionHeader->Title) + 1, ACS_VLINE, 4 + 6);
    mvwhline(stdscr, r - 3, c - (strlen(OptionHeader->Title)/2) - 3, ACS_ULCORNER, 1);
    mvwhline(stdscr, r - 3, c + (strlen(OptionHeader->Title)/2) + 2, ACS_URCORNER, 1);
    mvwhline(stdscr, r + 6, c - (strlen(OptionHeader->Title)/2) - 3, ACS_LLCORNER, 1);
    mvwhline(stdscr, r + 6, c + (strlen(OptionHeader->Title)/2) + 2, ACS_LRCORNER, 1);
    
    // Draw menu header (title, "options", separator)
    mvwprintw(stdscr, r - 2, c - (strlen(OptionHeader->Title)/2), "%s", OptionHeader->Title);
    mvwprintw(stdscr, r , c - (strlen("options")/2), "options");
    mvwprintw(stdscr, r + 1, c - (strlen("=======")/2), "=======");
    
    // === Main Display Loop ===
    // Continuously reads from background and control pipes, renders to screen
    while(1){
        size = 0;
        err = 0;
        
        // Read and render all pending background messages
        // Message format: [size] [row] [col] [string...]
        // size=0xAAAAAAAA is sentinel (no more messages), size=0xBBBBBBBB is exit signal
        while(err!=EOF&&size!=0xAAAAAAAA){
            err = read(BackgroundPipe[0],&size,sizeof(int));
            if(size==0xBBBBBBBB){
                // Exit signal received
                goto end;
            }
            if(err!=EOF&&size!=0xAAAAAAAA){
                // Read message coordinates and text
                read(BackgroundPipe[0],&r,sizeof(int));
                read(BackgroundPipe[0],&c,sizeof(int));

                read(BackgroundPipe[0],wbuffer,size*4);
                // Render text at specified screen position
                mvwaddnwstr(stdscr,r,c,wbuffer,size);

            }
        }
        size = 0;
        err = 0;
     
        // Get terminal dimensions and calculate menu center position
        getmaxyx(stdscr, maxr, maxc);
        r = maxr * 1/3;   // Menu positioned at 1/3 down the screen
        c = maxc * .5;    // Horizontally centered

        // Draw menu box border using ncurses line drawing characters
        mvwhline(stdscr, r - 3, c - (strlen(OptionHeader->Title)/2) - 3, ACS_HLINE, strlen(OptionHeader->Title) + 4);
        mvwhline(stdscr, r + 2 + 4 , c - (strlen(OptionHeader->Title)/2) - 3, ACS_HLINE, strlen(OptionHeader->Title) + 4);
        mvwvline(stdscr, r - 3, c - (strlen(OptionHeader->Title)/2) - 3, ACS_VLINE, 4 + 6);
        mvwvline(stdscr, r - 3, c - (strlen(OptionHeader->Title)/2) + strlen(OptionHeader->Title) + 1, ACS_VLINE, 4 + 6);
        mvwhline(stdscr, r - 3, c - (strlen(OptionHeader->Title)/2) - 3, ACS_ULCORNER, 1);
        mvwhline(stdscr, r - 3, c + (strlen(OptionHeader->Title)/2) + 2, ACS_URCORNER, 1);
        mvwhline(stdscr, r + 6, c - (strlen(OptionHeader->Title)/2) - 3, ACS_LLCORNER, 1);
        mvwhline(stdscr, r + 6, c + (strlen(OptionHeader->Title)/2) + 2, ACS_LRCORNER, 1);
        
        // Draw menu header (title, "options", separator)
        mvwprintw(stdscr, r - 2, c - (strlen(OptionHeader->Title)/2), "%s", OptionHeader->Title);
        mvwprintw(stdscr, r , c - (strlen("options")/2), "options");
        mvwprintw(stdscr, r + 1, c - (strlen("=======")/2), "=======");

        // Read and render all pending control/menu messages
        // Same protocol: [size] [row] [col] [string...]
        while(err!=EOF&&size!=0xAAAAAAAA){
            err = read(ControlPipe[0],&size,sizeof(int));
            if(size==0xBBBBBBBB){
                // Exit signal received
                goto end;
            }
            if(err!=EOF&&size!=0xAAAAAAAA){
                // Read message coordinates and menu text
                read(ControlPipe[0],&r,sizeof(int));
                read(ControlPipe[0],&c,sizeof(int));
                read(ControlPipe[0],buffer,size);
                //buffer[size-1] = '\0';
                // Render menu option at specified position
                //mvwprintw(stdscr,r,c,"%s",buffer);
                mvaddnstr(r,c,buffer,size);
            }
        }
        // Small delay to prevent excessive CPU usage
        usleep(100);
        // Update the screen with all pending changes
        refresh();
    }
    // === Cleanup ===
    end:
    kill(BackgroundID,SIGKILL);  // Terminate background process
    kill(ControlID,SIGKILL);     // Terminate control process
    endwin();                     // Close ncurses and restore terminal
}
// Background: Periodic background task
// Alternates between displaying "yes" and "no" at 1 second intervals
// Demonstrates inter-process communication via the pipe
void Background(int WP){
    setlocale(LC_ALL, "");

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
                    size = maxc;
                    r = i;  
                    c = 0;  // Column 1
                    
                    // Send message via pipe: [size] [row] [col] [text]
                    write(WP,&size,sizeof(int));
                    write(WP,&r,sizeof(int));
                    write(WP,&c,sizeof(int));
                    write(WP,buffer,size*sizeof(int));
                    
                    //mvaddnwstr(i,0,buffer,maxc);
                }else{
                    if(wstrlen(buffer)<maxc){
                        m = 0;

                        size = wstrlen(buffer) + 1;
                        r = i;  
                        c = 0;  // Column 1
                        
                        // Send message via pipe: [size] [row] [col] [text]
                        write(WP,&size,sizeof(int));
                        write(WP,&r,sizeof(int));
                        write(WP,&c,sizeof(int));
                        write(WP,buffer,size*sizeof(int));
                    
                        //mvaddwstr(i, 0, buffer);

                        j = maxc;
                        j -= wstrlen(buffer);
                        if(j-wstrlen(pbuffer)>0){
                            
                            size = wstrlen(pbuffer) + 1;
                            r = i;  
                            c = (wstrlen(buffer)-1) + (wstrlen(pbuffer)-1)*m;
                            
                            // Send message via pipe: [size] [row] [col] [text]
                            write(WP,&size,sizeof(int));
                            write(WP,&r,sizeof(int));
                            write(WP,&c,sizeof(int));
                            write(WP,pbuffer,size*sizeof(int));
                            
                            //mvaddwstr(i, (wstrlen(buffer)-1) + (wstrlen(pbuffer)-1)*m, pbuffer);
                            m++;
                            j -= wstrlen(buffer);
                        }
                        //mvprintw(1,1,"%d",wstrlen(buffer)-1);
                        if(maxc-((wstrlen(buffer)-1) + (wstrlen(pbuffer)-1)*m)!=0){
                            size = maxc - (wstrlen(buffer)-1) - (wstrlen(pbuffer)-1)*(m);
                            r = i;  
                            c = (wstrlen(buffer)-1) + (wstrlen(pbuffer)-1)*(m);
                            
                            // Send message via pipe: [size] [row] [col] [text]
                            write(WP,&size,sizeof(int));
                            write(WP,&r,sizeof(int));
                            write(WP,&c,sizeof(int));
                            write(WP,pbuffer,size*sizeof(int));

                            //mvaddnwstr(i, (wstrlen(buffer)-1) + (wstrlen(pbuffer)-1)*(m), pbuffer, maxc - (wstrlen(buffer)-1) - (wstrlen(pbuffer)-1)*(m));
                        }
                    }else{
                        size = maxc;
                        r = i;  
                        c = 0;
                        
                        // Send message via pipe: [size] [row] [col] [text]
                        write(WP,&size,sizeof(int));
                        write(WP,&r,sizeof(int));
                        write(WP,&c,sizeof(int));
                        write(WP,buffer,size*sizeof(int));
                        //mvaddnwstr(i, 0, buffer, maxc);
                    }

                }
            } 
            fclose(BackgroundFile);
            // Send sentinel value (0xAAAAAAAA = end of message batch)
            size = 0xAAAAAAAA;
            write(WP,&size,sizeof(int));            
            //refresh();  
            usleep(125000);
        }
    }
    endwin();
}
// Control: User input handler and menu management
// Reads keyboard input and sends menu updates via pipe
// Tracks selected option and sends formatted menu text to display
void Control(struct OptionHeader *OptionHeader, int WP){
    // Pipe communication for sending menu updates to display process
    unsigned int r, c, size;    // Message: row, column, size
    char buffer[512];           // Message text buffer
    int SelectedOption = 0;     // Currently highlighted option index
    int ioscan = 1;             // Input scan result
    int i;
    int maxr,maxc;              // Terminal dimensions
    int tr, tc;                 // Temporary row/column for menu items


    int flags;
    int IOPipe[2];
    pipe(IOPipe);

    // Set IO pipe to non-blocking mode
    flags = fcntl(IOPipe[0], F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(IOPipe[0], F_SETFL, flags);
    flags = fcntl(IOPipe[1], F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(IOPipe[1], F_SETFL, flags);

    // Fork IO process - handles user input
    int IOID = fork();
    if(IOID==0){
        // Child: handle user input
        IOProcess(IOPipe[1]);
        kill(getpid(),SIGKILL);
    }

    // === Main Control Loop ===
    // Runs until user selects exit option (ioscan becomes 0)
    while(ioscan!=0){
        // Draw all menu options, highlighting the selected one
        for(i=0;i<(OptionHeader->OptionCount)-1;i++){
            // Calculate position for current menu item
            getmaxyx(stdscr, maxr, maxc);
            r = maxr * 1/3 + 2;  // Menu starts below title
            c = maxc * .5;        // Centered horizontally
            tr = r + i;           // One line per option
            tc = c - (strlen(((OptionHeader->List)+i)->OptionText)/2) - 1;  // Left bracket position
            
            // === Send left bracket ([ or space) ===
            size = 2;
            if(i==SelectedOption){
                // Highlight selected option with brackets
                strcpy(buffer,"[");
                write(WP,&size,sizeof(int));
                write(WP,&tr,sizeof(int));
                write(WP,&tc,sizeof(int));
                write(WP,buffer,size);
            }else{
                // Non-selected options get space
                strcpy(buffer," ");
                write(WP,&size,sizeof(int));
                write(WP,&tr,sizeof(int));
                write(WP,&tc,sizeof(int));
                write(WP,buffer,size);
            }

            // === Send option text ===
            tr = r + i;
            tc = c - (strlen(((OptionHeader->List)+i)->OptionText)/2);
            strcpy(buffer,((OptionHeader->List)+i)->OptionText);
            size = strlen(buffer) + 1;
            write(WP,&size,sizeof(int));
            write(WP,&tr,sizeof(int));
            write(WP,&tc,sizeof(int));
            write(WP,buffer,size);

            // === Send right bracket (] or space) ===
            tr = r + i;
            tc = c + (strlen(((OptionHeader->List)+i)->OptionText)/2);
            size = 2;
            if(i==SelectedOption){
                strcpy(buffer,"]");
                write(WP,&size,sizeof(int));
                write(WP,&tr,sizeof(int));
                write(WP,&tc,sizeof(int));
                write(WP,buffer,size);
            }else{
                strcpy(buffer," ");
                write(WP,&size,sizeof(int));
                write(WP,&tr,sizeof(int));
                write(WP,&tc,sizeof(int));
                write(WP,buffer,size);
            }    
        }
        // Send sentinel to mark end of menu updates
        size = 0xAAAAAAAA;
        write(WP,&size,sizeof(int));

        ioscan = 3;
        // Get next user input
        read(IOPipe[0], &ioscan, sizeof(int));
        //printf("%d\n",ioscan);
        //ioscan = InputScan();
        switch(ioscan){
        case 1:
            // UP arrow: move to previous option
            if(SelectedOption>0) SelectedOption--;
            break;
        case 2:
            // DOWN arrow: move to next option
            if(SelectedOption<((OptionHeader->OptionCount)-2)) SelectedOption++;
            break;
        case 5:
            // ENTER key: process selection
            switch(SelectedOption){
            case 3:
                // Option 3 is the exit option
                ioscan = 0;  // Exit main loop
                // Send exit signal (size=0xBBBBBBBB) to all processes
                r = maxr * .5;
                c = maxc * .5;
                size = 0xBBBBBBBB;
                write(WP,&size,sizeof(int));
                break;
            default:
                // Other options: send selection message
                r = 1;
                c = 1;
                snprintf(buffer, sizeof(buffer), "option %d selected",SelectedOption+1);
                size = strlen(buffer) + 1;
                write(WP,&size,sizeof(int));
                write(WP,&r,sizeof(int));
                write(WP,&c,sizeof(int));
                write(WP,buffer,size);
                break;
            }
            break;
        }
        usleep(1000);
    }
}
// InitializeOptions: Parse menu options from file
// Reads options.txt and populates the OptionHeader structure
// File format: First line = title, subsequent lines = menu options
struct OptionHeader *InitializeOptions(FILE *OptionsFile){
    // Allocate and initialize the menu container
    struct OptionHeader *OptionHeader;
    OptionHeader = (struct OptionHeader *)malloc(sizeof(struct OptionHeader));
    OptionHeader->OptionCount = 0;
    char text[64] = "placeholder";  // Buffer for reading lines from file
    int i;
    
    // === Read Menu Title ===
    if(fgets(OptionHeader->Title,64,OptionsFile)==NULL){
        // Fallback if file is empty
        strncpy(OptionHeader->Title,"not found",63);
        OptionHeader->Title[63] = '\0';
    }else{
        // Remove newline character from title
        i = 0;
        while((OptionHeader->Title)[i]!='\n' && i < 63){
            i++;
        }
        (OptionHeader->Title)[i] = '\0';
    }
    
    // === Read Menu Options ===
    if(text!=NULL){
        // Allocate first option
        struct Option *Option;
        Option = (struct Option *)malloc(sizeof(struct Option));
        OptionHeader->List = Option;
        
        // Read first option from file
        fgets(text,64,OptionsFile);
        strncpy(Option->OptionText,text,31);
        Option->OptionText[31] = '\0';
        
        // Remove newline if present
        i = 0;
        int len;
        len = strlen(Option->OptionText);
        if(len > 0 && (Option->OptionText)[len-1] == '\n'){
            (Option->OptionText)[len-1] = '\0';
        }
        OptionHeader->OptionCount++;
        
        // Read remaining options and dynamically grow the array
        while(fgets(text,64,OptionsFile)!=NULL){
            // Reallocate array to accommodate new option
            Option = (struct Option *)realloc((void *)Option,sizeof(struct Option) * (OptionHeader->OptionCount+1));
            OptionHeader->List = Option;
            
            // Copy option text into new slot
            strncpy((Option+(OptionHeader->OptionCount))->OptionText,text,31);
            (Option+(OptionHeader->OptionCount))->OptionText[31] = '\0';
            
            // Remove newline from option text
            len = strlen((Option+(OptionHeader->OptionCount))->OptionText);
            if(len > 0 && ((Option+(OptionHeader->OptionCount))->OptionText)[len-1] == '\n'){
                ((Option+(OptionHeader->OptionCount))->OptionText)[len-1] = '\0';
            }
            OptionHeader->OptionCount++;
        }
    }
    // Return populated menu structure
    return OptionHeader;
}
// InputScan: Non-blocking keyboard input handler
// Flushes input queue then waits for next valid input (UP, DOWN, ENTER)
// Returns: 1=UP, 2=DOWN, 5=ENTER, -1=invalid
int InputScan(void){
    // === Flush Input Queue ===
    // Clear any buffered keystrokes before waiting for fresh input
    struct pollfd pfd;
    pfd.fd = STDIN_FILENO;
    pfd.events = POLLIN; 
    char c[255];
    int flag = 0;
    int ret;
    
    // Non-blocking loop: drain all pending input
    while(flag==0){
        int ret = poll(&pfd, 1, 0);  // Check for pending input (timeout=0)
        if (ret > 0 && (pfd.revents & POLLIN)){
            // Input available: read and discard
            read(STDIN_FILENO, c, 255);
        }else{
            // No more input available
            flag = 1;
        }
    }
    
    // === Wait for Valid Input ===
    // Block until user presses UP, DOWN, or ENTER
    int returnvalue = -1;
    int userinput = 0;
    while(returnvalue==-1){ 
        if(userinput==KEY_UP){
            // Up arrow: return 1
            return 1;
        }else if(userinput==KEY_DOWN){
            // Down arrow: return 2
            return 2;
        }else if(userinput==10){
            // Enter key (ASCII 10): return 5
            return 5;
        }else{
            // Get next keystroke from ncurses
            userinput = getch();
        }
    }
    return returnvalue;
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
void IOProcess(int WP){
    int ioscan;
    while(1){
        ioscan = InputScan();
        write(WP, &ioscan, sizeof(int));
    }
    
}