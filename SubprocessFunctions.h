/*
Subprocess Functions Header
*/


#ifndef SUBPROCESSFUNCTIONS_H  // Check if the macro is not defined
#define SUBPROCESSFUNCTIONS_H  // Define the macro

struct MultiShare{
    struct M2Dshared *MainData;
    struct D2Sshared *SubData;
};
struct M2Dshared{
    unsigned char KillMain;
    unsigned char KillDisplay;
    unsigned char UseRodent;
    unsigned char PlayMusic;
    unsigned char PlayTitle;
    unsigned char PlayBackground;
    unsigned int MaxRows;
    unsigned int MaxCols;
};
struct D2Sshared{
    unsigned char KillMusic;
    uint16_t KillMenu;
    unsigned char BackStart;
    unsigned char MenuStart;
    unsigned int MaxRows;
    unsigned int MaxCols;
    //unsigned int *CycleState;
    struct Pixel *DisplayArray;
    //pthread_mutex_t lock;
    struct OptionHeader *OptionHeader;
    unsigned char SelectedOption;
};
struct Parameters{
    unsigned int MaxRows;
    unsigned int MaxCols;
    unsigned int FRow;
    unsigned int FCol;
    struct Pixel *DisplayArray;
    int LineDelay;
    //pthread_mutex_t lock;
    char Filename[128];
};
struct Pixel{
    uint8_t Priority;
    cchar_t Data;
    uint8_t Cursor;
};
struct RodentData{
    struct D2Sshared *SharedData;
    cchar_t Cursor;
    char *Paths;
};


void *Resize(void *);
void *Display(void *);
void *Rodent(void *TRodentData);
void *Background(void *);
void *Control(void *);
void *InputScan(void *);
void *TitleSequence(void *TSharedData);
void *Menu(void *TShared);

void DisplayClear(struct Pixel *DisplayArray, uint32_t MaxRows, uint32_t MaxCols);
void DisplayFill(struct Pixel *DisplayArray, uint32_t MaxRows, uint32_t MaxCols);
void PrintFile(struct Parameters *Parameters, int Priority);
void PrintBackdrop(struct Parameters *Parameters);
int getrows(char *Name);
int getcols(char *Name);

#endif  // End of the include guard