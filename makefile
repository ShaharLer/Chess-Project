CC = gcc

OBJS = main.o SPChessConsoleManager.o SPChessSettingState.o SPChessParser.o SPChessGame.o SPChessGameAux.o SPChessMinimax.o SPChessArrayList.o SPChessMove.o \
	 SPChessGuiManager.o SPChessMainWindow.o SPChessSettingsWindow.o SPChessGameWindow.o SPChessLoadWindow.o SPChessButton.o SPChessWidget.o
EXEC = chessprog
COMP_FLAG = -std=c99 -Wall -Wextra -Werror -pedantic-errors

SDL_COMP_FLAG = -I/usr/local/lib/sdl_2.0.5/include/SDL2 -D_REENTRANT
SDL_LIB = -L/usr/local/lib/sdl_2.0.5/lib -Wl,-rpath,/usr/local/lib/sdl_2.0.5/lib -Wl,--enable-new-dtags -lSDL2 -lSDL2main

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(COMP_FLAG) $(OBJS) $(SDL_LIB) -o $@

main.o: main.c SPChessConsoleManager.h SPChessGuiManager.h
	$(CC) $(COMP_FLAG) $(SDL_COMP_FLAG) -c $*.c
SPChessConsoleManager.o: SPChessConsoleManager.c SPChessConsoleManager.h SPChessSettingState.h
	$(CC) $(COMP_FLAG) -c $*.c
SPChessSettingState.o: SPChessSettingState.c SPChessSettingState.h SPChessMinimax.h SPChessParser.h
	$(CC) $(COMP_FLAG) -c $*.c
SPChessMinimax.o: SPChessMinimax.c SPChessMinimax.h SPChessGame.h
	$(CC) $(COMP_FLAG) -c $*.c
SPChessParser.o: SPChessParser.c SPChessParser.h SPChessGameAux.h
	$(CC) $(COMP_FLAG) -c $*.c
SPChessGame.o: SPChessGame.c SPChessGame.h SPChessGameAux.h
	$(CC) $(COMP_FLAG) -c $*.c
SPChessGameAux.o: SPChessGameAux.c SPChessGameAux.h SPChessArrayList.h
	$(CC) $(COMP_FLAG) -c $*.c
SPChessArrayList.o: SPChessArrayList.c SPChessArrayList.h SPChessMove.h
	$(CC) $(COMP_FLAG) -c $*.c
SPChessMove.o: SPChessMove.c SPChessMove.h
	$(CC) $(COMP_FLAG) -c $*.c

SPChessGuiManager.o: SPChessGuiManager.c SPChessGuiManager.h SPChessMainWindow.h SPChessSettingsWindow.h SPChessGameWindow.h SPChessLoadWindow.h
	$(CC) $(COMP_FLAG) $(SDL_COMP_FLAG) -c $*.c
SPChessMainWindow.o: SPChessMainWindow.c SPChessMainWindow.h SPChessButton.h
	$(CC) $(COMP_FLAG) $(SDL_COMP_FLAG) -c $*.c
SPChessSettingsWindow.o: SPChessSettingsWindow.c SPChessSettingsWindow.h SPChessButton.h SPChessSettingState.h
	$(CC) $(COMP_FLAG) $(SDL_COMP_FLAG) -c $*.c
SPChessGameWindow.o: SPChessGameWindow.c SPChessGameWindow.h SPChessButton.h SPChessSettingState.h
	$(CC) $(COMP_FLAG) $(SDL_COMP_FLAG) -c $*.c
SPChessLoadWindow.o: SPChessLoadWindow.c SPChessLoadWindow.h SPChessButton.h
	$(CC) $(COMP_FLAG) $(SDL_COMP_FLAG) -c $*.c
SPChessButton.o: SPChessButton.c SPChessButton.h SPChessWidget.h
	$(CC) $(COMP_FLAG) $(SDL_COMP_FLAG) -c $*.c
SPChessWidget.o: SPChessWidget.c SPChessWidget.h
	$(CC) $(COMP_FLAG) $(SDL_COMP_FLAG) -c $*.c
	
clean:
	rm -f *.o $(EXEC)