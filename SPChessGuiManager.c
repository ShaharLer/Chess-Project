#include "SPChessGuiManager.h"

/**
 * This function is the main loop of the gui mode.
 * It does all that needed to starting/finishing the gui mode correctly,
 * and send the events to be handled when they accure.
 */
void guiMainLoop() {
	setvbuf(stdout,NULL,_IONBF,0);
	setvbuf(stderr,NULL,_IONBF,0);
	setvbuf(stdin,NULL,_IONBF,0);

	// SDL2 Initialization
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf(SDL_INIT_ERROR, SDL_GetError());
		return;
	}

	SPGuiManager* manager = guiManagerCreate();
	if (manager == NULL ) {
		SDL_Quit();
		return;
	}

	SDL_Event sdlEvent;
	while (1) {
		SDL_WaitEvent(&sdlEvent);
		SP_MANAGER_EVENT managerEvent = managerHandleEvent(manager, &sdlEvent);

		if (managerEvent == SP_MANAGER_QUIT)
			break;

		if (managerEvent == SP_MANAGER_UPDATE)
			guiManagerDraw(manager);
	}

	guiManagerDestroy(manager);
	manager = NULL;
	SDL_Quit();
}

/**
 * This function creates a new GuiManager structure.
 *
 * @return
 * 		NULL      - if there was a memory error.
 * 		Otherwise - retuerns the manager.
 */
SPGuiManager* guiManagerCreate() {
	SPGuiManager* manager = (SPGuiManager*) malloc(sizeof(SPGuiManager));

	if (manager == NULL)
		return NULL;

	manager->mainWindow = mainWindowCreate();

	if(manager->mainWindow == NULL) {
		printf(MAIN_WINDOW_CREATION_ERROR);
		free(manager);
		manager = NULL;
		return NULL;
	}

	manager->settingsWindow = NULL;
	manager->loadWindow     = NULL;
	manager->gameWindow     = NULL;
	manager->activeWindow   = SP_MAIN_WINDOW_ACTIVE;

	return manager;
}

/**
 * The fucnction draws the relevat window, i.e. the window that is currently active.
 *
 * @param manager - The gui manager
 */
void guiManagerDraw(SPGuiManager* manager) {
	if(manager == NULL)
		return;

	switch (manager->activeWindow) {
		case SP_MAIN_WINDOW_ACTIVE:
			mainWindowDraw(manager->mainWindow);
			break;

		case SP_SETTINGS_WINDOW_ACTIVE:
			settingsWindowDraw(manager->settingsWindow);
			break;

		case SP_GAME_WINDOW_ACTIVE:
			gameWindowDraw(manager->gameWindow, true);
			break;

		case SP_LOAD_WINDOW_ACTIVE:
			loadWindowDraw(manager->loadWindow);
	}
}

/**
 * The function handles the relevant SDL event that is received by
 * calling the relevant helper function, according to the window that is currently active.
 *
 * @param manager - The gui manager
 * @param event   - The SDL_Event event that occurred
 *
 * @return
 * 		SP_MANAGER_NONE - If either the received manager is NULL or the the SDL event is NULL.
 *		Otherwise       - Returns the result that came back from the relevant helper function that was called.
 */
SP_MANAGER_EVENT managerHandleEvent(SPGuiManager* manager, SDL_Event* event) {
	if ((manager == NULL) || (event == NULL))
		return SP_MANAGER_NONE;

	switch (manager->activeWindow) {
		case SP_MAIN_WINDOW_ACTIVE:

			if (manager->mainWindow->firstCreated) {
				manager->mainWindow->firstCreated = false;
				guiManagerDraw(manager);
			}

			return handleManagerDueToMainEvent(manager, mainWindowHandleEvent(manager->mainWindow, event));

		case SP_SETTINGS_WINDOW_ACTIVE:

			return handleManagerDueToSettingsEvent(manager, settingsWindowHandleEvent(manager->settingsWindow, event));

		case SP_GAME_WINDOW_ACTIVE:

			if (manager->gameWindow->firstCreated) {
				manager->gameWindow->firstCreated = false;

				SPGameWindow* gameWindow = manager->gameWindow;
				if(gameWindow->settings->userColor == BLACK_PLAYER) {
					gameWindow->gameIsSaved = false;
					if (executeCompterMove(gameWindow) == SP_GAME_EVENT_QUIT) { // execute computer move
						gameWindowDestroy(manager->gameWindow);
						manager->gameWindow = NULL;
						return SP_MANAGER_QUIT;
					}
				}

				guiManagerDraw(manager);
			}

			return handleManagerDueToGameEvent(manager, gameWindowHandleEvent(manager->gameWindow, event));

		case SP_LOAD_WINDOW_ACTIVE:

			return handleManagerDueToLoadEvent(manager, loadWindowHandleEvent(manager->loadWindow, event));

		default:
			return SP_MANAGER_NONE;
	}
}

/**
 * The function handles the program according to the result that came back
 * after the event that occurred in the Main window was handled.
 *
 * @param manager - The gui manager
 * @param event   - The SP_MAIN_EVENT event that indicates what action that the gui manager needs to take
 *  				 after the Main window's event was handled.
 *
 * @return
 * 		SP_MANAGER_UPDATE - If either the "New Game" button or the "Load" button were clicked.
 *		SP_MANAGER_QUIT   - If the "Exit" button was clicked or if the gui program need to be terminated due to a memory error.
 *		SP_MANAGER_NONE   - Otherwise.
 */
SP_MANAGER_EVENT handleManagerDueToMainEvent(SPGuiManager* manager, SP_MAIN_EVENT event) {
	if (manager == NULL)
		return SP_MANAGER_NONE;

	switch (event) {
		case SP_MAIN_START:
			SDL_HideWindow(manager->mainWindow->window);
			manager->settingsWindow = settingsWindowCreate();

			if (manager->settingsWindow == NULL) {
				printf(SETTINGS_WINDOW_CREATION_ERROR);
				return SP_MANAGER_QUIT;
			}

			manager->activeWindow = SP_SETTINGS_WINDOW_ACTIVE;
			return SP_MANAGER_UPDATE;

		case SP_MAIN_LOAD:
			SDL_HideWindow(manager->mainWindow->window);
			manager->loadWindow = loadWindowCreate();

			if (manager->loadWindow == NULL) {
				printf(LOAD_WINDOW_CREATION_ERROR);
				return SP_MANAGER_QUIT;
			}

			manager->activeWindow = SP_LOAD_WINDOW_ACTIVE;
			return SP_MANAGER_UPDATE;

		case SP_MAIN_EXIT:
			return SP_MANAGER_QUIT;

		default:
			return SP_MANAGER_NONE;
	}
}

/**
 * The function handles the program according to the result that came back
 * after the event that occurred in the Settings window was handled.
 *
 * @param manager - The gui manager
 * @param event   - The SP_SETTINGS_EVENT event that indicates what action that the gui manager needs to take
 *  				 after the Settings window's event was handled.
 *
 * @return
 * 		SP_MANAGER_UPDATE - If the Settings window needs to be updated.
 *		SP_MANAGER_QUIT   - If the gui program need to be terminated due to a memory error.
 *		SP_MANAGER_NONE   - Otherwise.
 */
SP_MANAGER_EVENT handleManagerDueToSettingsEvent(SPGuiManager* manager, SP_SETTINGS_EVENT event) {
	if (manager == NULL)
		return SP_MANAGER_NONE;

	GameSetting* settings = NULL; // declared here to prevent logical error the appears inside the "switch-case"

	switch (event) {
		case SP_SETTINGS_EVENT_UPDATE:
			return SP_MANAGER_UPDATE;

		case SP_SETTINGS_EVENT_START:
			settings = copySettings(manager->settingsWindow->settings, false);
			settingsWindowDestroy(manager->settingsWindow);
			manager->settingsWindow = NULL;

			if (settings == NULL) {
				printf(GAME_WINDOW_CREATION_ERROR);
				return SP_MANAGER_QUIT;
			}

			settings->game = spChessGameCreate(HISTORY_SIZE);
			if (settings->game == NULL) {
				printf(GAME_WINDOW_CREATION_ERROR);
				destroySettings(settings);
				settings = NULL;
				return SP_MANAGER_QUIT;
			}

			manager->gameWindow = gameWindowCreate(settings);
			if (manager->gameWindow == NULL) {
				printf(GAME_WINDOW_CREATION_ERROR);
				destroySettings(settings);
				settings = NULL;
				return SP_MANAGER_QUIT;
			}

			manager->activeWindow = SP_GAME_WINDOW_ACTIVE;
			return SP_MANAGER_UPDATE;

		case SP_SETTINGS_EVENT_BACK:
			settingsWindowDestroy(manager->settingsWindow); // going back to the Main window
			manager->settingsWindow = NULL;
			SDL_ShowWindow(manager->mainWindow->window);

			manager->activeWindow = SP_MAIN_WINDOW_ACTIVE;
			return SP_MANAGER_UPDATE;

		case SP_SETTINGS_EVENT_EXIT:
			settingsWindowDestroy(manager->settingsWindow);
			manager->settingsWindow = NULL;
			return SP_MANAGER_QUIT;

		default:
			return SP_MANAGER_NONE;
	}
}

/**
 * The function handles the program according to the result that came back
 * after the event that occurred in the Load window was handled.
 *
 * @param manager - The gui manager
 * @param event   - The SP_LOAD_EVENT event that indicates what action that the gui manager needs to take
 *  				 after the Load window's event was handled.
 *
 * @return
 * 		SP_MANAGER_UPDATE - If either the Game window need to be refreshed (because a new slot was chosen),
 * 		                      or if the "Back" button was clicked.
 *		SP_MANAGER_QUIT   - If the "X" button (at the top corner of the screen) was clicked.
 *		SP_MANAGER_NONE   - If no refresh needs to be done to the screen.
 *
 *		If the "Load" button was clicked from inside the Load window - returns the result
 * 		that came back from the helper function handleExitFromTheGame.
 */
SP_MANAGER_EVENT handleManagerDueToLoadEvent(SPGuiManager* manager, SP_LOAD_EVENT event) {
	if (manager == NULL)
		return SP_MANAGER_NONE;

	SP_MANAGER_EVENT result; // in order to prevent logical error in the "switch-case"

	switch (event) {
		case SP_LOAD_UPDATE:
			return SP_MANAGER_UPDATE;

		case SP_LOAD_BACK:
			loadWindowDestroy(manager->loadWindow);
			manager->loadWindow = NULL;

			if (manager->gameWindow != NULL) { // back to the Game window
				SDL_ShowWindow(manager->gameWindow->window);
				manager->activeWindow = SP_GAME_WINDOW_ACTIVE;
			}
			else {                             // back to the Main window
				SDL_ShowWindow(manager->mainWindow->window);
				manager->activeWindow = SP_MAIN_WINDOW_ACTIVE;
			}

			return SP_MANAGER_UPDATE;

		case SP_LOAD_START:
			result = loadGameToGameWindow(manager);
			if ((result != SP_MANAGER_UPDATE) && (manager->loadWindow != NULL)) {
				loadWindowDestroy(manager->loadWindow);
				manager->loadWindow = NULL;
			}

			return result;

		case SP_LOAD_EXIT:
			loadWindowDestroy(manager->loadWindow);
			manager->loadWindow = NULL;
			return SP_MANAGER_QUIT;

		default:
			return SP_MANAGER_NONE;
	}
}

/**
 * The function loads a game that was chosen from the slots list by the user.
 *
 * @param manager - The gui manager
 *
 * @return
 * 		SP_MANAGER_QUIT   - If the gui program need to be terminated due to a memory error.
 * 		SP_MANAGER_UPDATE - Otherwise.
 */
SP_MANAGER_EVENT loadGameToGameWindow(SPGuiManager* manager) {
	GameSetting* settings;

	if (manager->gameWindow != NULL) { // in case we entered the Load window from the Game window
		settings = copySettings(manager->gameWindow->settings, false);
		gameWindowDestroy(manager->gameWindow);
		manager->gameWindow = NULL;
	}
	else                               // in case we entered the Load window from the Game window
		settings = (GameSetting*) malloc(sizeof(GameSetting));

	if (settings == NULL) // allocation error
		return SP_MANAGER_QUIT;

	// we load the game that is saved in the slot's path and then destroy the current Load window
	loadGame(manager->loadWindow->slotPathToLoad ,settings);
	loadWindowDestroy(manager->loadWindow);
	manager->loadWindow = NULL;
	manager->gameWindow = gameWindowCreate(settings);

	if (manager->gameWindow == NULL) {
		destroySettings(settings);
		settings = NULL;
		printf("ERROR: Couldn't create the Game window\n");
		return SP_MANAGER_QUIT;
	}

	manager->gameWindow->firstCreated = false;
	manager->gameWindow->gameIsSaved = true;
	manager->activeWindow = SP_GAME_WINDOW_ACTIVE;

	return SP_MANAGER_UPDATE;
}

/**
 * The function handles the program according to the result that came back
 * after the event that occurred in the Game window was handled.
 *
 * @param manager - The gui manager
 * @param event   - The SP_GAME_EVENT event that indicates what action that the gui manager needs to take
 *  				 after the Game window's event was handled.
 *
 * @return
 * 		SP_MANAGER_UPDATE  - If either the Game window need to be refreshed with the new game board state,
 * 		                      or if the Load window needs to be shown (after being created).
 *		SP_MANAGER_QUIT    - If the gui program need to be terminated due to a memory error.
 * 		SP_GAME_EVENT_NONE - If no refresh needs to be done to the screen.
 *
 *
 * 		If the received event is either SP_GAME_EVENT_MAIN_MENU or SP_GAME_EVENT_EXIT (i.e. the user is trying to exit
 * 		the game) - returns the result that comes back from the helper function handleExitFromTheGame.
 */
SP_MANAGER_EVENT handleManagerDueToGameEvent(SPGuiManager* manager, SP_GAME_EVENT event) {
	if (manager == NULL)
		return SP_MANAGER_NONE;

	SPGameWindow* gameWindow;
	switch (event) {
		case SP_GAME_EVENT_RESET_GAME:
			gameWindow = manager->gameWindow;
			if (gameWindow->settings->userColor == BLACK_PLAYER) {
				gameWindow->gameIsSaved = false;
				if (executeCompterMove(gameWindow) == SP_GAME_EVENT_QUIT) { // execute the computer move as the first move
					gameWindowDestroy(manager->gameWindow);
					manager->gameWindow = NULL;
					return SP_MANAGER_QUIT;
				}
			}
			return SP_MANAGER_UPDATE;

		case SP_GAME_EVENT_UPDATE:
			return SP_MANAGER_UPDATE;

		case SP_GAME_EVENT_SAVE:
			gameWindow = manager->gameWindow;
			if (!gameWindow->gameIsSaved) {
				if (!saveGameToSlot(manager->gameWindow)) {
					printf(SAVING_GAME_ERROR);
					return SP_MANAGER_QUIT;
				}

				gameWindow->gameIsSaved = true;
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, GAME_TITLE, GAME_SAVED_MESSAGE, NULL);
				gameWindowDraw(manager->gameWindow, false);
			}
			return SP_GAME_EVENT_NONE;

		case SP_GAME_EVENT_LOAD:
			SDL_HideWindow(manager->gameWindow->window);
			manager->loadWindow = loadWindowCreate();

			if (manager->loadWindow == NULL) {
				printf(LOAD_WINDOW_CREATION_ERROR);
				gameWindowDestroy(manager->gameWindow);
				return SP_MANAGER_QUIT;
			}

			manager->activeWindow = SP_LOAD_WINDOW_ACTIVE;
			return SP_MANAGER_UPDATE;

		case SP_GAME_EVENT_MAIN_MENU:
		case SP_GAME_EVENT_EXIT:
			return handleExitFromTheGame(manager, event);

		case SP_GAME_EVENT_QUIT:
			gameWindowDestroy(manager->gameWindow);
			manager->gameWindow = NULL;
			return SP_MANAGER_QUIT;

		default:
			return SP_MANAGER_NONE;
	}
}

/**
 * The function handled the situtaion when the player is trying to exit the game.
 * If the current game board was is not saved - we call a helper function to handle it.
 *
 * @param manager - The gui manager
 * @param event   - The SP_GAME_EVENT event that indicates what was the button that was clicked to exit the game
 *
 * @return
 * 		SP_MANAGER_QUIT   - If the current game board is saved and the player is trying to exit from the program.
 * 		SP_MANAGER_UPDATE - If the current game board is saved and the player is trying to exit to the main menu.
 *		Otherwise         - The function returns what comes back from the helper function handleNonSavedGameExit.
 */
SP_MANAGER_EVENT handleExitFromTheGame(SPGuiManager* manager, SP_GAME_EVENT event) {
	if (manager->gameWindow->gameIsSaved) {
		gameWindowDestroy(manager->gameWindow);
		manager->gameWindow = NULL;

		if (event == SP_GAME_EVENT_EXIT)
			return SP_MANAGER_QUIT;

		manager->activeWindow = SP_MAIN_WINDOW_ACTIVE;
		SDL_ShowWindow(manager->mainWindow->window);
		return SP_MANAGER_UPDATE;
	}

	return handleNonSavedGameExit(manager, event);
}

/**
 * The function handled the situtaion when the player is trying to exit
 * the game (either to the main ment or out of the program), but the current game board is not saved:
 * The program prompt a message box asking the player if he/she
 * wants to save the game before exit or not (or maybe cancel the exit).
 *
 * @param manager - The gui manager
 * @param event   - The SP_GAME_EVENT event that indicates what was the button that was clicked to exit the game
 *
 * @return
 * 		SP_MANAGER_NONE   - If the user clicked the "Cancel" button at the message box.
 * 		SP_MANAGER_UPDATE - If the user clicked the "Main Menu" at the Game Window (and either clicked "Yes" or "No in the prompted message box.
 * 		SP_MANAGER_QUIT   - If the user clicked the "Exit" at the Game Window (and either clicked "Yes" or "No in the prompted message box,
 * 		                     or if an error occurred during the time that the message box was prompted.
 */
SP_MANAGER_EVENT handleNonSavedGameExit(SPGuiManager* manager, SP_GAME_EVENT event) {
	const SDL_MessageBoxButtonData saveOptionsButtons[] = { { 0, CANCEL_BUTTON_MESSAGE_INDEX   , CANCEL_BUTTON_MESSAGE    },
															{ 0, DONT_SAVE_BUTTON_MESSAGE_INDEX, DONT_SAVE_BUTTON_MESSAGE },
															{ 0, SAVE_BUTTON_MESSAGE_INDEX     , SAVE_BUTTON_MESSAGE      } };

	const SDL_MessageBoxData messageboxdata = { SDL_MESSAGEBOX_INFORMATION       ,
												NULL                             ,
												GAME_TITLE                       ,
												SAVE_BEFORE_EXIT_MESSAGE         ,
												SDL_arraysize(saveOptionsButtons),
												saveOptionsButtons               ,
												NULL                              };

	int buttonID;
	if (SDL_ShowMessageBox(&messageboxdata, &buttonID) == 0) {
		switch (buttonID) {
			case SAVE_BUTTON_MESSAGE_INDEX:
			case DONT_SAVE_BUTTON_MESSAGE_INDEX:
				if (buttonID == SAVE_BUTTON_MESSAGE_INDEX)
					saveGameToSlot(manager->gameWindow);

				gameWindowDestroy(manager->gameWindow);
				manager->gameWindow = NULL;

				if (event == SP_GAME_EVENT_MAIN_MENU) {
					manager->activeWindow = SP_MAIN_WINDOW_ACTIVE;
					SDL_ShowWindow(manager->mainWindow->window);
					return SP_MANAGER_UPDATE;
				}

				return SP_MANAGER_QUIT; // event == SP_GAME_EVENT_EXIT

			case CANCEL_BUTTON_MESSAGE_INDEX:
				return SP_MANAGER_NONE;
		}
	}

	gameWindowDestroy(manager->gameWindow);
	return SP_MANAGER_QUIT;
}

/**
 * The function saves the current game in the first slot of the slots files.
 * If the list of file is full, the latest file is forgotten and every file is saved
 * to the next file in the list.
 *
 * @param gameWindow - The Game window of the program
 *
 * @return
 * 		True  - If the game was saved successfully.
 * 		False - If the game couldn't be saved (due to an I/O error).
 */
bool saveGameToSlot(SPGameWindow* gameWindow) {
	GameSetting* settings = (GameSetting*) malloc(sizeof(GameSetting));
	if (settings == NULL)
		return false;

	settings->game = NULL;
	char filePath[SP_MAX_PATH_LENGTH];
	int numOfSavedGames = (numberOfSavedGames());

	for (int slotNum = numOfSavedGames; slotNum > 0; slotNum--) {
		if (slotNum == NUMBER_OF_SAVE_SLOTS)
			continue;

		sprintf(filePath, SLOT_PATH, slotNum);
		loadGame(filePath, settings);
		sprintf(filePath, SLOT_PATH, slotNum + 1);
		saveGame(filePath, settings->game, settings);
		spChessGameDestroy(settings->game);
		settings->game = NULL;
	}

	destroySettings(settings);
	settings = NULL;
	sprintf(filePath, SLOT_PATH, FIRST_SLOT);
	saveGame(filePath, gameWindow->settings->game, gameWindow->settings);

	return true;
}

/**
 * This function frees all the memory of the given gui manager.
 *
 * @param manager - The gui manager
 */
void guiManagerDestroy(SPGuiManager* manager) {
	if (manager == NULL)
		return;

	mainWindowDestroy(manager->mainWindow);
	manager->mainWindow = NULL;
	free(manager);
}
