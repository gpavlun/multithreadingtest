/*
Header for Music Functions
*/
struct MusicInfo{
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


void generate_square_wave(int16_t *buffer, int samples, float frequency);
void *Music(void *);