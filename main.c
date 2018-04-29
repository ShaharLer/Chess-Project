#include "SPChessConsoleManager.h"
#include "SPChessGuiManager.h"

#define CONSOLE_MODE "-c"
#define GUI_MODE "-g"
#define CONSOLE_MODE_ENTERED ((argc == 2) && (strcmp(argv[1], CONSOLE_MODE) == 0))
#define GUI_MODE_ENTERED ((argc == 2) && (strcmp(argv[1], GUI_MODE) == 0))
#define GAME_MODE_NOT_ENTERED (argc == 1)
#define WRONG_GAME_MODE "ERROR: Invalid game mode was chosen"

int main(int argc, char** argv) {
	// console mode is also the default when no specific mode was entered
	if (CONSOLE_MODE_ENTERED || GAME_MODE_NOT_ENTERED)
		consoleMainLoop();
	else if (GUI_MODE_ENTERED)
		guiMainLoop();
	else
		printf(WRONG_GAME_MODE);

	return 0;
}
