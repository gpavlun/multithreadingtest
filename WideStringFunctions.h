/*
Wide Character String Functions Header
*/
#define _XOPEN_SOURCE_EXTENDED
#include <ncurses.h>
#include "SubprocessFunctions.h"

int wterminate(wchar_t *wstring);
int wstrlen(wchar_t *Wstring);
int wstrcpy(wchar_t *dest, wchar_t *source);
int uencode(struct Pixel *dest, wchar_t *source, int n);
int fuencode(struct Pixel *dest, wchar_t *source, attr_t attr, int n);
int Pfuencode(struct Pixel *dest, wchar_t *source, attr_t attr, int n, int Priority);
int narrow2wide(wchar_t *dest, char *src, int n);