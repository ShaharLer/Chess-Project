#include "SPChessSettingsWindow.h"

/**
 * The function creates the Settings window in the gui mode.
 *
 * @return
 * 		NULL      - If there was a memory error.
 * 		Otherwise - Retuerns the Settings window.
 */
SPSettingsWindow* settingsWindowCreate() {
	SPSettingsWindow* settingsWindow = (SPSettingsWindow*) malloc(sizeof(SPSettingsWindow));
	SDL_Window*       window         = SDL_CreateWindow(SETTINGS_TILTE        ,
				                                        SDL_WINDOWPOS_CENTERED,
				                                        SDL_WINDOWPOS_CENTERED,
				                                        SETTINGS_WINDOW_WIDTH ,
				                                        SETTINGS_WINDOW_HEIGHT,
				                                        SDL_WINDOW_OPENGL      );

	SDL_Renderer* renderer = SDL_CreateRenderer(window, FIRST_RENDERER, SDL_RENDERER_ACCELERATED);
	SPWidget**    widgets  = settingsWindowWidgetsCreate(renderer);
	GameSetting*  settings = (GameSetting*) malloc(sizeof(GameSetting));

	if ((settingsWindow == NULL) || (window == NULL) || (renderer == NULL) || (widgets == NULL) || (settings == NULL)) {
		free(settings);
		for (int widgetIndex = 0; widgetIndex < SETTINGS_WINDOW_WIDGETS; widgetIndex++)
			destroyWidget(widgets[widgetIndex]); // safe to pass NULL
		free(widgets);
		free(renderer);
		free(window);
		free(settingsWindow);
		return NULL;
	}

	setDefaultValues(settings); // set the settings to their default at the creation of the window

	settingsWindow->window   = window;
	settingsWindow->renderer = renderer;
	settingsWindow->widgets  = widgets;
	settingsWindow->settings = settings;
	settingsWindow->menu     = SP_SETTINGS_GAME_MODE_MENU;

	return settingsWindow;
}

/**
 * The function creates the widgets of the Settings window.
 *
 * @param renderer - The renderer of the Settings window
 *
 * @return
 * 		NULL      - If there was a memory error.
 * 		Otherwise - Retuerns the widgets of the Settings window.
 */
SPWidget** settingsWindowWidgetsCreate(SDL_Renderer* renderer) {
	if (renderer == NULL)
		return NULL;

	SPWidget** widgets = calloc(SETTINGS_WINDOW_WIDGETS, sizeof(SPWidget*));
	if (widgets == NULL)
		return NULL;

	// The location definitions for the widgets of the Settings window
	SDL_Rect gameModeRect   = { .x = GAME_MODE_SHIFT         , .y = MENU_TITLES_TOP_BORDER   , .w = GAME_MODE_WIDTH     , .h = GAME_MODE_HEIGHT         };
	SDL_Rect difficultyRect = { .x = DIFFICULTY_SHIFT        , .y = MENU_TITLES_TOP_BORDER   , .w = DIFFICULTY_WIDTH    , .h = DIFFICULTY_HEIGHT        };
	SDL_Rect userColorRect  = { .x = USER_COLOR_SHIFT        , .y = MENU_TITLES_TOP_BORDER   , .w = USER_COLOR_WIDTH    , .h = USER_COLOR_HEIGHT        };
	SDL_Rect onePlayerRect  = { .x = ONE_PLAYER_SHIFT        , .y = CHOOSE_PLAYERS_TOP_BORDER, .w = BUTTON_WIDTH_DEFAULT, .h = GAME_MODE_BUTTONS_HEIGHT };
	SDL_Rect twoPlayersRect = { .x = TWO_PLAYERS_SHIFT       , .y = CHOOSE_PLAYERS_TOP_BORDER, .w = BUTTON_WIDTH_DEFAULT, .h = GAME_MODE_BUTTONS_HEIGHT };
	SDL_Rect nextRect       = { .x = NEXT_SHIFT              , .y = EXIT_MENU_TOP_BORDER     , .w = BUTTON_WIDTH_DEFAULT, .h = BUTTON_HEIGHT_DEFAULT    };
	SDL_Rect startRect      = { .x = START_SHIFT             , .y = EXIT_MENU_TOP_BORDER     , .w = BUTTON_WIDTH_DEFAULT, .h = BUTTON_HEIGHT_DEFAULT    };
	SDL_Rect backRect       = { .x = BACK_SHIFT              , .y = EXIT_MENU_TOP_BORDER     , .w = BUTTON_WIDTH_DEFAULT, .h = BUTTON_HEIGHT_DEFAULT    };
	SDL_Rect noobRect       = { .x = DIFFICULTY_BUTTONS_SHIFT, .y = NOOB_TOP_BORDER  , .w = BUTTON_WIDTH_DEFAULT, .h = BUTTON_HEIGHT_DEFAULT    };
	SDL_Rect easyRect       = { .x = DIFFICULTY_BUTTONS_SHIFT, .y = EASY_TOP_BORDER          , .w = BUTTON_WIDTH_DEFAULT, .h = BUTTON_HEIGHT_DEFAULT    };
	SDL_Rect moderateRect   = { .x = DIFFICULTY_BUTTONS_SHIFT, .y = MODERATE_TOP_BORDER      , .w = BUTTON_WIDTH_DEFAULT, .h = BUTTON_HEIGHT_DEFAULT    };
	SDL_Rect hardRect       = { .x = DIFFICULTY_BUTTONS_SHIFT, .y = HARD_TOP_BORDER          , .w = BUTTON_WIDTH_DEFAULT, .h = BUTTON_HEIGHT_DEFAULT    };
	SDL_Rect blackColorRect = { .x = BLACK_PLAYER_SHIFT      , .y = COLORS_TOP_BORDER        , .w = COLORS_WIDTH        , .h = COLORS_HEIGHT            };
	SDL_Rect whiteColorRect = { .x = WHITE_PLAYER_SHIFT      , .y = COLORS_TOP_BORDER        , .w = COLORS_WIDTH        , .h = COLORS_HEIGHT            };

	// The creation of the widgets (as a menu button) for the Settings menus: "Game Mode" menu, "Difficulty" menu, "User Color" menu
	widgets[GAME_MODE_INDEX]     = createButton(renderer, &gameModeRect  , GAME_MODE_TITLE_IMAGE   , NULL                        , false, SP_BUTTON_MENU);
	widgets[DIFFICULTY_INDEX]    = createButton(renderer, &difficultyRect, DIFFICULTY_TITLE_IMAGE  , NULL                        , false, SP_BUTTON_MENU);
	widgets[USER_COLOR_INDEX]    = createButton(renderer, &userColorRect , USER_COLOR_TITLE_IMAGE  , NULL                        , false, SP_BUTTON_MENU);
	widgets[ONE_PLAYER_INDEX]    = createButton(renderer, &onePlayerRect , ONE_PLAYER_CHOSEN_IMAGE , ONE_PLAYER_NOT_CHOSEN_IMAGE , false, SP_BUTTON_MENU);
	widgets[TWO_PLAYERS_INDEX]   = createButton(renderer, &twoPlayersRect, TWO_PLAYERS_CHOSEN_IMAGE, TWO_PLAYERS_NOT_CHOSEN_IMAGE, false, SP_BUTTON_MENU);
	widgets[NEXT_INDEX]          = createButton(renderer, &nextRect      , NEXT_IMAGE              , NULL                        , false, SP_BUTTON_MENU);
	widgets[START_INDEX]         = createButton(renderer, &startRect     , START_IMAGE             , NULL                        , false, SP_BUTTON_MENU);
	widgets[BACK_INDEX_SETTINGS] = createButton(renderer, &backRect      , BACK_IMAGE              , NULL                        , false, SP_BUTTON_MENU);
	widgets[NOOB_INDEX]          = createButton(renderer, &noobRect      , NOOB_CHOSEN_IMAGE       , NOOB_NOT_CHOSEN_IMAGE       , false, SP_BUTTON_MENU);
	widgets[EASY_INDEX]          = createButton(renderer, &easyRect      , EASY_CHOSEN_IMAGE       , EASY_NOT_CHOSEN_IMAGE       , false, SP_BUTTON_MENU);
	widgets[MODERATE_INDEX]      = createButton(renderer, &moderateRect  , MODERATE_CHOSEN_IMAGE   , MODERATE_NOT_CHOSEN_IMAGE   , false, SP_BUTTON_MENU);
	widgets[HARD_INDEX]          = createButton(renderer, &hardRect      , HARD_CHOSEN_IMAGE       , HARD_NOT_CHOSEN_IMAGE       , false, SP_BUTTON_MENU);
	widgets[BLACK_COLOR_INDEX]   = createButton(renderer, &blackColorRect, BLACK_COLOR_CHOSEN_IMAGE, BLACK_COLOR_NOT_CHOSEN_IMAGE, false, SP_BUTTON_MENU);
	widgets[WHITE_COLOR_INDEX]   = createButton(renderer, &whiteColorRect, WHITE_COLOR_CHOSEN_IMAGE, WHITE_COLOR_NOT_CHOSEN_IMAGE, false, SP_BUTTON_MENU);

	for (int widgetIndex = 0; widgetIndex < SETTINGS_WINDOW_WIDGETS; widgetIndex++) {
		if(widgets[widgetIndex] == NULL)
			return NULL;
	}

	return widgets;
}

/**
 * The function draws the relevant menu for Settings window to the screen.
 *
 * @param settingsWindow - The Settings window of the program
 */
void settingsWindowDraw(SPSettingsWindow* settingsWindow) {
	if (settingsWindow == NULL)
		return;

	SDL_SetRenderDrawColor(settingsWindow->renderer, MAX_COLOR_TONE, MAX_COLOR_TONE, MAX_COLOR_TONE, MAX_COLOR_TONE);
	SDL_RenderClear(settingsWindow->renderer);

	// clear the indicators for all buttons before updating the relevant ones
	for (int widgetIndex = 0; widgetIndex < SETTINGS_WINDOW_WIDGETS; widgetIndex++) {
		((SPButton*) settingsWindow->widgets[widgetIndex]->data)->showButton = false;
		((SPButton*) settingsWindow->widgets[widgetIndex]->data)->isActive   = false;
	}

	switch (settingsWindow->menu) {
		case (SP_SETTINGS_GAME_MODE_MENU):

			// update the buttons that need to be shown/active in the first menu of the Settings window
			setGameModeMenu(settingsWindow);
			break;

		case (SP_SETTINGS_DIFFICULTY_MENU):

			// update the buttons that need to be shown/active in the second menu of the Settings window
			setDifficultyMenu(settingsWindow);
			break;

		case (SP_SETTINGS_USER_COLOR_MENU):

			// update the buttons that need to be shown/active in the third menu of the Settings window
			setUserColorMenu(settingsWindow);
	}

	((SPButton*) settingsWindow->widgets[BACK_INDEX_SETTINGS]->data)->showButton = true; // "Back" button is shown in every Settings menu

	SPWidget* widget;
	for (int widgetIndex = 0; widgetIndex < SETTINGS_WINDOW_WIDGETS; widgetIndex++) {
		widget = settingsWindow->widgets[widgetIndex];
		widget->drawWidget(widget);
	}

	SDL_RenderPresent(settingsWindow->renderer);
}

/**
 * The function makes the relevant updates for the buttons of the Settings window's "Game Mode" menu.
 *
 * @param settingsWindow - The Settings window of the program
 */
void setGameModeMenu(SPSettingsWindow* settingsWindow) {
	((SPButton*) settingsWindow->widgets[GAME_MODE_INDEX]->data)->showButton   = true;
	((SPButton*) settingsWindow->widgets[ONE_PLAYER_INDEX]->data)->showButton  = true;
	((SPButton*) settingsWindow->widgets[TWO_PLAYERS_INDEX]->data)->showButton = true;

	if (settingsWindow->settings->gameMode == ONE_PLAYER_MODE) {
		((SPButton*) settingsWindow->widgets[ONE_PLAYER_INDEX]->data)->isActive = true;
		((SPButton*) settingsWindow->widgets[NEXT_INDEX]->data)->showButton     = true;
	}
	else if (settingsWindow->settings->gameMode == TWO_PLAYERS_MODE) {
		((SPButton*) settingsWindow->widgets[TWO_PLAYERS_INDEX]->data)->isActive = true;
		((SPButton*) settingsWindow->widgets[START_INDEX]->data)->showButton     = true;
	}
}

/**
 * The function makes the relevant updates for the buttons of the Settings window's "Difficulty" menu.
 *
 * @param settingsWindow - The Settings window of the program
 */
void setDifficultyMenu(SPSettingsWindow* settingsWindow) {
	((SPButton*) settingsWindow->widgets[DIFFICULTY_INDEX]->data)->showButton = true;
	((SPButton*) settingsWindow->widgets[NEXT_INDEX]->data)->showButton       = true;
	((SPButton*) settingsWindow->widgets[NOOB_INDEX]->data)->showButton       = true;
	((SPButton*) settingsWindow->widgets[EASY_INDEX]->data)->showButton       = true;
	((SPButton*) settingsWindow->widgets[MODERATE_INDEX]->data)->showButton   = true;
	((SPButton*) settingsWindow->widgets[HARD_INDEX]->data)->showButton       = true;

	int difficulty = settingsWindow->settings->difficulty;

	switch (difficulty) {
		case NOOB_LEVEL:
			((SPButton*) settingsWindow->widgets[NOOB_INDEX]->data)->isActive = true;
			break;

		case EASY_LEVEL:
			((SPButton*) settingsWindow->widgets[EASY_INDEX]->data)->isActive = true;
			break;

		case MODERATE_LEVEL:
			((SPButton*) settingsWindow->widgets[MODERATE_INDEX]->data)->isActive = true;
			break;

		case HARD_LEVEL:
			((SPButton*) settingsWindow->widgets[HARD_INDEX]->data)->isActive = true;
			break;
	}
}

/**
 * The function makes the relevant updates for the buttons of the Settings window's "User Color" menu.
 *
 * @param settingsWindow - The Settings window of the program
 */
void setUserColorMenu(SPSettingsWindow* settingsWindow) {
	((SPButton*) settingsWindow->widgets[USER_COLOR_INDEX]->data)->showButton  = true;
	((SPButton*) settingsWindow->widgets[START_INDEX]->data)->showButton       = true;
	((SPButton*) settingsWindow->widgets[BLACK_COLOR_INDEX]->data)->showButton = true;
	((SPButton*) settingsWindow->widgets[WHITE_COLOR_INDEX]->data)->showButton = true;

	(settingsWindow->settings->userColor == BLACK_PLAYER) ?
							(((SPButton*) settingsWindow->widgets[BLACK_COLOR_INDEX]->data)->isActive = true)
						  : (((SPButton*) settingsWindow->widgets[WHITE_COLOR_INDEX]->data)->isActive = true);
}

/**
 * The function handles the event that occurred in the Settings window.
 *
 * @param settingsWindow - The Settings window of the program
 * @param event          - The event to handle
 *
 * @return
 * 		SP_SETTINGS_EVENT_INVALID_ARGUMENT - If the function received a NULL pointer.
 * 		SP_SETTINGS_EVENT_EXIT             - If either the "Exit" button was clicked or the closed window button ("X" button at the window's corner).
 * 		SP_SETTINGS_EVENT_NONE             - If no button was clicked.
 * 		Otherwise                          - Returns the SP_SETTINGS_EVENT that returned form the relevant helper function.
 */
SP_SETTINGS_EVENT settingsWindowHandleEvent(SPSettingsWindow* settingsWindow, SDL_Event* event) {
	if ((settingsWindow == NULL) || (event == NULL))
		return SP_SETTINGS_EVENT_INVALID_ARGUMENT;

	switch (event->type) {
		case SDL_MOUSEBUTTONUP:
			switch (settingsWindow->menu) {
				case SP_SETTINGS_GAME_MODE_MENU:
					return gameModeMenuHandleEvent(settingsWindow, event);

				case SP_SETTINGS_DIFFICULTY_MENU:
					return difficultyMenuHandleEvent(settingsWindow, event);

				case SP_SETTINGS_USER_COLOR_MENU:
					return userColorMenuHandleEvent(settingsWindow, event);
			}

			break;

		case SDL_WINDOWEVENT:
			if (event->window.event == SDL_WINDOWEVENT_CLOSE)
				return SP_SETTINGS_EVENT_EXIT;
	}

	return SP_SETTINGS_EVENT_NONE;
}

/**
 * The function handles the event that occurred in the "Game Mode" menu of the Settings window.
 *
 * @param settingsWindow - The Settings window of the program
 * @param event          - The event to handle
 *
 * @return
 * 		SP_SETTINGS_EVENT_BACK   - If the "BACK" button was clicked.
 * 		SP_SETTINGS_EVENT_START  - If the "START" button was clicked.
 *		SP_SETTINGS_EVENT_UPDATE - If one of the other buttons was clicked.
 *		SP_SETTINGS_EVENT_NONE   - Default value.
 */
SP_SETTINGS_EVENT gameModeMenuHandleEvent(SPSettingsWindow* settingsWindow, SDL_Event* event) {
	SPWidget* onePlayerWidget  = settingsWindow->widgets[ONE_PLAYER_INDEX];
	SPWidget* twoPlayersWidget = settingsWindow->widgets[TWO_PLAYERS_INDEX];
	SPWidget* nextWidget       = settingsWindow->widgets[NEXT_INDEX];
	SPWidget* startWidget      = settingsWindow->widgets[START_INDEX];
	SPWidget* backWidget       = settingsWindow->widgets[BACK_INDEX_SETTINGS];

	if (onePlayerWidget->handleEvent(onePlayerWidget, event)) {
		settingsWindow->settings->gameMode = ONE_PLAYER_MODE;
		return SP_SETTINGS_EVENT_UPDATE;
	}

	if (twoPlayersWidget->handleEvent(twoPlayersWidget, event)) {
		settingsWindow->settings->gameMode = TWO_PLAYERS_MODE;
		return SP_SETTINGS_EVENT_UPDATE;
	}

	if (nextWidget->handleEvent(nextWidget, event)) {
		settingsWindow->menu++;
		return SP_SETTINGS_EVENT_UPDATE;
	}

	if (startWidget->handleEvent(startWidget, event))
		return SP_SETTINGS_EVENT_START;

	if (backWidget->handleEvent(backWidget, event))
		return SP_SETTINGS_EVENT_BACK;

	return SP_SETTINGS_EVENT_NONE;
}

/**
 * The function handles the event that occurred in the "Difficulty" menu of the Settings window.
 *
 * @param settingsWindow - The Settings window of the program
 * @param event          - The event to handle
 *
 * @return
 *		SP_SETTINGS_EVENT_UPDATE - If one of the buttons was clicked.
 *		SP_SETTINGS_EVENT_NONE   - Default value.
 */
SP_SETTINGS_EVENT difficultyMenuHandleEvent(SPSettingsWindow* settingsWindow, SDL_Event* event) {
	SPWidget* noobWidget     = settingsWindow->widgets[NOOB_INDEX];
	SPWidget* easyWidget     = settingsWindow->widgets[EASY_INDEX];
	SPWidget* moderateWidget = settingsWindow->widgets[MODERATE_INDEX];
	SPWidget* hardWidget     = settingsWindow->widgets[HARD_INDEX];
	SPWidget* nextWidget     = settingsWindow->widgets[NEXT_INDEX];
	SPWidget* backWidget     = settingsWindow->widgets[BACK_INDEX_SETTINGS];

	if (noobWidget->handleEvent(noobWidget, event)) {
		settingsWindow->settings->difficulty = NOOB_LEVEL;
		return SP_SETTINGS_EVENT_UPDATE;
	}

	if (easyWidget->handleEvent(easyWidget, event)) {
		settingsWindow->settings->difficulty = EASY_LEVEL;
		return SP_SETTINGS_EVENT_UPDATE;
	}

	if (moderateWidget->handleEvent(moderateWidget, event)) {
		settingsWindow->settings->difficulty = MODERATE_LEVEL;
		return SP_SETTINGS_EVENT_UPDATE;
	}

	if (hardWidget->handleEvent(hardWidget, event)) {
		settingsWindow->settings->difficulty = HARD_LEVEL;
		return SP_SETTINGS_EVENT_UPDATE;
	}

	if (nextWidget->handleEvent(nextWidget, event)) {
		settingsWindow->menu++;
		return SP_SETTINGS_EVENT_UPDATE;
	}

	if (backWidget->handleEvent(backWidget,event)) {
		settingsWindow->menu--;
		return SP_SETTINGS_EVENT_UPDATE;
	}

	return SP_SETTINGS_EVENT_NONE;
}

/**
 * The function handles the event that occurred in the "User Color" menu of the Settings window.
 *
 * @param settingsWindow - The Settings window of the program
 * @param event          - The event to handle
 *
 * @return
 * 		SP_SETTINGS_EVENT_START  - If the "START" button was clicked.
 *		SP_SETTINGS_EVENT_UPDATE - If one of the other buttons was clicked.
 *		SP_SETTINGS_EVENT_NONE   - Default value.
 */
SP_SETTINGS_EVENT userColorMenuHandleEvent(SPSettingsWindow* settingsWindow, SDL_Event* event) {
	SPWidget* blackColorWidget = settingsWindow->widgets[BLACK_COLOR_INDEX];
	SPWidget* whiteColorWidget = settingsWindow->widgets[WHITE_COLOR_INDEX];
	SPWidget* startWidget      = settingsWindow->widgets[START_INDEX];
	SPWidget* backWidget       = settingsWindow->widgets[BACK_INDEX_SETTINGS];


	if (blackColorWidget->handleEvent(blackColorWidget, event)) {
		settingsWindow->settings->userColor = BLACK_PLAYER;
		return SP_SETTINGS_EVENT_UPDATE;
	}

	if (whiteColorWidget->handleEvent(whiteColorWidget, event)) {
		settingsWindow->settings->userColor = WHITE_PLAYER;
		return SP_SETTINGS_EVENT_UPDATE;
	}

	if (startWidget->handleEvent(startWidget, event))
		return SP_SETTINGS_EVENT_START;

	if (backWidget->handleEvent(backWidget, event)) {
		settingsWindow->menu--;
		return SP_SETTINGS_EVENT_UPDATE;
	}

	return SP_SETTINGS_EVENT_NONE;
}

/**
 * The function frees all the memory of the Settings window.
 *
 * @param settingsWindow - The Settings window of the program
 */
void settingsWindowDestroy(SPSettingsWindow* settingsWindow) {
	if (settingsWindow == NULL)
		return;

	destroySettings(settingsWindow->settings);

	for (int widgetIndex = 0; widgetIndex < SETTINGS_WINDOW_WIDGETS; widgetIndex++)
		destroyWidget(settingsWindow->widgets[widgetIndex]); // NULL free function
	free(settingsWindow->widgets);

	if (settingsWindow->renderer != NULL)
		SDL_DestroyRenderer(settingsWindow->renderer);

	if (settingsWindow->window != NULL)
		SDL_DestroyWindow(settingsWindow->window);

	free(settingsWindow);
}
