#include "SPChessGameWindow.h"

/**
 * The function creates the Game window in the gui mode.
 *
 * @param settings - The game settings for the game to be played in the Game window
 *
 * @return
 * 		NULL      - If there was a memory error.
 * 		Otherwise - Retuerns the Game window.
 */
SPGameWindow* gameWindowCreate(GameSetting* settings) {
	if (settings == NULL)
		return NULL;

	SPGameWindow* gameWindow = (SPGameWindow*) malloc(sizeof(SPGameWindow));
	SDL_Window*   window     = SDL_CreateWindow(GAME_TITLE            ,
												SDL_WINDOWPOS_CENTERED,
												SDL_WINDOWPOS_CENTERED,
			                                    GAME_WINDOW_WIDTH     ,
			                                    GAME_WINDOW_HEIGHT    ,
			                                    SDL_WINDOW_OPENGL      );

	SDL_Renderer* renderer = SDL_CreateRenderer(window, FIRST_RENDERER, SDL_RENDERER_ACCELERATED);
	SPWidget**    widgets  = gameWindowWidgetsCreate(renderer);

	SDL_Surface* loadingSurface = SDL_LoadBMP(CHESS_BOARD_IMAGE);
	SDL_Texture*  boardTexture  = SDL_CreateTextureFromSurface(renderer, loadingSurface);
	SDL_FreeSurface(loadingSurface);

	SDL_Texture** framesTextures = createFramesTextures(renderer);
	SDL_Texture** piecesTextures = createPiecesTextures(renderer);

	if ((!gameWindow) || (!window) || (!renderer) || (!widgets) || (!boardTexture) || (!framesTextures) || (!piecesTextures)) {
				for (int textureIndex = 0; textureIndex < TOTAL_PIECES_TYPES; textureIndex++) {
					if (piecesTextures[textureIndex] != NULL)
						SDL_DestroyTexture(piecesTextures[textureIndex]);
				}
				free(piecesTextures);
				for (int textureIndex = 0; textureIndex < TOTAL_FRAMES_TYPES; textureIndex++) {
					if (framesTextures[textureIndex] != NULL)
						SDL_DestroyTexture(framesTextures[textureIndex]);
				}
				free(framesTextures);
				for (int widgetIndex = 0; widgetIndex < GAME_WINDOW_TOTAL_WIDGETS; widgetIndex++)
					destroyWidget(widgets[widgetIndex]); // safe to pass NULL
				if (boardTexture != NULL)
					SDL_DestroyTexture(boardTexture);
				free(widgets);
				free(renderer);
				free(window);
				free(gameWindow);
				return NULL;
	}

	gameWindow->window             = window;
	gameWindow->renderer           = renderer;
	gameWindow->widgets            = widgets;
	gameWindow->boardTexture       = boardTexture;
	gameWindow->framesTextures     = framesTextures;
	gameWindow->piecesTextures     = piecesTextures;
	gameWindow->settings           = settings;
	gameWindow->possibleMoves      = NULL;
	gameWindow->actualNumOfWidgets = GAME_WINDOW_TOTAL_WIDGETS;
	gameWindow->firstCreated       = true;
	gameWindow->gameIsSaved        = false;

	return gameWindow;
}

/**
 * The function creates the widgets of the Game window.
 * At the creation of the window we create GAME_WINDOW_TOTAL_WIDGETS widgets,
 * and during the game we update the actual number of pieces (we do so for
 * not drawing piece widget if the piece is not on the board - happens when
 * not all pieces are still on board).
 *
 * @param renderer - The renderer of the Game window
 *
 * @return
 * 		NULL      - If there was a memory error.
 * 		Otherwise - Retuerns the widgets of the Settings window.
 */
SPWidget** gameWindowWidgetsCreate(SDL_Renderer* renderer) {
	if (renderer == NULL)
		return NULL;

	SPWidget** widgets = calloc(GAME_WINDOW_TOTAL_WIDGETS, sizeof(SPWidget*));
	if (widgets == NULL)
		return NULL ;

	// The location definitions for the menu widgets of the Game window
	SDL_Rect restartRect  = { .x = BUTTONS_SHIFT_GAME, .y = RESTART_TOP_BORDER  , .w = BUTTON_WIDTH_DEFAULT, .h = BUTTON_HEIGHT_DEFAULT };
	SDL_Rect saveRect     = { .x = BUTTONS_SHIFT_GAME, .y = SAVE_TOP_BORDER     , .w = BUTTON_WIDTH_DEFAULT, .h = BUTTON_HEIGHT_DEFAULT };
	SDL_Rect loadRect     = { .x = BUTTONS_SHIFT_GAME, .y = LOAD_TOP_BORDER_GAME, .w = BUTTON_WIDTH_DEFAULT, .h = BUTTON_HEIGHT_DEFAULT };
	SDL_Rect undoRect     = { .x = BUTTONS_SHIFT_GAME, .y = UNDO_TOP_BORDER     , .w = BUTTON_WIDTH_DEFAULT, .h = BUTTON_HEIGHT_DEFAULT };
	SDL_Rect mainMenuRect = { .x = BUTTONS_SHIFT_GAME, .y = MAIN_MENU_TOP_BORDER, .w = BUTTON_WIDTH_DEFAULT, .h = BUTTON_HEIGHT_DEFAULT };
	SDL_Rect exitRect     = { .x = BUTTONS_SHIFT_GAME, .y = EXIT_TOP_BORDER_GAME, .w = BUTTON_WIDTH_DEFAULT, .h = BUTTON_HEIGHT_DEFAULT };

	// The creation of the widgets (as a menu button) for the Game window
	widgets[RESTART_INDEX]   = createButton(renderer, &restartRect , RESTART_IMAGE    , NULL                 , true, SP_BUTTON_MENU);
	widgets[SAVE_INDEX]      = createButton(renderer, &saveRect    , SAVE_IMAGE       , NULL                 , true, SP_BUTTON_MENU);
	widgets[LOAD_INDEX_GAME] = createButton(renderer, &loadRect    , LOAD_IMAGE       , NULL                 , true, SP_BUTTON_MENU);
	widgets[UNDO_INDEX]      = createButton(renderer, &undoRect    , UNDO_ACTIVE_IMAGE, UNDO_NOT_ACTIVE_IMAGE, true, SP_BUTTON_MENU);
	widgets[MAIN_MENU_INDEX] = createButton(renderer, &mainMenuRect, MAIN_MENU_IMAGE  , NULL                 , true, SP_BUTTON_MENU);
	widgets[EXIT_INDEX_GAME] = createButton(renderer, &exitRect    , EXIT_IMAGE       , NULL                 , true, SP_BUTTON_MENU);

	for (int widgetIndex = FIRST_PIECE_INDEX; widgetIndex < GAME_WINDOW_TOTAL_WIDGETS; widgetIndex++)
		widgets[widgetIndex] = createButton(renderer, NULL, NULL, NULL, false, SP_BUTTON_BOARD_PIECE);

	for (int widgetIndex = 0; widgetIndex < GAME_WINDOW_TOTAL_WIDGETS; widgetIndex++) {
		if (widgets[widgetIndex] == NULL)
			return NULL;
	}

	return widgets;
}

/**
 * The function creates the frames textures (for the "get possible moves" option) of the Game window.
 *
 * @param renderer - The renderer of the Game window

 * @return
 * 		NULL      - If there was a memory error.
 * 		Otherwise - Retuerns the pieces textures of the Game window.
 */
SDL_Texture** createFramesTextures(SDL_Renderer* renderer) {
	if (renderer == NULL)
		return NULL;

	SDL_Texture** framesTextures = calloc(TOTAL_FRAMES_TYPES, sizeof(SDL_Texture*));
	if (framesTextures == NULL)
		return NULL;

	SDL_Surface* loadingSurface        = SDL_LoadBMP(RED_FRAME_IMAGE);
	framesTextures[RED_FRAME_INDEX]    = SDL_CreateTextureFromSurface(renderer, loadingSurface); // red frame's texture
	SDL_FreeSurface(loadingSurface);

	loadingSurface                     = SDL_LoadBMP(GREEN_FRAME_IMAGE);
	framesTextures[GREEN_FRAME_INDEX]  = SDL_CreateTextureFromSurface(renderer, loadingSurface); // green frame's texture
	SDL_FreeSurface(loadingSurface);

	loadingSurface                     = SDL_LoadBMP(BLUE_FRAME_IMAGE);
	framesTextures[BLUE_FRAME_INDEX]   = SDL_CreateTextureFromSurface(renderer, loadingSurface); // blue frame's texture
	SDL_FreeSurface(loadingSurface);

	loadingSurface                     = SDL_LoadBMP(YELLOW_FRAME_IMAGE);
	framesTextures[YELLOW_FRAME_INDEX] = SDL_CreateTextureFromSurface(renderer, loadingSurface); // yellow frame's texture
	SDL_FreeSurface(loadingSurface);

	for (int textureIndex = 0; textureIndex < TOTAL_FRAMES_TYPES; textureIndex++) {
		if (framesTextures[textureIndex] == NULL)
			return NULL;
	}

	return framesTextures;
}

/**
 * The function creates the pieces textures of the Game window.
 *
 * @param renderer - The renderer of the Game window

 * @return
 * 		NULL      - If there was a memory error.
 * 		Otherwise - Retuerns the pieces textures of the Game window.
 */
SDL_Texture** createPiecesTextures(SDL_Renderer* renderer) {
	if (renderer == NULL)
		return NULL;

	SDL_Texture** piecesTextures = calloc(TOTAL_PIECES_TYPES, sizeof(SDL_Texture*));
	if (piecesTextures == NULL)
		return NULL;

	createWhitePiecesTextures(renderer, piecesTextures); // the textures for the white pieces of the board
	createBlackPiecesTextures(renderer, piecesTextures); // the textutes for the black pieces of the board

	for (int textureIndex = 0; textureIndex < TOTAL_PIECES_TYPES; textureIndex++) {
		if (piecesTextures[textureIndex] == NULL)
			return NULL;
	}

	return piecesTextures;
}

/**
 * The function creates the pieces textures for the white player's pieces in the Game window.
 *
 * @param renderer - The renderer of the Game window
 *
 * @return
 * 		NULL      - If there was a memory error.
 * 		Otherwise - Retuerns the pieces textures of the Game window.
 */
void createWhitePiecesTextures(SDL_Renderer* renderer, SDL_Texture** piecesTextures) {
	SDL_Surface* loadingSurface        = SDL_LoadBMP(WHITE_PAWN_IMAGE);
	SDL_SetColorKey(loadingSurface, SDL_TRUE, SDL_MapRGB(loadingSurface->format, MAX_COLOR_TONE, MIN_COLOR_TONE, MAX_COLOR_TONE));
	piecesTextures[WHITE_PAWN_INDEX]   = SDL_CreateTextureFromSurface(renderer, loadingSurface); // white pawn's texture
	SDL_FreeSurface(loadingSurface);

	loadingSurface                     = SDL_LoadBMP(WHITE_KNIGHT_IMAGE);
	SDL_SetColorKey(loadingSurface, SDL_TRUE, SDL_MapRGB(loadingSurface->format, MAX_COLOR_TONE, MIN_COLOR_TONE, MAX_COLOR_TONE));
	piecesTextures[WHITE_KNIGHT_INDEX] = SDL_CreateTextureFromSurface(renderer, loadingSurface); // white knight's texture
	SDL_FreeSurface(loadingSurface);

	loadingSurface                     = SDL_LoadBMP(WHITE_BISHOP_IMAGE);
	SDL_SetColorKey(loadingSurface, SDL_TRUE, SDL_MapRGB(loadingSurface->format, MAX_COLOR_TONE, MIN_COLOR_TONE, MAX_COLOR_TONE));
	piecesTextures[WHITE_BISHOP_INDEX] = SDL_CreateTextureFromSurface(renderer, loadingSurface); // white bishop's texture
	SDL_FreeSurface(loadingSurface);

	loadingSurface                     = SDL_LoadBMP(WHITE_ROOK_IMAGE);
	SDL_SetColorKey(loadingSurface, SDL_TRUE, SDL_MapRGB(loadingSurface->format, MAX_COLOR_TONE, MIN_COLOR_TONE, MAX_COLOR_TONE));
	piecesTextures[WHITE_ROOK_INDEX]   = SDL_CreateTextureFromSurface(renderer, loadingSurface); // white rook's texture
	SDL_FreeSurface(loadingSurface);

	loadingSurface                     = SDL_LoadBMP(WHITE_QUEEN_IMAGE);
	SDL_SetColorKey(loadingSurface, SDL_TRUE, SDL_MapRGB(loadingSurface->format, MAX_COLOR_TONE, MIN_COLOR_TONE, MAX_COLOR_TONE));
	piecesTextures[WHITE_QUEEN_INDEX]  = SDL_CreateTextureFromSurface(renderer, loadingSurface); // white queen's texture
	SDL_FreeSurface(loadingSurface);

	loadingSurface                     = SDL_LoadBMP(WHITE_KING_IMAGE);
	SDL_SetColorKey(loadingSurface, SDL_TRUE, SDL_MapRGB(loadingSurface->format, MAX_COLOR_TONE, MIN_COLOR_TONE, MAX_COLOR_TONE));
	piecesTextures[WHITE_KING_INDEX]   = SDL_CreateTextureFromSurface(renderer, loadingSurface); // white king's texture
	SDL_FreeSurface(loadingSurface);
}

/**
 * The function creates the pieces textures for the Black player's pieces in the Game window.
 *
 * @param renderer - The renderer of the Game window
 *
 * @return
 * 		NULL      - If there was a memory error.
 * 		Otherwise - Retuerns the pieces textures of the Game window.
 */
void createBlackPiecesTextures(SDL_Renderer* renderer, SDL_Texture** piecesTextures) {
	SDL_Surface* loadingSurface        = SDL_LoadBMP(BLACK_PAWN_IMAGE);
	SDL_SetColorKey(loadingSurface, SDL_TRUE, SDL_MapRGB(loadingSurface->format, MAX_COLOR_TONE, MIN_COLOR_TONE, MAX_COLOR_TONE));
	piecesTextures[BLACK_PAWN_INDEX]   = SDL_CreateTextureFromSurface(renderer, loadingSurface); // black pawn's texture
	SDL_FreeSurface(loadingSurface);

	loadingSurface                     = SDL_LoadBMP(BLACK_KNIGHT_IMAGE);
	SDL_SetColorKey(loadingSurface, SDL_TRUE, SDL_MapRGB(loadingSurface->format, MAX_COLOR_TONE, MIN_COLOR_TONE, MAX_COLOR_TONE));
	piecesTextures[BLACK_KNIGHT_INDEX] = SDL_CreateTextureFromSurface(renderer, loadingSurface); // black knight's texture
	SDL_FreeSurface(loadingSurface);

	loadingSurface                     = SDL_LoadBMP(BLACK_BISHOP_IMAGE);
	SDL_SetColorKey(loadingSurface, SDL_TRUE, SDL_MapRGB(loadingSurface->format, MAX_COLOR_TONE, MIN_COLOR_TONE, MAX_COLOR_TONE));
	piecesTextures[BLACK_BISHOP_INDEX] = SDL_CreateTextureFromSurface(renderer, loadingSurface); // black bishop's texture
	SDL_FreeSurface(loadingSurface);

	loadingSurface                     = SDL_LoadBMP(BLACK_ROOK_IMAGE);
	SDL_SetColorKey(loadingSurface, SDL_TRUE, SDL_MapRGB(loadingSurface->format, MAX_COLOR_TONE, MIN_COLOR_TONE, MAX_COLOR_TONE));
	piecesTextures[BLACK_ROOK_INDEX]   = SDL_CreateTextureFromSurface(renderer, loadingSurface); // black rook's texture
	SDL_FreeSurface(loadingSurface);

	loadingSurface                     = SDL_LoadBMP(BLACK_QUEEN_IMAGE);
	SDL_SetColorKey(loadingSurface, SDL_TRUE, SDL_MapRGB(loadingSurface->format, MAX_COLOR_TONE, MIN_COLOR_TONE, MAX_COLOR_TONE));
	piecesTextures[BLACK_QUEEN_INDEX]  = SDL_CreateTextureFromSurface(renderer, loadingSurface); // black queen's texture
	SDL_FreeSurface(loadingSurface);

	loadingSurface                     = SDL_LoadBMP(BLACK_KING_IMAGE);
	SDL_SetColorKey(loadingSurface, SDL_TRUE, SDL_MapRGB(loadingSurface->format, MAX_COLOR_TONE, MIN_COLOR_TONE, MAX_COLOR_TONE));
	piecesTextures[BLACK_KING_INDEX]   = SDL_CreateTextureFromSurface(renderer, loadingSurface); // black king's texture
	SDL_FreeSurface(loadingSurface);
}

/**
 * The function draws the relevant Game window to the screen.
 *
 * @param gameWindow  - The Game window of the program
 * @param updateBoard - Indicates if the game board needs to be updated on the screen
 */
void gameWindowDraw(SPGameWindow* gameWindow, bool updateBoard) {
	if (gameWindow == NULL)
		return;

	// Draws the board on the screen
	SDL_Rect boardRec = { .x = BOARD_SHIFT, .y = BOARD_TOP_BORDER, .w = BOARD_WIDTH, .h = BOARD_HEIGHT };
	SDL_SetRenderDrawColor(gameWindow->renderer, MAX_COLOR_TONE, MAX_COLOR_TONE, MAX_COLOR_TONE, MAX_COLOR_TONE);
	SDL_RenderClear(gameWindow->renderer);
	SDL_RenderCopy(gameWindow->renderer, gameWindow->boardTexture, NULL, &boardRec);

	char piece;
	int widgetIndex        = FIRST_PIECE_INDEX;
	int currentNumOfPieces = 0;
	SPChessGame* game      = gameWindow->settings->game;
	SPWidget**   widgets   = gameWindow->widgets;

	if (updateBoard) { // If needed, updates the game board on the screen
		if (   (gameWindow->settings->gameMode == ONE_PLAYER_MODE)
			&& spChessGameIsUndoPossible(gameWindow->settings->game, gameWindow->settings->userColor)) {

				((SPButton*) gameWindow->widgets[UNDO_INDEX]->data)->isActive = true; // in case there an undo is possible we activate it
		}
		else
			((SPButton*) gameWindow->widgets[UNDO_INDEX]->data)->isActive = false;

		for (int row = 0; (row < BOARD_LENGTH); row++) {
			for (int col = 0; (col < BOARD_LENGTH); col++) {
				piece = game->board[row][col];

				if (piece != EMPTY_POSITION) { // if there is a board piece, we update the next widget with the piece's button
					SPButton* pieceButton = ((SPButton*) widgets[widgetIndex++]->data);
					pieceButton->location->x        = (BOARD_SHIFT + (col * POSITION_LENGTH));
					pieceButton->location->y        = ((BOARD_LAST_ROW_INDEX - row) * POSITION_LENGTH);
					pieceButton->location->w        = POSITION_LENGTH;
					pieceButton->location->h        = POSITION_LENGTH;
					pieceButton->textureForButtonOn = getPieceTexture(gameWindow->piecesTextures, piece); // getting the relevant image's texture
					pieceButton->showButton         = true;
					currentNumOfPieces++;
				}
			}
		}
		// we update the new number of actual widgets (the menu widgets + the current number of pieces on the board)
		gameWindow->actualNumOfWidgets = (GAME_WINDOW_MENU_WIDGETS + currentNumOfPieces);
	}

	SPArrayList* possibleMoves = gameWindow->possibleMoves;
	if (possibleMoves != NULL) {
		if (!spArrayListIsEmpty(possibleMoves)) // in case that there are possible moves to draw
			drawPossibleMoves(gameWindow);

		spArrayListDestroy(gameWindow->possibleMoves);
		gameWindow->possibleMoves = NULL;
	}

	for (int widgetIndex = 0; widgetIndex < gameWindow->actualNumOfWidgets; widgetIndex++) {
		SPWidget* widget = widgets[widgetIndex];
		widget->drawWidget(widget);
	}

	SDL_RenderPresent(gameWindow->renderer);
}

/**
 * The function returns the relevant piece texture (as a constant char) for the recievded board piece.
 *
 * @param piecesTextures - The piece textures array
 * @param piece          - A board piece
 *
 * @return
 * 		NULL      - If the recieved board piece is illegal.
 * 		Otherwise - The relevant piece texture.
 */
SDL_Texture* getPieceTexture(SDL_Texture** piecesTextures, char piece) {
	switch (piece) {
		case WHITE_PAWN:
			return piecesTextures[WHITE_PAWN_INDEX];

		case BLACK_PAWN:
			return piecesTextures[BLACK_PAWN_INDEX];

		case WHITE_KNIGHT:
			return piecesTextures[WHITE_KNIGHT_INDEX];

		case BLACK_KNIGHT:
			return piecesTextures[BLACK_KNIGHT_INDEX];

		case WHITE_BISHOP:
			return piecesTextures[WHITE_BISHOP_INDEX];

		case BLACK_BISHOP:
			return piecesTextures[BLACK_BISHOP_INDEX];

		case WHITE_ROOK:
			return piecesTextures[WHITE_ROOK_INDEX];

		case BLACK_ROOK:
			return piecesTextures[BLACK_ROOK_INDEX];

		case WHITE_QUEEN:
			return piecesTextures[WHITE_QUEEN_INDEX];

		case BLACK_QUEEN:
			return piecesTextures[BLACK_QUEEN_INDEX];

		case WHITE_KING:
			return piecesTextures[WHITE_KING_INDEX];

		case BLACK_KING:
			return piecesTextures[BLACK_KING_INDEX];
	}

	return NULL;
}

/**
 * The funtion draws on the Game window's board all the possible moves to play next:
 * 		- Red frame if it is a move that passes to a position that puts at the player's piece under threat.
 * 		- Blue frame if it is an "eat" move ("eating" a rival's piece) without being threatened at the new position.
 * 		- Yellow frame if it is a castle move.
 * 		- Green frame if it is a non-threatening move (i.e. neither of the ones mentioned above).
 */
void drawPossibleMoves(SPGameWindow* gameWindow) {
	SPArrayList* possibleMoves = gameWindow->possibleMoves;
	int numOfPossibleMoves = spArrayListSize(gameWindow->possibleMoves);
	char srcPiece = spArrayListGetAt(possibleMoves, 0)->srcPiece; // Get the piece from the first possible move

	for (int moveNum = 0; moveNum < numOfPossibleMoves; moveNum++) {
		Move* currentMove = spArrayListGetAt(possibleMoves, moveNum);
		int dstCol    = currentMove->dstCol;
		int dstRow    = currentMove->dstRow;

		SDL_Rect dstPosRect = { .x = (BOARD_SHIFT + (dstCol * POSITION_LENGTH))         ,
								.y = ((BOARD_LAST_ROW_INDEX - dstRow) * POSITION_LENGTH),
							    .w = POSITION_LENGTH                                    ,
							    .h = POSITION_LENGTH                                     };


		// The frames are being showes as explained in the funtion description
		if (currentMove->threatenedAfterMove)
			SDL_RenderCopy(gameWindow->renderer, gameWindow->framesTextures[RED_FRAME_INDEX] , NULL, &dstPosRect);
		else if (currentMove->dstPieceCaptured)
			SDL_RenderCopy(gameWindow->renderer, gameWindow->framesTextures[BLUE_FRAME_INDEX], NULL, &dstPosRect);
		else if (currentMove->castleMove) {
			switch (srcPiece) {
				case WHITE_KING:
				case BLACK_KING:
					dstPosRect.x = (BOARD_SHIFT + (currentMove->srcCol * POSITION_LENGTH));
				    dstPosRect.y = (srcPiece == BLACK_KING) ? BOARD_FIRST_ROW_INDEX                     // The relevant black rook's row
				    										: (BOARD_LAST_ROW_INDEX * POSITION_LENGTH); // The relevant white rook's row

				    break;

				case WHITE_ROOK:
				case BLACK_ROOK:
				    dstPosRect.x = (BOARD_SHIFT + (KING_COL * POSITION_LENGTH));
				    dstPosRect.y = (srcPiece == BLACK_ROOK) ? BOARD_FIRST_ROW_INDEX                     // The relevant black king's row
				    										: (BOARD_LAST_ROW_INDEX * POSITION_LENGTH);	// The relevant white king's row
			}
			SDL_RenderCopy(gameWindow->renderer, gameWindow->framesTextures[YELLOW_FRAME_INDEX], NULL, &dstPosRect);
		}
		else
			SDL_RenderCopy(gameWindow->renderer, gameWindow->framesTextures[GREEN_FRAME_INDEX], NULL, &dstPosRect);
	}
}

/**
 * The function handles the event that occurred in the Game window.
 *
 * @param gameWindow - The Game window of the program
 * @param event      - The event to handle
 *
 * @return
 * 		SP_GAME_EVENT_EXIT - If the "X" button (at the window's corner) was clicked.
 * 		Otherwise          - Returns the SP_GAME_EVENT that returned from the relevant helper function if a "MOUSEBUTTONUP" event or
 * 		                      a "MOUSEBUTTONDOWN" event occurred. If non of them occurred, returns SP_GAME_EVENT_NONE as a default value.
 */
SP_GAME_EVENT gameWindowHandleEvent(SPGameWindow* gameWindow, SDL_Event* event) {
	if ((gameWindow == NULL) || (event == NULL))
		return SP_GAME_EVENT_INVALID_ARGUMENT;

	switch (event->type) {
		case SDL_MOUSEBUTTONUP:
			return handleEventMouseButtonUp(gameWindow, event);

		case SDL_MOUSEBUTTONDOWN:
			return handleEventMouseButtonDown(gameWindow, event);

		case SDL_WINDOWEVENT:
			if (event->window.event == SDL_WINDOWEVENT_CLOSE)
				return SP_GAME_EVENT_EXIT;

			break;

		default:
			return SP_GAME_EVENT_NONE;
	}

	return SP_GAME_EVENT_NONE;
}

/**
 * The function handles the event "MOUSEBUTTONUP" (relevant to the menu buttons of the Game window).
 *
 * @param gameWindow - The Game window of the program
 * @param event      - A pointer the current "MOUSEBUTTONUP" event
 *
 * @return
 * 		SP_GAME_EVENT_RESET     - If the "RESTART" button was clicked.
 * 		SP_GAME_EVENT_SAVE      - If the "SAVE" button was clicked.
 *		SP_GAME_EVENT_LOAD      - If the "LOAD" button was clicked.
 * 		SP_GAME_EVENT_UPDATE    - If the "UNDO" button (when it is active).
 *		SP_GAME_EVENT_MAIN_MENU - If the "MAIN MENU" button was clicked.
 *		SP_GAME_EVENT_NONE      - Default value.
 */
SP_GAME_EVENT handleEventMouseButtonUp(SPGameWindow* gameWindow, SDL_Event* event) {
	SPWidget* restartWidget  = gameWindow->widgets[RESTART_INDEX];
	SPWidget* saveWidget     = gameWindow->widgets[SAVE_INDEX];
	SPWidget* loadWidget     = gameWindow->widgets[LOAD_INDEX_GAME];
	SPWidget* undoWidget     = gameWindow->widgets[UNDO_INDEX];
	SPWidget* mainMenuWidget = gameWindow->widgets[MAIN_MENU_INDEX];
	SPWidget* exitWidget     = gameWindow->widgets[EXIT_INDEX_GAME];

	if (restartWidget->handleEvent(restartWidget, event)) {
		spChessGameDestroy(gameWindow->settings->game);
		gameWindow->settings->game = NULL;
		gameWindow->settings->game = spChessGameCreate(HISTORY_SIZE);

		gameWindow->gameIsSaved = false;
		return SP_GAME_EVENT_RESET_GAME;
	}

	if (saveWidget->handleEvent(saveWidget, event))
		return SP_GAME_EVENT_SAVE;

	if (loadWidget->handleEvent(loadWidget, event)) {
		gameWindow->gameIsSaved = true;
		return SP_GAME_EVENT_LOAD;
	}

	if (undoWidget->handleEvent(undoWidget, event)) {
		if ((gameWindow->settings->gameMode == ONE_PLAYER_MODE) && (((SPButton*) gameWindow->widgets[UNDO_INDEX]->data)->isActive)) {
			// undoes the last user's move
			Move* userMove = spArrayListGetLast(gameWindow->settings->game->history);
			spChessGameUndoPrevMove(gameWindow->settings->game, userMove);

			// undoes the last computer's move
			Move* computerMove = spArrayListGetLast(gameWindow->settings->game->history);
			spChessGameUndoPrevMove(gameWindow->settings->game, computerMove);

			gameWindow->gameIsSaved = false;
			return SP_GAME_EVENT_UPDATE;
		}

		return SP_GAME_EVENT_NONE;
	}

	if (mainMenuWidget->handleEvent(mainMenuWidget, event))
		return SP_GAME_EVENT_MAIN_MENU;

	if (exitWidget->handleEvent(exitWidget, event))
		return SP_GAME_EVENT_EXIT;

	return SP_GAME_EVENT_NONE;
}

/**
 * The function handles the event "MOUSEBUTTONDOWN" (relevant to the game board area of the screen).
 *
 * @param gameWindow - The Game window of the program
 * @param event      - A pointer the current "MOUSEBUTTONDOWN" event
 *
 * @return
 * 		SP_GAME_EVENT_NONE   - If an illegal piece was clicked or if a right click occured.
 * 		SP_GAME_EVENT_EXIT   - If after proccesing the event the game is finished.
 * 		SP_GAME_EVENT_UPDATE - Otherwise.
 */
SP_GAME_EVENT handleEventMouseButtonDown(SPGameWindow* gameWindow, SDL_Event* event) {
	SPWidget* pieceWidget = NULL;
	Move* userMove = spCreateMove();

	// First we check if a legal board piece was clicked (menu buttons are handled in the MOUSEBUTTONUP event handling function)
	if (!isLegalPieceClicked(gameWindow, &pieceWidget, event, userMove)) {
		free(userMove);
		return SP_GAME_EVENT_NONE;
	}

	if (event->button.button != SDL_BUTTON_RIGHT) { // i.e. the user is trying to set a move
		SP_GAME_EVENT moveResult = processMoves(gameWindow, pieceWidget, userMove);
		free(userMove);
		return moveResult;
	}
	else {
		/**
		 * If the user clicked the right button on the mouse - we calculate the possible moves for the requested board piece,
		 * only if it is legal to ask that (i.e. the game mode is "one player" and the difficulty is "Noob" or "Easy"
		 */
		GameSetting* settings = gameWindow->settings;
		if ((settings->gameMode == ONE_PLAYER_MODE) && (settings->difficulty <= EASY_LEVEL)) {
			gameWindow->possibleMoves = spArrayListCreate(MAX_MOVES_FOR_PIECE);
			spChessGetPossibleMoves(gameWindow->settings->game, gameWindow->possibleMoves, userMove->srcRow, userMove->srcCol);
			gameWindowDraw(gameWindow, false);
		}
	}

	free(userMove);
	return SP_GAME_EVENT_NONE;
}

/**
 * The function checks if a legal piece was clicked (i.e. a piece that belongs to the current player).
 *
 * @param gameWindow        - The Game window of the program
 * @param pieceWidget       - A pointer to a widget pointer (that will be assigned with the relevant widget that its piece has been clicked)
 * @param event             - The current event that occurred in the Game window
 * @param currentPlayerMove - The player that is currently playing a move (either the user in "One Player" mode or one of the
 *                             black/white player in the "Two Players" mode
 *
 * @return
 * 		true  - If the curent player did clicked one of its pieces
 * 		false - Otherwise.
 */
bool isLegalPieceClicked(SPGameWindow* gameWindow, SPWidget** pieceWidget, SDL_Event* event, Move* currentPlayerMove) {
	int srcRow, srcCol;
	SPChessGame* game = gameWindow->settings->game;

	for (int widgetIndex = FIRST_PIECE_INDEX; widgetIndex < gameWindow->actualNumOfWidgets; widgetIndex++) {
		*(pieceWidget) = gameWindow->widgets[widgetIndex];

		if ((*(pieceWidget))->handleEvent(*(pieceWidget), event)) {
			srcRow = (BOARD_LAST_ROW_INDEX - (event->button.y / POSITION_LENGTH));
			srcCol = ((event->button.x - BOARD_SHIFT) / POSITION_LENGTH);

			if (currentPlayerPiece(game->currentPlayer, game->board[srcRow][srcCol])) {
				currentPlayerMove->srcRow = srcRow;
				currentPlayerMove->srcCol = srcCol;
				return true;
			}
		}
	}

	return false;
}

/**
 * The function process the current player move, and then calls the relevant
 * helper function according to the game mode.
 *
 * @param gameWindow  - The Game window of the program
 * @param pieceWidget - The widget that represents the position on the game board that was clicked
 * @param userMove    - The move to be executed by the user
 *
 * @return
 * 		SP_GAME_EVENT_EXIT   - If the game ended after proccesing the relevant moves.
 * 		SP_GAME_EVENT_QUIT   - If a memory error occurred.
 *		SP_GAME_EVENT_UPDATE - Otherwise.
 */
SP_GAME_EVENT processMoves(SPGameWindow* gameWindow, SPWidget* pieceWidget, Move* userMove) {
	SDL_Event eventmotion;
	handleEventMouseMotion(gameWindow, pieceWidget, &eventmotion);

	// update the destination position coardinates
	userMove->dstRow  = (BOARD_LAST_ROW_INDEX - (eventmotion.button.y / POSITION_LENGTH));
	userMove->dstCol  = ((eventmotion.button.x - BOARD_SHIFT) / POSITION_LENGTH);
	SPChessGame* game = gameWindow->settings->game;

	// helper function if the user is trying to castle
	updateMoveIfCastle(game->board, userMove, userMove->srcRow, userMove->srcCol, userMove->dstRow, userMove->dstCol);

	SP_CHESS_GAME_MESSAGE returnedMessage = spChessGameSetMove(game, userMove, false, false); // The execution of the current player move
	if (returnedMessage == SP_CHESS_GAME_SUCCESS) {
		gameWindowDraw(gameWindow, true);

		if (userMove->pawnPromotion) {
			bool promotionMemoryError = isPawnPromotionMemoryFailling(game, userMove); // The execution of the pawn promotion move

			if (promotionMemoryError) {
				MEMORY_ALLOCATION_PERROR;
				return SP_GAME_EVENT_QUIT;
			}
			gameWindowDraw(gameWindow, true); // if pawn promotion succeeded we draw the Game window again with the updated piece
		}

		gameWindow->gameIsSaved = false;
		int gameMode = gameWindow->settings->gameMode;

		switch (gameMode) { // continue according to the game mode
			case ONE_PLAYER_MODE:
				return gameFinishedOnePlayersMode(gameWindow);

			case TWO_PLAYERS_MODE:
				if (gameFinishedTwoPlayersMode(game->status)) {
					gameWindow->gameIsSaved = true; // needed so we won't save the game
					return SP_GAME_EVENT_EXIT;
				}
		}
	}
	else if (returnedMessage == SP_CHESS_GAME_MEMORY_FAILURE) {
		MEMORY_ALLOCATION_PERROR;
		return SP_GAME_EVENT_QUIT;
	}

	return SP_GAME_EVENT_UPDATE;
}

/**
 * The function is implementing the the "Drag and Drop" feature, in a way that the
 * cursor always appears in the center of the draged piece (while it is still draged).
 *
 * @param gameWindow  - The Game window of the program
 * @param pieceWidget - The widget that represents the draged piece
 * @eventmotion       - A pointer that will be updated each time a "MOUSEMOTION" event will occur
 */
void handleEventMouseMotion(SPGameWindow* gameWindow, SPWidget* pieceWidget, SDL_Event* eventmotion) {
	SDL_WaitEvent(eventmotion);

	while (eventmotion->type == SDL_MOUSEMOTION) {
		((SPButton*) pieceWidget->data)->location->x = (eventmotion->motion.x - MOTION_RATIO); // Centering the piece's column according to the cursor
		((SPButton*) pieceWidget->data)->location->y = (eventmotion->motion.y - MOTION_RATIO); // Centering the piece's row according to the cursor
		gameWindowDraw(gameWindow, false);
		SDL_WaitEvent(eventmotion);
	}
}

/**
 * The function checks if the user is trying to make a castle move,
 * and if so - updates the relevant fields of the move.
 *
 * @param board    - The current game board
 * @param userMove - The move that the user is trying to play
 * @param srcRow   - The row of the piece that the player draged over the screen
 * @param srcCol   - The column of the piece that the player draged over the screen
 * @param dstRow   - The row of the position that the player draged the piece to
 * @param dstCol   - The column of the position that the player draged the piece to
 */
void updateMoveIfCastle(char board[][BOARD_LENGTH], Move* userMove, int srcRow, int srcCol, int dstRow, int dstCol) {
	char srcPiece = board[srcRow][srcCol];
	char dstPiece = board[dstRow][dstCol];

	if (   ((srcPiece == WHITE_ROOK) && (dstPiece == WHITE_KING))
		|| ((srcPiece == BLACK_ROOK) && (dstPiece == BLACK_KING))) {

			// In case the user draged the rook over the king
			userMove->castleMove = true;
	}
	else if (   ((srcPiece == WHITE_KING) && (dstPiece == WHITE_ROOK))
			 || ((srcPiece == BLACK_KING) && (dstPiece == BLACK_ROOK))) {

				// In case the user draged the king over the rook
				userMove->castleMove = true;
				userMove->srcRow     = dstRow;
				userMove->srcCol     = dstCol;
				userMove->dstRow     = srcRow;
				userMove->dstCol     = srcCol;
	}
}

/**
 * The function executes a "Pawn Promotion" move, by asking the
 * user for a piece to promote to and updates the board.
 * We look for a memory error after the execution because we
 * want to terminate the program only if a such an error occurred,
 * but not if another error occurred (during the message box process).
 *
 * @param game     - The current Chess game
 * @param userMove - The move current move that was played by the user
 *
 * @return
 * 		True  - If a memory error occurred during the execution of the "Pawn Promotion" move.
 * 		False - Otherwise.
 */
bool isPawnPromotionMemoryFailling(SPChessGame* game, Move* userMove) {
	const SDL_MessageBoxButtonData promotionButtons[] = { { 0, PAWN_BUTTON_MESSAGE_INDEX  , PAWN_BUTTON_MESSAGE   },
														  { 0, KNIGHT_BUTTON_MESSAGE_INDEX, KNIGHT_BUTTON_MESSAGE },
														  { 0, BISHOP_BUTTON_MESSAGE_INDEX, BISHOP_BUTTON_MESSAGE },
														  { 0, ROOK_BUTTON_MESSAGE_INDEX  , ROOK_BUTTON_MESSSAGE  },
														  { 0, QUEEN_BUTTON_MESSAGE_INDEX ,  QUEEN_BUTTON_MESSAGE } };

	const SDL_MessageBoxData messageboxdata = { SDL_MESSAGEBOX_INFORMATION     ,
												NULL                           ,
												GAME_TITLE                     ,
												USER_PAWN_PROMOTION_MESSAGE    ,
												SDL_arraysize(promotionButtons),
												promotionButtons               ,
												NULL                            };

	char newPiece = NULL_CHARACTER;

	int buttonID;
	if (SDL_ShowMessageBox(&messageboxdata, &buttonID) == 0) {
		int currentPlayer = game->currentPlayer;

		switch (buttonID) {
			case PAWN_BUTTON_MESSAGE_INDEX:
				newPiece = (currentPlayer == WHITE_PLAYER) ? WHITE_PAWN   : BLACK_PAWN;
				break;

			case KNIGHT_BUTTON_MESSAGE_INDEX:
				newPiece = (currentPlayer == WHITE_PLAYER) ? WHITE_KNIGHT : BLACK_KNIGHT;
				break;

			case BISHOP_BUTTON_MESSAGE_INDEX:
				newPiece = (currentPlayer == WHITE_PLAYER) ? WHITE_BISHOP : BLACK_BISHOP;
				break;

			case ROOK_BUTTON_MESSAGE_INDEX:
				newPiece = (currentPlayer == WHITE_PLAYER) ? WHITE_ROOK   : BLACK_ROOK;
				break;

			case QUEEN_BUTTON_MESSAGE_INDEX:
				newPiece = (currentPlayer == WHITE_PLAYER) ? WHITE_QUEEN  : BLACK_QUEEN;
		}

		if (newPiece != NULL_CHARACTER) { // pawn promoting
			userMove->srcPiece = newPiece;
			return (spChessGameSetPawnPromotion(game, userMove, false) == SP_CHESS_GAME_MEMORY_FAILURE);
		}
	}

	MESSAGEBOX_ERROR_ON_PAWN_PAROMOTION;
	return false;
}

/**
 * The function checks if the game is finished (in the "One Player" mode):
 * If it did - prompts a relevant message, if it didn't - calls to
 * a helper function tp execute the next computer move.
 *
 * @param gmaWindow - The Game window of the program
 *
 * @return
 * 		SP_GAME_EVENT_EXIT - If the game is finished
 * 		Otherwise          - Returns what came back from the helper function executeCompterMove.
 */
SP_GAME_EVENT gameFinishedOnePlayersMode(SPGameWindow* gameWindow) {
	GAME_STATUS gameStatus = gameWindow->settings->game->status;

	switch (gameStatus) {
		case WHITE_PLAYER_WINS:
		case BLACK_PLAYER_WINS:
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, GAME_TITLE, USER_WON_MESSAGE, NULL);
			gameWindow->gameIsSaved = true; // needed so we won't save the game
			return SP_GAME_EVENT_EXIT;

		case TIED_GAME:
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, GAME_TITLE, TIE_GAME_MESSAGE, NULL);
			gameWindow->gameIsSaved = true; // needed so we won't save the game
			return SP_GAME_EVENT_EXIT;

		case GAME_NOT_FINISHED_CHECK:
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, GAME_TITLE, CHECK_MESSAGE   , NULL);
			break;

		default:
			break;
	}

	return executeCompterMove(gameWindow);
}

/**
 * The function executes the computer move chosen by the Minimax algorithm,
 * and if after it the game is finished - prompts a relevant message.
 *
 * @param gmaWindow - The Game window of the program
 *
 * @return
 *		SP_GAME_EVENT_QUIT   - If a memory error occurred.
 *		SP_GAME_EVENT_EXIT   - If the compter's move finished the game.
 *		SP_GAME_EVENT_UPDATE - Otherwise.
 */
SP_GAME_EVENT executeCompterMove(SPGameWindow* gameWindow) {
	SPChessGame* game = gameWindow->settings->game;

	Move* computerMove = spChessMinimaxMove(game, gameWindow->settings->difficulty);
	if (computerMove == NULL) {
		MEMORY_ALLOCATION_PERROR;
		return SP_GAME_EVENT_QUIT;
	}

	SP_CHESS_GAME_MESSAGE returnedMessage = spChessGameSetMove(game, computerMove, false, true); // set the computer move
	if (returnedMessage == SP_CHESS_GAME_SUCCESS) {
		gameWindowDraw(gameWindow, true);

		// If the computer has made a pawn promtion we prompts a relevant massagebox to the screen
		if (computerMove->pawnPromotion) {
			char computerNewPiece[SP_MAX_LINE_LENGTH];
			char* srcPiece = formatSrcPiece(computerMove->srcPiece);
			sprintf(computerNewPiece, COMPUTER_PAWN_PROMOTION_MESSAGE, srcPiece);
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, GAME_TITLE, computerNewPiece, NULL);
		}

		GAME_STATUS gameStatus = game->status;
		switch (gameStatus) {
			case WHITE_PLAYER_WINS:
			case BLACK_PLAYER_WINS:
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, GAME_TITLE, COMPUTER_WON_MESSAGE, NULL);
				gameWindow->gameIsSaved = true;
				free(computerMove);
				return SP_GAME_EVENT_EXIT;

			case TIED_GAME:
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, GAME_TITLE, TIE_GAME_MESSAGE    , NULL);
				gameWindow->gameIsSaved = true;
				free(computerMove);
				return SP_GAME_EVENT_EXIT;

			case GAME_NOT_FINISHED_CHECK:
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, GAME_TITLE, CHECK_MESSAGE       , NULL);
				break;

			default:
				break;
		}
	}
	else if (returnedMessage == SP_CHESS_GAME_MEMORY_FAILURE) {
		free(computerMove);
		MEMORY_ALLOCATION_PERROR;
		return SP_GAME_EVENT_QUIT;
	}

	free(computerMove);
	return SP_GAME_EVENT_UPDATE;
}

/**
 * The function checks if the game is finished (in the "Two Players" mode),
 * and if it did - prompts a relevant message;
 *
 * @param gameStatus - The current status of the game after the las move was executed
 *
 * @return
 * 		true  - If the game is finished
 * 		false - Otherwise.
 */
bool gameFinishedTwoPlayersMode(GAME_STATUS gameStatus) {
	switch (gameStatus) {
		case WHITE_PLAYER_WINS:
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, GAME_TITLE, WHITE_PLAYER_WON_MESSAGE, NULL);
			return true;

		case BLACK_PLAYER_WINS:
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, GAME_TITLE, BLACK_PLAYER_WON_MESSAGE, NULL);
			return true;

		case TIED_GAME:
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, GAME_TITLE, TIE_GAME_MESSAGE        , NULL);
			return true;

		case GAME_NOT_FINISHED_CHECK:
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, GAME_TITLE, CHECK_MESSAGE           , NULL);
			break;

		default:
			break;
	}

	return false;
}

/**
 * The function frees all the memory of the Game window.
 *
 * @param gameWindow - The Game window of the program
 */
void gameWindowDestroy(SPGameWindow* gameWindow) {
	if (gameWindow == NULL)
		return;

	spArrayListDestroy(gameWindow->possibleMoves);
	destroySettings(gameWindow->settings);

	if (gameWindow->piecesTextures != NULL) {
		for (int textureIndex = 0; textureIndex < TOTAL_PIECES_TYPES; textureIndex++) {
			if (gameWindow->piecesTextures[textureIndex] != NULL)
				SDL_DestroyTexture(gameWindow->piecesTextures[textureIndex]);
		}
		free(gameWindow->piecesTextures);
	}

	if ((gameWindow->framesTextures != NULL)) {
		for (int textureIndex = 0; textureIndex < TOTAL_FRAMES_TYPES; textureIndex++) {
			if (gameWindow->framesTextures[textureIndex] != NULL)
				SDL_DestroyTexture(gameWindow->framesTextures[textureIndex]);
		}
		free(gameWindow->framesTextures);
	}

	if (gameWindow->boardTexture != NULL)
		SDL_DestroyTexture(gameWindow->boardTexture);

	if (gameWindow->widgets != NULL) {
		for (int widgetIndex = 0; widgetIndex < GAME_WINDOW_TOTAL_WIDGETS; widgetIndex++)
			destroyWidget(gameWindow->widgets[widgetIndex]); // safe to pass NULL
		free(gameWindow->widgets);
	}

	if (gameWindow->renderer != NULL)
		SDL_DestroyRenderer(gameWindow->renderer);

	if (gameWindow->window != NULL)
		SDL_DestroyWindow(gameWindow->window);

	free(gameWindow);
}
