/*
Option Initialize Header
*/
#ifndef OPTIONINITIALIZE_H  // Check if the macro is not defined
#define OPTIONINITIALIZE_H  // Define the macro

#include <stdio.h>
struct OptionHeader{
    int OptionCount;        // Number of options in the list
    char Title[64];         // Menu title/header text
    struct Option *List;    // Pointer to array of options
};
struct Option{
    char OptionText[32];    // Text displayed for this option
};
struct OptionHeader *InitializeOptions(char *filename);

#endif

