/*
Option Initialize Source
*/
#include <unistd.h> 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "WideStringFunctions.h"

#include "OptionInitializer.h"

struct OptionHeader *InitializeOptions(char *filename){
    
    FILE *OptionsFile = fopen(filename,"r");
    if(OptionsFile==NULL){
        return NULL;
    }
    
    struct OptionHeader *OptionHeader;
    OptionHeader = (struct OptionHeader *)malloc(sizeof(struct OptionHeader));
    OptionHeader->OptionCount = 0;
    wchar_t text[32] = L"placeholder";

    struct Option *Option;
    Option = (struct Option *)malloc(sizeof(struct Option));
    OptionHeader->List = Option;
    
    while(fgetws(text,32,OptionsFile)!=NULL){
        Option = (struct Option *)realloc((void *)Option,sizeof(struct Option) * (OptionHeader->OptionCount+1));
        OptionHeader->List = Option;

        wterminate(text);
        
        wstrcpy((Option+(OptionHeader->OptionCount))->OptionText,text);
        
        OptionHeader->OptionCount++;
    }

    fclose(OptionsFile);
    return OptionHeader;
}