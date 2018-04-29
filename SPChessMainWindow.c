#include "SPChessMainWindow.h"

/**
 * The function creates the Main window in the gui mode.
 *
 * @return
 * 		NULL      - If there was a memory error.
 * 		Otherwise - Retuerns the main window.
 */
SPMainWindow* mainWindowCreate() {
	SPMainWindow* mainWindow = (SPMainWindow*) malloc(sizeof(SPMainWindow));

	// The Main window's settings
	SDL_Window* window = SDL_CreateWindow(MAIN_MENU_TITLE       ,
										  SDL_WINDOWPOS_CENTERED,
										  SDL_WINDOWPOS_CENTERED,
										  MAIN_WINDOW_WIDTH     ,
										  MAIN_WINDOW_HEIGHT    ,
										  SDL_WINDOW_OPENGL      );

	SDL_Renderer* renderer = SDL_CreateRenderer(window, FIRST_RENDERER, SDL_RENDERER_ACCELERATED);
	SPWidget**    widgets  = mainWindowWidgetsCreate(renderer);

	if ((mainWindow == NULL) || (window == NULL) || (renderer == NULL) || (widgets == NULL)) {
		for (int widgetIndex = 0; widgetIndex < MAIN_WINDOW_WIDGETS; widgetIndex++)
			destroyWidget(widgets[widgetIndex]); // safe to pass NULL
		free(widgets);
		free(renderer);
		free(window);
		free(mainWindow);
		return NULL;
	}

	mainWindow->window       = window;
	mainWindow->renderer     = renderer;
	mainWindow->widgets      = widgets;
	mainWindow->firstCreated = true;

	return mainWindow;
}

/**
 * The function creates the widgets of the Main window.
 *
 * @param renderer - The renderer of the Main window
 *
 * @return
 * 		NULL      - If there was a memory error.
 * 		Otherwise - Retuerns the widgets of the main window.
 */
SPWidget** mainWindowWidgetsCreate(SDL_Renderer* renderer) {
	if (renderer == NULL)
		return NULL ;

	SPWidget** widgets = calloc(MAIN_WINDOW_WIDGETS, sizeof(SPWidget*));
	if (widgets == NULL)
		return NULL ;

	// The location definitions for the widgets of the Main window
	SDL_Rect newGameRect = { .x = BUTTONS_SHIFT_MAIN, .y = NEW_GAME_TOP_BORDER , .w = BUTTON_WIDTH_DEFAULT, .h = BUTTON_HEIGHT_DEFAULT };
	SDL_Rect loadRect    = { .x = BUTTONS_SHIFT_MAIN, .y = LOAD_TOP_BORDER_MAIN, .w = BUTTON_WIDTH_DEFAULT, .h = BUTTON_HEIGHT_DEFAULT };
	SDL_Rect exitRect    = { .x = BUTTONS_SHIFT_MAIN, .y = EXIT_TOP_BORDER_MAIN, .w = BUTTON_WIDTH_DEFAULT, .h = BUTTON_HEIGHT_DEFAULT };

	// The creation of the widgets (as a menu button) for the Main window
	widgets[NEW_GAME_INDEX]  = createButton(renderer, &newGameRect, NEW_GAME_IMAGE, NULL, true, SP_BUTTON_MENU);
	widgets[LOAD_INDEX_MAIN] = createButton(renderer, &loadRect   , LOAD_IMAGE    , NULL, true, SP_BUTTON_MENU);
	widgets[EXIT_INDEX_MAIN] = createButton(renderer, &exitRect   , EXIT_IMAGE    , NULL, true, SP_BUTTON_MENU);

	for (int widgetIndex = 0; widgetIndex < MAIN_WINDOW_WIDGETS; widgetIndex++) {
		if(widgets[widgetIndex] == NULL)
			return NULL;
	}

	return widgets;
}

/**
 * The function draws the Main window to the screen.
 *
 * @param mainWindow - The Main window of the program
 */
void mainWindowDraw(SPMainWindow* mainWindow) {
	if (mainWindow == NULL)
		return;

	SDL_SetRenderDrawColor(mainWindow->renderer, MAX_COLOR_TONE, MAX_COLOR_TONE, MAX_COLOR_TONE, MAX_COLOR_TONE);
	SDL_RenderClear(mainWindow->renderer);

	// loop over all the widgets to draw them on the renderer
	SPWidget* widget;
	for (int widgetIndex = 0; widgetIndex < MAIN_WINDOW_WIDGETS; widgetIndex++) {
		widget = mainWindow->widgets[widgetIndex];
		widget->drawWidget(widget);
	}

	SDL_RenderPresent(mainWindow->renderer);
}

/**
 * The function handles the event that occurred in the main window.
 *
 * @param mainWindow - The main window of the program
 * @param event      - The eventbe handle
 *
 * @return
 * 		SP_MAIN_INVALID_ARGUMENT - If the function received a NULL pointer.
 * 		SP_MAIN_START            - If the "New game" button was clicked.
 * 		SP_MAIN_LOAD             - If the "Load" button was clicked.
 * 		SP_MAIN_EXIT             - If either the "Exit" button was clicked or the closed window button ("X" button at the window's corner).
 * 		SP_MAIN_NONE             - If no button was clicked.
 */
SP_MAIN_EVENT mainWindowHandleEvent(SPMainWindow* mainWindow, SDL_Event* event) {
	if ((mainWindow == NULL) || (event == NULL))
		return SP_MAIN_INVALID_ARGUMENT;

	SPWidget *newGameWidget, *loadGameWidget, *exitGameWidget;

	switch (event->type) {
		case SDL_MOUSEBUTTONUP:
			newGameWidget  = mainWindow->widgets[NEW_GAME_INDEX];
			loadGameWidget = mainWindow->widgets[LOAD_INDEX_MAIN];
			exitGameWidget = mainWindow->widgets[EXIT_INDEX_MAIN];

			if (newGameWidget->handleEvent(newGameWidget, event))
				return SP_MAIN_START;
			else if (loadGameWidget->handleEvent(loadGameWidget, event))
				return SP_MAIN_LOAD;
			else if (exitGameWidget->handleEvent(exitGameWidget, event))
				return SP_MAIN_EXIT;

			break;

		case SDL_WINDOWEVENT:

			if (event->window.event == SDL_WINDOWEVENT_CLOSE)
				return SP_MAIN_EXIT;
	}

	return SP_MAIN_NONE;
}

/**
 * The function frees all the memory of the Main window.
 *
 * @param mainWindow - The Main window of the program
 */
void mainWindowDestroy(SPMainWindow* mainWindow) {
	if (mainWindow == NULL)
		return;

	for (int widgetIndex = 0; widgetIndex < MAIN_WINDOW_WIDGETS; widgetIndex++)
		destroyWidget(mainWindow->widgets[widgetIndex]);
	free(mainWindow->widgets);

	if (mainWindow->renderer != NULL)
		SDL_DestroyRenderer(mainWindow->renderer);

	if (mainWindow->window != NULL)
		SDL_DestroyWindow(mainWindow->window);

	free(mainWindow);
}
