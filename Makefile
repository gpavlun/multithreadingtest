shared: GQmenu.c SubprocessFunctions.c  WideStringFunctions.c MusicFunctions.c OptionInitializer.c TitleSequence.c
	gcc GQmenu.c SubprocessFunctions.c WideStringFunctions.c MusicFunctions.c OptionInitializer.c TitleSequence.c -o GQmenu -lm -lncursesw -lSDL2 -Wall

# gcc test.c SubprocessFunctions.c WideStringFunctions.c MusicFunctions.c OptionInitializer.c -o test -lm -lncursesw -lSDL2 -Wall
#