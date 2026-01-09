/*
Wide Character String Functions Header
*/
#define _XOPEN_SOURCE_EXTENDED
#include <ncurses.h>

int wterminate(wchar_t *);
int wstrlen(wchar_t *);
int wstrcpy(wchar_t *, wchar_t *);
int uencode(cchar_t *, wchar_t *,int );
int narrow2wide(wchar_t *, char *, int );