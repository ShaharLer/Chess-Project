#include "SPChessButton.h"

/**
 * The function creates a button (either menu button of a board piece button) by creating a widget and assigning the button as its data.
 *
 * @param windowRenderer - The renderer of the button's widnow
 * @param location       - The rectangle where the button will be placed (initialized as NULL for board piece)
 * @param buttonOn       - The image path of the button (for a menu button, it is the image when it is on/available)
 * @param buttonOf       - The image path of the button when it is off/not available (NULL for all piece buttons)
 * @param showButton     - Indicates if the button will be showed
 *
 * @return
 * 		NULL      - If an alocation memory occurs.
 * 		Otherwise - Returns the widget that the new button is its data.
 */
SPWidget* createButton(SDL_Renderer* renderer, SDL_Rect* location, const char* buttonOn, const char* buttonOff, bool showButton, SP_BUTTON_TYPE type) {
	if ((renderer == NULL) || ((type == SP_BUTTON_MENU) && ((location == NULL) || (buttonOn == NULL))))
		return NULL;

	SPWidget* widget = (SPWidget*) malloc(sizeof(SPWidget));
	SPButton* button = (SPButton*) malloc(sizeof(SPButton));
	if ((widget == NULL) || (button == NULL)) {
		free(button);
		free(widget);
		return NULL;
	}

	button->textureForButtonOn       = NULL;  // will  be updated for a menu button
	button->textureForButtonOff      = NULL;  // might be updated for a menu button
	SDL_Texture* textureForButtonOn  = NULL;
	SDL_Texture* textureForButtonOff = NULL;

	if (type == SP_BUTTON_MENU) {
		SDL_Surface* loadingSurface = SDL_LoadBMP(buttonOn);
		textureForButtonOn = SDL_CreateTextureFromSurface(renderer, loadingSurface);
		SDL_FreeSurface(loadingSurface); // After the texture is created, we can free the surface (it is safe to pass NULL)

		if (buttonOff != NULL) { // Creating the texture for the button when it is not avtive/not chosen
			SDL_Surface* loadingPressedSurface = SDL_LoadBMP(buttonOff);
			textureForButtonOff = SDL_CreateTextureFromSurface(renderer,loadingPressedSurface);
			SDL_FreeSurface(loadingPressedSurface); // after the texture is created, we can free the surface (it is safe to pass NULL)

			button->textureForButtonOff = textureForButtonOff;
		}

		button->textureForButtonOn = textureForButtonOn;
		button->location           = copyRect(location);
		if ((textureForButtonOn == NULL) || ((buttonOff != NULL) && (textureForButtonOff == NULL)) || (button->location == NULL)) {
			free(button->location);
			SDL_DestroyTexture(textureForButtonOff);
			SDL_DestroyTexture(textureForButtonOn);
			free(button);
			free(widget);
			return NULL;
		}
	}
	else { // type == SP_BUTTON_BOARD_PIECE
		button->location = (SDL_Rect*) malloc(sizeof(SDL_Rect)); // allocate memory because we create one and then update the coordinates each time
		if (button->location == NULL) {
			free(button);
			free(widget);
			return NULL ;
		}
	}

	// updating the button fields and then the widget fields (which are it's function and the created button as its data)
	button->windowRenderer = renderer;
	button->type           = type;
	button->showButton     = showButton;
	button->isActive       = false;
	widget->drawWidget     = drawButton;
	widget->handleEvent    = handleButtonEvenet;
	widget->destroyWidget  = destroyButton;
	widget->data           = button;
	return widget;
}

/**
 * The function copies the source rectangle to a new rectangle.
 *
 * @param srcRect - The source rectangle
 *
 * @return
 * 		NULL      - If the source rectangle is NULL.
 * 		Otherwise - Returns a new rectangle with the same coordinates as the source rectangle.
 */
SDL_Rect* copyRect(SDL_Rect* srcRect) {
	if (srcRect == NULL)
		return NULL;

	SDL_Rect* dstRect = (SDL_Rect*) malloc(sizeof(SDL_Rect));
	if (dstRect == NULL)
		return NULL;

	dstRect->x = srcRect->x;
	dstRect->y = srcRect->y;
	dstRect->w = srcRect->w;
	dstRect->h = srcRect->h;

	return dstRect;
}

/**
 * The function draws the button (after casting the given widget's data).
 *
 * @param widget - The widget that its data is the button to draw
 */
void drawButton(SPWidget* widget) {
	if (widget == NULL)
		return;

	SPButton* button = (SPButton*) widget->data;

	// we only draw the button if it needs to be shown
	if (button->showButton) {
		if (button->textureForButtonOff == NULL)
			SDL_RenderCopy(button->windowRenderer, button->textureForButtonOn, NULL, button->location);
		else
			(button->isActive) ? SDL_RenderCopy(button->windowRenderer, button->textureForButtonOn , NULL, button->location)
							   : SDL_RenderCopy(button->windowRenderer, button->textureForButtonOff, NULL, button->location);
	}

}

/**
 * The function handles the given event for the given button.
 *
 * @param widget - The widget that its data is the button that its event should be handled
 * @param event  - The event to be handled.
 *
 * @return
 * 		True  - If the event is MOUSEBUTTONUP and the point on screen of the mouse is inside button's rectangle.
 * 		False - Otherwise.
 */
bool handleButtonEvenet(SPWidget* widget, SDL_Event* event) {
	if ((widget == NULL) || (event == NULL))
		return false;

	SPButton* button = (SPButton*) widget->data;
	if(!(button->showButton))
		return false;

	/* For the menu buttons we check the SDL_MOUSEBUTTONUP event,
		and for the board piece buttons we also check the SDL_MOUSEBUTTONDOWN and the SDL_MOUSEMOTION events */
	if (   (event->type == SDL_MOUSEBUTTONUP)
		|| ((button->type == SP_BUTTON_BOARD_PIECE) && ((event->type == SDL_MOUSEBUTTONDOWN) || (event->type == SDL_MOUSEMOTION)))) {

				SDL_Point point;
				point.x = event->button.x;
				point.y = event->button.y;

				if (SDL_PointInRect(&point, button->location))
					return true;
	}

	return false;
}

/**
 * The function frees all the memory associated with the given widget (including its associated button).
 *
 * @param - The widget that its memory should be destroyed
 */
void destroyButton(SPWidget* widget) {
	if (widget == NULL)
		return;

	SPButton* button = (SPButton*) widget->data;
	free(button->location);

	if (button->textureForButtonOff != NULL)
		SDL_DestroyTexture(button->textureForButtonOff);

	/* For piece widgets, this texture is already destroyed before, when the piece
	   textures are destroyed */
	if ((button->type != SP_BUTTON_BOARD_PIECE) && (button->textureForButtonOn != NULL))
		SDL_DestroyTexture(button->textureForButtonOn);

	free(button);
	free(widget);
}
