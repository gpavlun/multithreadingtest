/*
Option Initialize Source
*/
#include <unistd.h> 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "OptionInitializer.h"

struct OptionHeader *InitializeOptions(char *filename){
    
    FILE *OptionsFile = fopen(filename,"r");
    if(OptionsFile==NULL){
        return NULL;
    }
    
    struct OptionHeader *OptionHeader;
    OptionHeader = (struct OptionHeader *)malloc(sizeof(struct OptionHeader));
    OptionHeader->OptionCount = 0;
    char text[64] = "placeholder";
    int i;
    
    // === Read Menu Title ===
    if(fgets(OptionHeader->Title,64,OptionsFile)==NULL){
        strncpy(OptionHeader->Title,"not found",63);
        OptionHeader->Title[63] = '\0';
    }else{
        i = 0;
        while((OptionHeader->Title)[i]!='\n' && i < 63){
            i++;
        }
        (OptionHeader->Title)[i] = '\0';
    }
    
    // === Read Menu Options ===

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
    
    fclose(OptionsFile);
    // Return populated menu structure
    return OptionHeader;
}