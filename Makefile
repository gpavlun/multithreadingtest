shared: GQmenu.c SubprocessFunctions.c  WideStringFunctions.c MusicFunctions.c OptionInitializer.c
	gcc GQmenu.c SubprocessFunctions.c WideStringFunctions.c MusicFunctions.c OptionInitializer.c -o GQmenu -lm -lncursesw -lSDL2 -Wall

