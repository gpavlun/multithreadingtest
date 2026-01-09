/*
Subprocess Functions Header
*/


#ifndef SUBPROCESSFUNCTIONS_H  // Check if the macro is not defined
#define SUBPROCESSFUNCTIONS_H  // Define the macro

struct M2Dshared{
    unsigned char KillMain;
    unsigned char KillDisplay;
    unsigned char PlayMusic;
    unsigned char PlayTitle;
};
struct D2Sshared{
    unsigned char KillMusic;
    unsigned char BackStart;
    unsigned int MaxRows;
    unsigned int MaxCols;
    unsigned int *CycleState;
    cchar_t *DisplayArray;
    pthread_mutex_t lock;
    struct OptionHeader *OptionHeader;
};
struct Parameters{
    unsigned int MaxRows;
    unsigned int MaxCols;
    unsigned int MaxFRows;
    unsigned int FRow;
    unsigned int FCol;
    cchar_t *DisplayArray;
    pthread_mutex_t lock;
    char Filename[128];
};
void *Resize(void *);
void *Display(void *);
void *Background(void *);
void *Control(void *);
void *InputScan(void *);
void *TitleSequence(void *TSharedData);

void DisplayClear(cchar_t *DisplayArray, uint32_t MaxRows, uint32_t MaxCols);
void DisplayFill(cchar_t *DisplayArray, uint32_t MaxRows, uint32_t MaxCols);
void PrintFile(struct Parameters *Parameters);

#endif  // End of the include guard