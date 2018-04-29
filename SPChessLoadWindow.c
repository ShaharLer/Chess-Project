#include "SPChessLoadWindow.h"

/**
 * The function creates the Load window in the gui mode.
 *
 * @return
 * 		NULL      - If there was a memory error.
 * 		Otherwise - Returns the load window.
 */
SPLoadWindow* loadWindowCreate() {
	SPLoadWindow* loadWindow = (SPLoadWindow*) malloc(sizeof(SPLoadWindow));

	// The Load window's settings
	SDL_Window* window = SDL_CreateWindow(LOAD_TITLE            ,
										  SDL_WINDOWPOS_CENTERED,
										  SDL_WINDOWPOS_CENTERED,
										  LOAD_WINDOW_WIDTH     ,
										  LOAD_WINDOW_HEIGHT    ,
										  SDL_WINDOW_OPENGL      );

	SDL_Renderer* renderer = SDL_CreateRenderer(window, FIRST_RENDERER, SDL_RENDERER_ACCELERATED);

	int numOfWidgets = (LOAD_WINDOW_FIXED_WIDGETS + numberOfSavedGames());
	SPWidget** widgets = loadWindowWidgetsCreate(renderer, numOfWidgets);

	if ((loadWindow == NULL) || (window == NULL) || (renderer == NULL) || (widgets == NULL)) {
		for (int widgetIndex = 0; widgetIndex < numOfWidgets; widgetIndex++)
			destroyWidget(widgets[widgetIndex]); // safe to pass NULL
		free(widgets);
		free(renderer);
		free(window);
		free(loadWindow);
		return NULL;
	}

	loadWindow->window       = window;
	loadWindow->renderer     = renderer;
	loadWindow->widgets      = widgets;
    loadWindow->numOfWidgets = numOfWidgets;

	return loadWindow;
}

/**
 * The function counts the number of save slots that are not empty (i.e. have games saved in them) and returns it.
 *
 * @return
 * 		The number of saved games so far.
 */
int numberOfSavedGames() {
	char filePath[SP_MAX_PATH_LENGTH];
	int numberOfSlots = 0;

	for (int i = 0; i < NUMBER_OF_SAVE_SLOTS; i++) {
		sprintf(filePath, SLOT_PATH, i+1);

		if (isSlotEmpty(filePath))
			break;

		numberOfSlots++; // game slot number i is not empty
	}

	return numberOfSlots;
}

/**
 * The function checks if a certain slot is empty.
 *
 * @param filePath - the path of the slot that is checked.
 *
 * @returns
 * 		true - If the save slot is empty. Otherwise, it returns false.
 */
bool isSlotEmpty(char* filePath) {
	FILE* file = fopen(filePath, "r");

	if (file == NULL) { // if file doesn't exist or cannot be opened
		ERROR_OPENING_XML(filePath);
		return true;
	}

	fseek(file, ZERO_OFFSET, SEEK_END);
	long endPosition = ftell(file);
	fclose(file);

	return (endPosition == START_OF_FILE_POSITION);
}

/**
 * The function creates the widgets of the Main window.
 *
 * @param renderer     - The renderer of the Main window
 * @param numOfWidgets - The actual number of widget (menu buttons and slots) of the current Load window
 *
 * @return
 * 		NULL      - If there was a memory error.
 * 		Otherwise - Retuerns the widgets of the main window.
 */
SPWidget** loadWindowWidgetsCreate(SDL_Renderer* renderer, int numOfWidgets) {
	if (renderer == NULL)
		return NULL;

	SPWidget** widgets = calloc(numOfWidgets, sizeof(SPWidget*));
	if (widgets == NULL)
		return NULL;

	// The location definitions for the menu widgets of the Load window
	SDL_Rect backRect = { .x = BACK_BUTTON_SHIFT, .y = BACK_AND_LOAD_BUTTONS_TOP_BORDER, .w = BUTTON_WIDTH_DEFAULT, .h = BUTTONS_HEIGHT_LOAD_WINDOW };
	SDL_Rect loadRect = { .x = LOAD_BUTTON_SHIFT, .y = BACK_AND_LOAD_BUTTONS_TOP_BORDER, .w = BUTTON_WIDTH_DEFAULT, .h = BUTTONS_HEIGHT_LOAD_WINDOW };

	// The creation of the widgets (as a menu button) for the Load window
	widgets[BACK_WIDGET_INDEX] = createButton(renderer, &backRect, BACK_IMAGE, NULL                  , true, SP_BUTTON_MENU);
	widgets[LOAD_WIDGET_INDEX] = createButton(renderer, &loadRect, LOAD_IMAGE, LOAD_UNAVAILABLE_IMAGE, true, SP_BUTTON_MENU);

	// The creation of the files widgets for the Load window
	char slotChosenImage[SP_MAX_PATH_LENGTH];
	char slotNotChosenImage[SP_MAX_PATH_LENGTH];

	for (int widgetIndex = FIRST_SLOT_INDEX; widgetIndex < numOfWidgets; widgetIndex++) {
		int fileIndex = (widgetIndex - 1); // We get the file number by reducing 1 from the widget index

		sprintf(slotChosenImage   , SLOT_CHOSEN_IMAGE_PATH    , fileIndex); // after reducing 1 from the widget index we get the right file number
		sprintf(slotNotChosenImage, SLOT_NOT_CHOSEN_IMAGE_PATH, fileIndex); // after reducing 1 from the widget index we get the right file number

		SDL_Rect slotRect = { .x = GAME_SLOT_BUTTONS_SHIFT, .y = GAME_SLOT_BUTTONS_TOP_BORDER(fileIndex),
				              .w = BUTTON_WIDTH_DEFAULT   , .h = BUTTONS_HEIGHT_LOAD_WINDOW              };

		widgets[widgetIndex] = createButton(renderer, &slotRect, slotChosenImage, slotNotChosenImage, true, SP_BUTTON_MENU);
	}

	for (int widgetIndex = 0; widgetIndex < numOfWidgets; widgetIndex++) {
		if (widgets[widgetIndex] == NULL)
			return NULL;
	}

	return widgets;
}

/**
 * The function draws the Load window to the screen.
 *
 * @param loadWindow - The Load window of the program
 */
void loadWindowDraw(SPLoadWindow* loadWindow) {
	if (loadWindow == NULL)
		return;

	SDL_SetRenderDrawColor(loadWindow->renderer, MAX_COLOR_TONE, MAX_COLOR_TONE, MAX_COLOR_TONE, MAX_COLOR_TONE);
	SDL_RenderClear(loadWindow->renderer);

	// loop over all the widgets to draw them on the renderer
	SPWidget* widget;
	for (int widgetIndex = 0; widgetIndex < loadWindow->numOfWidgets; widgetIndex++) {
		widget = loadWindow->widgets[widgetIndex];
		widget->drawWidget(widget);

		if ((widgetIndex != BACK_WIDGET_INDEX) && (widgetIndex != LOAD_WIDGET_INDEX))
			((SPButton*) widget->data)->isActive = false; // a slot button will be turned on afterwards if it will be clicked
	}

	SDL_RenderPresent(loadWindow->renderer);
}

/**
 * The function handles the event that occurred in the Main window.
 *
 * @param mainWindow - The Main window of the program
 * @param event      - The event to be handled
 *
 * @return
 * 		MAIN_INVALID_ARGUMENT - If the function received a NULL pointer.
 * 		MAIN_START            - If the "New game" button was pushed.
 * 		MAIN_LOAD             - If the "Load" button was pushed.
 * 		MAIN_EXIT             - If the "X" button at the window's corner was clicked.
 * 		MAIN_NONE             - If no button was pushed.
 */
SP_LOAD_EVENT loadWindowHandleEvent(SPLoadWindow* loadWindow, SDL_Event* event) {
	if ((loadWindow == NULL) || (event == NULL))
		return SP_LOAD_INVALID_ARGUMENT;

	SPWidget *backWidget, *loadGameWidget, *slotWidget;

	switch (event->type) {
		case SDL_MOUSEBUTTONUP:
			backWidget     = loadWindow->widgets[BACK_WIDGET_INDEX];
			loadGameWidget = loadWindow->widgets[LOAD_WIDGET_INDEX];

			if (backWidget->handleEvent(backWidget, event))
				return SP_LOAD_BACK;

			if (loadGameWidget->handleEvent(loadGameWidget, event)) {
				if (((SPButton*) loadGameWidget->data)->isActive)
					return SP_LOAD_START;

				break;
			}

			for (int widgetIndex = FIRST_SLOT_INDEX; widgetIndex < loadWindow->numOfWidgets; widgetIndex++) {
				slotWidget = loadWindow->widgets[widgetIndex];

				if (slotWidget->handleEvent(slotWidget, event)) {
					sprintf(loadWindow->slotPathToLoad, SLOT_PATH, widgetIndex - 1); // after reducing 1 we get the right file index in the files list
					((SPButton*) slotWidget->data)->isActive = true;
					((SPButton*) loadWindow->widgets[LOAD_WIDGET_INDEX]->data)->isActive = true;

					return SP_LOAD_UPDATE;
				}
			}

			return SP_LOAD_NONE;

		case SDL_WINDOWEVENT:

			if (event->window.event == SDL_WINDOWEVENT_CLOSE)
				return SP_LOAD_EXIT;
	}

	return SP_LOAD_NONE;
}

/**
 * The function frees all the memory of the Main window.
 *
 * @param mainWindow - The Main window of the program
 */
void loadWindowDestroy(SPLoadWindow* loadWindow) {
	if (loadWindow == NULL)
		return;

	for (int widgetIndex = 0; widgetIndex < loadWindow->numOfWidgets; widgetIndex++)
		destroyWidget(loadWindow->widgets[widgetIndex]);
	free(loadWindow->widgets);

	if (loadWindow->renderer != NULL)
		SDL_DestroyRenderer(loadWindow->renderer);

	if (loadWindow->window != NULL)
		SDL_DestroyWindow(loadWindow->window);

	free(loadWindow);
}
