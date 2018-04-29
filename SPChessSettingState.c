#include "SPChessSettingState.h"

/**
 * The function creates a copy of a given settings (excepts the game that is .
 *
 * @param src      - The source settings to copy
 * @param copyGame - Indicates if the game should also be copied or not
 *
 * @return
 * 		NULL if either src is NULL or a memory allocation failure occurred.
 * 		Otherwise, a new copy of the source game is returned.
 */
GameSetting* copySettings(GameSetting* src, bool copyGame) {
	if (src == NULL)
		return NULL;

	GameSetting* dest = (GameSetting*) malloc(sizeof(GameSetting));
	if (dest == NULL)
		return NULL;

	dest->gameMode     = src->gameMode;
	dest->difficulty   = src->difficulty;
	dest->userColor    = src->userColor;
	dest->isGameLoaded = src->isGameLoaded;

	if (copyGame) {
		SPChessGame* game = spChessGameCopy(src->game);

		if (game == NULL) {
			destroySettings(dest);
			return NULL;
		}

		dest->game = game;
	}
	else
		dest->game = NULL;

	return dest;
}

/**
 * This function destroys the given settings (frees its asscociated memory).
 *
 * @param setting - The settings to destroy
 */
void destroySettings(GameSetting* setting) {
	if (setting == NULL)
		return;

	if (setting->game != NULL)
		spChessGameDestroy(setting->game);
	free(setting);
}

/**
 * This function sets the game mode and prints the proper message.
 *
 * @param setting - the game setting to be updated
 * @param command - the command that the user has entered
 * @precondition setting != NULL
 * @precondition command != NULL, command.cmd == SP_GAME_MODE
 */
void setGameMode(GameSetting* setting, SPCommand command){
	if (!command.validIntArg || (command.intArg != ONE_PLAYER_MODE && command.intArg != TWO_PLAYERS_MODE) )
		GAME_MODE_PERROR;
	else { //integer argument of the command is 1 or 2
		(command.intArg == ONE_PLAYER_MODE) ? PRINT_GAME_MODE_1_PLAYER : PRINT_GAME_MODE_2_PLAYERS;
		setting->gameMode = command.intArg;
	}
}

/**
 * This function sets the difficulty level of the game.
 * If the integer argument of the command is not valid
 * or not in the proper range, then it prints an error.
 *
 * @param setting - the game setting to be updated
 * @param command - the command that the user has entered
 * @precondition setting != NULL
 * @precondition command != NULL, command.cmd == command.cmd == SP_DIFFICULTY
 */
void setDifficulty(GameSetting* setting, SPCommand command){
	if ( !command.validIntArg || command.intArg < NOOB_LEVEL || command.intArg > EXPERT_LEVEL )
		DIFFICULTY_PERROR;
	else if (command.intArg == EXPERT_LEVEL)
		DIFFICULTY_EXPERT_NOT_SUPPORTED_PERROR;
	else //difficulty is in the proper range
		setting->difficulty = command.intArg;
}

/**
 * Resets the game setting to the default values:
 * The game mode 					- the default value is 1 (1-player mode)
 * The difficulty level of the game - the default value is 2 (easy level)
 * User color						- the default value is 1 (white)
 *
 * @param setting - the game setting to be updated
 * @precondition setting != NULL
 */
void setDefaultValues(GameSetting* setting){
	setting->gameMode = ONE_PLAYER_MODE;
	setting->difficulty = EASY_LEVEL;
	setting->userColor = WHITE_PLAYER;
	setting->isGameLoaded = GAME_NOT_LOADED_YET;
	setting->game = NULL;
}

/***
 * Prints the current game setting to the console in the suitable format.
 *
 * @param setting - the game setting to be printed
 * @precondition setting != NULL
 */
void printSetting(GameSetting* setting){
	if (setting->gameMode == ONE_PLAYER_MODE)
		PRINT_SETTING_1_PLAYER(setting->difficulty, setting->userColor ? WHITE_PLAYER_UPPER_CASE_STRING : \
				BLACK_PLAYER_UPPER_CASE_STRING);
	else
		PRINT_SETTING_2_PLAYERS;
}

/*
 * This function prompts for the user's commands.
 * Once the user enters the start command, the program switches to the game state
 * with the specified setting.
 *
 * @param setting - the game setting to be updated
 * @precondition setting != NULL
 *
 * @return the last command type that was entered before finishing with the setting state.
 */
SP_COMMAND SettingState(GameSetting* setting){
	SPCommand command;
	char str[SP_MAX_LINE_LENGTH];

	setDefaultValues(setting);
	PRINT_SETTING_STATE_MESSAGE;
	do {
		fgets(str, SP_MAX_LINE_LENGTH, stdin);
		command = spParserParseLine(str, true);
		settingCommandHandler(setting, command);
	} while (command.cmd != SP_QUIT && command.cmd != SP_START
			&& !(command.cmd == SP_LOAD && setting->isGameLoaded == MEMORY_ALLOCATION_ERROR_DURING_LOADING));
	return command.cmd;
}

/**
 * Given the user's command, this function handles it and updates the game setting accordingly.
 * After executing the command, the function prints a suitable message.
 *
 * @param setting - the game setting to be updated
 * @param command - the user's command, after parsing it.
 * @precondition setting != NULL
 * @precondition command != NULL
 */
void settingCommandHandler(GameSetting* setting, SPCommand command){
	if (command.cmd == SP_GAME_MODE)
		setGameMode(setting, command);

	else if (command.cmd == SP_DIFFICULTY && setting->gameMode == ONE_PLAYER_MODE)
		setDifficulty(setting, command);

	else if (command.cmd == SP_USER_COLOR && setting->gameMode == ONE_PLAYER_MODE)
		setting->userColor = command.intArg; // we assume that the int argument is valid, according to Moab's message in the forum

	else if (command.cmd == SP_LOAD)
		loadGame((command.validStrArg ? command.strArg : NULL), setting);

	else if (command.cmd == SP_PRINT_SETTING)
		printSetting(setting);

	else if (command.cmd == SP_DEFAULT)
		setDefaultValues(setting);

	else if (command.cmd == SP_QUIT)
		PRINT_QUIT_MESSAGE;

	else if (command.cmd != SP_START)
		INVALID_COMMAND_PERROR;
}

/**
 * This function loads the game setting from a file and updates
 * the game setting that were sent to the function.
 *
 * @param fileName  the fileName specified by the user
 * @param setting   the game setting to be updated
 * @precondition 	if the file name is valid - the file contains valid data and is correctly formatted.
 */
void loadGame(const char* fileName, GameSetting* setting){
	FILE * file = ((fileName==NULL) ? NULL : fopen(fileName, "r"));
	if (file == NULL){ //if file doesn't exist or cannot be opened
		LOAD_PERROR;
		return;
	}
	setting->game = spChessGameCreate(HISTORY_SIZE); // create a new game inside the structure of the setting
	if (setting->game == NULL){
		MEMORY_ALLOCATION_PERROR;
		setting->isGameLoaded = MEMORY_ALLOCATION_ERROR_DURING_LOADING;
		fclose(file);
		return;
	}

	char str[SP_MAX_LINE_LENGTH];
	bool isDoneLoading = false;
	int row=BOARD_LENGTH-1;

	while ((fscanf(file, "%s", str) == 1) && !hasReachedTheNextTag(str ,GAME_OPENING_TAG)){
		//the scanning to str is until we reach the game opening tag
	}
	while (fscanf(file, "%s", str) == 1){
		//inside game tag, we keep looping until we reach the board opening tag
		int status = gameSettingParser(setting, str);
		if (status == LOAD_TERMINATE){
			LOAD_EXPERT_LEVEL_NOT_SUPPORTED_PERROR;
			fclose(file);
			return;
		}
		if (status == LOAD_EXIT_LOOP)
			break;
	}

	nullifyArmies(setting->game); // we reset the army states of each player before the board tag is scanned
	while ((fscanf(file, "%s", str) == 1) && (boardRowParser(setting, str, row) != LOAD_EXIT_LOOP)){
		row--; //inside board tag, we keep looping until we reach the board closing tag
	}

	while ((fscanf(file, "%s", str) == 1) && !(hasReachedTheNextTag(str ,GENERAL_OPENING_TAG))){
		//the scanning to str is until we reach the general opening tag
		if (hasReachedTheNextTag(str , GAME_CLOSING_TAG)){
			loadGameWithoutGeneral(setting, setting->game);
			if (setting->isGameLoaded == MEMORY_ALLOCATION_ERROR_DURING_LOADING){
				MEMORY_ALLOCATION_PERROR;
				fclose(file);
				return;
			}
			// if we reached to the game closing tag then the file doesn't
			// contain a general tag and so the scanning is done.
			isDoneLoading = true;
		}
	}
	while (!isDoneLoading && (fscanf(file, "%s", str) == 1) && (generalTagParser(setting, str) != LOAD_EXIT_LOOP)){
		//inside general tag, we keep looping until we reach the general closing tag
	}

	fclose(file);
	setting->isGameLoaded = GAME_LOADED_SUCCESSFULLY;
}

/**
 * Parses a single line from inside the game tag and updates the game setting accordingly.
 *
 * @param setting - the game setting to be updated
 * @param str	  - the string to be parsed
 * @precondition setting != NULL, str != NULL
 *
 * @return
 * LOAD_EXIT_LOOP if we reached the board opening tag.
 * LOAD_TERMINATE if the difficulty tag contains the expert level.
 * LOAD_CONTINUE_LOOP, otherwise.
 */
int gameSettingParser(GameSetting* setting, char* str){
	char *tag, *content;
	tag = strtok(str, ">");
	content = strtok(NULL, ">");
	content = strtok(content, "<");

	if (!strcmp(tag, BOARD_OPENING_TAG)) // if we reached board opening tag - break the while-loop
		return LOAD_EXIT_LOOP;

	if (!strcmp(tag, DIFFICULTY_OPENING_TAG)){ // if we reached the difficulty opening tag
		int difficulty = atoi(content);
		if (difficulty == EXPERT_LEVEL){
			// expert level is not supported - therefore game could not be load
			setting->isGameLoaded = EXPERT_LEVEL_NOT_SUPPORTED;
			spChessGameDestroy(setting->game);
			return LOAD_TERMINATE;
		}
		else
			setting->difficulty = difficulty;
	}
	else if (!strcmp(tag, CURRENT_TURN_OPENING_TAG)) // if we reached current_turn opening tag
		setting->game->currentPlayer = atoi(content);
	else if (!strcmp(tag, GAME_MODE_OPENING_TAG)) // if we reached the game_mode mode tag
		setting->gameMode = atoi(content);
	else if (!strcmp(tag, USER_COLOR_OPENING_TAG)) // if we reached user_color opening tag
		setting->userColor = atoi(content);
	return LOAD_CONTINUE_LOOP;
}

/**
 * Parses a single line from inside the board tag and updates the game setting accordingly.
 *
 * @param setting - the game setting to be updated
 * @param str	  - the string to be parsed
 * @param row	  - the row that needs to be updated
 * @precondition setting != NULL, str != NULL
 *
 * @return LOAD_EXIT_LOOP if we reached the general closing tag. otherwise returns LOAD_CONTINUE_LOOP.
 */
int boardRowParser(GameSetting* setting, char* str, int row){
	char *tag, *content;

	tag = strtok(str, ">");
	content = strtok(NULL, ">");
	content = strtok(content, "<");

	if (!strcmp(tag, BOARD_CLOSING_TAG))
		return LOAD_EXIT_LOOP;

	for (int col=0; col<BOARD_LENGTH && row>=0; col++){
		setting->game->board[row][col] = content[col];

		if (content[col] == EMPTY_POSITION)
			continue;
		else if (content[col] == WHITE_KING){
			setting->game->whiteKingRow = row;
			setting->game->whiteKingCol = col;
		}
		else if (content[col] == BLACK_KING){
			setting->game->blackKingRow = row;
			setting->game->blackKingCol = col;
		}
		else
			updatePiecesAmount(setting->game, content[col], true);
	}
	return LOAD_CONTINUE_LOOP;
}

/**
 * Parses a single line from inside the general tag and updates the game setting accordingly.
 *
 * @param setting - the game setting to be updated
 * @param str	  - the string to be parsed
 * @precondition setting != NULL, str != NULL
 *
 * @return LOAD_EXIT_LOOP if we reached the general closing tag, otherwise returns LOAD_CONTINUE_LOOP.
 */
int generalTagParser(GameSetting* setting, char* str){
	char *tag, *content;

	tag = strtok(str, ">");
	content = strtok(NULL, ">");
	content = strtok(content, "<");

	if (!strcmp(tag, GENERAL_CLOSING_TAG))
		return LOAD_EXIT_LOOP;

	if (!strcmp(tag, WHITE_LEFT_CASTLE_OPENING_TAG))
		setting->game->whiteLeftCastle = atoi(content);
	else if (!strcmp(tag, WHITE_RIGHT_CASTLE_OPENING_TAG))
		setting->game->whiteRightCastle = atoi(content);
	else if (!strcmp(tag, BLACK_LEFT_CASTLE_OPENING_TAG))
		setting->game->blackLeftCastle = atoi(content);
	else if (!strcmp(tag, BLACK_RIGHT_CASTLE_OPENING_TAG))
		setting->game->blackRightCastle = atoi(content);
	else if (!strcmp(tag, GAME_STATUS_OPENING_TAG))
		setting->game->status = atoi(content);
	return LOAD_CONTINUE_LOOP;
}

/**
 * Given a tag we're looking for, the function takes the given string
 * and compares it to the tag.
 *
 * @param str 		  - the string to be compared to the searched tag
 * @param searchedTag - the tag we search for
 * @precondition searchedTag != NULL, str != NULL
 *
 * @return true if we reached the tag we've been searching, otherwise false.
 */
bool hasReachedTheNextTag(char* str ,const char* searchedTag){
	char* tag = strtok(str, ">");
	return (!strcmp(tag, searchedTag));
}

/**
 * Updates general information of a game that's loaded from a file without the general tag.
 * The information that is updated is:
 * 1. The status of the game - calculated with getGameStatus function
 * 2. Castling options		 - is set to "false" meaning that castling is not possible
 * 							   (as Moab said that castling will not be tested with loaded games)
 * If a memory allocation error occurs in getGameStatus, then we update the game status and
 * the state of the loading (in the setting object) accordingly.
 *
 * @param setting  the game's setting
 * @param game     the game that we load
 * @precondition   setting != NULL
 * @precondition   game != NULL
 */
void loadGameWithoutGeneral(GameSetting* setting, SPChessGame* game){
	bool isWhiteTurn = game->currentPlayer;

	game->status = getGameStatus(game, isWhiteTurn ? game->whiteKingRow : game->blackKingRow,
			isWhiteTurn ? game->whiteKingCol : game->blackKingCol);  //update the game status
	if (game->status == MEMORY_FAILURE){
		setting->isGameLoaded = MEMORY_ALLOCATION_ERROR_DURING_LOADING;
		return;
	}
	game->whiteLeftCastle = false;
	game->whiteRightCastle = false;
	game->blackLeftCastle = false;
	game->blackRightCastle = false;
}

/**
 * Saves the current game state to a file with the specified name that the user entered.
 * If the file cannot be created or modified, the function prints a message.
 *
 * @param setting		  the game setting
 * @param game    		  the game to be updated
 * @param fileName		  the file's relative or full path
 * @precondition 		  setting != NULL
 * @precondition		  game != NULL
 *
 * @return
 * RETRY 			if the file cannot be created or modified
 * TURN_IS_NOT_DONE if the command has been executed successfully
 */
TurnStatus saveGame(char* fileName, SPChessGame* game, GameSetting* setting){
	FILE * file = ((fileName==NULL) ? NULL : fopen(fileName, "w"));
	if (file == NULL){ //if file cannot be created or modified
		SAVE_ERROR;
		return RETRY;
	}

	XML_WRITE_SINGLE_TAG(file, XML_DECLARATION); // write the XML declaration
	XML_WRITE_SINGLE_TAG(file, GAME_OPENING_TAG); // write the game opening tag

	// write the current turn
	XML_WRITE_TAG_LINE(file, CURRENT_TURN_OPENING_TAG, game->currentPlayer, CURRENT_TURN_CLOSING_TAG);
	// write the game mode
	XML_WRITE_TAG_LINE(file, GAME_MODE_OPENING_TAG, setting->gameMode, GAME_MODE_CLOSING_TAG);

	if (setting->gameMode == ONE_PLAYER_MODE){ // if the game is in mode of 1 player
		// write the difficulty
		XML_WRITE_TAG_LINE(file,  DIFFICULTY_OPENING_TAG, setting->difficulty, DIFFICULTY_CLOSING_TAG);
		// write the user color
		XML_WRITE_TAG_LINE(file, USER_COLOR_OPENING_TAG, setting->userColor, USER_COLOR_CLOSING_TAG);
	}

	// write the board state inside the board tag
	XML_WRITE_SINGLE_TAG(file, BOARD_OPENING_TAG);
	for (int rowNum = BOARD_LENGTH; rowNum>FIRST_ROW_AND_COL; rowNum--){
		XML_WRITE_ROW_OPENING_TAG(rowNum);
		for (int colNum = FIRST_ROW_AND_COL; colNum<BOARD_LENGTH; colNum++)
			fprintf(file, "%c", game->board[rowNum-1][colNum]); // write the current piece to file
		XML_WRITE_ROW_CLOSING_TAG(rowNum);
	}
	XML_WRITE_SINGLE_TAG(file, BOARD_CLOSING_TAG);

	// write additional information in general tag
	XML_WRITE_SINGLE_TAG(file, GENERAL_OPENING_TAG);

	// write the status of the validity of a any possible castle move and the game status
	XML_WRITE_TAG_LINE(file, WHITE_LEFT_CASTLE_OPENING_TAG, game->whiteLeftCastle, WHITE_LEFT_CASTLE_CLOSING_TAG);
	XML_WRITE_TAG_LINE(file, WHITE_RIGHT_CASTLE_OPENING_TAG, game->whiteRightCastle, WHITE_RIGHT_CASTLE_CLOSING_TAG);
	XML_WRITE_TAG_LINE(file, BLACK_LEFT_CASTLE_OPENING_TAG, game->blackLeftCastle, BLACK_LEFT_CASTLE_CLOSING_TAG);
	XML_WRITE_TAG_LINE(file, BLACK_RIGHT_CASTLE_OPENING_TAG, game->blackRightCastle, BLACK_RIGHT_CASTLE_CLOSING_TAG);
	XML_WRITE_TAG_LINE(file,GAME_STATUS_OPENING_TAG, game->status, GAME_STATUS_CLOSING_TAG);

	// close the general and the game tags
	XML_WRITE_SINGLE_TAG(file, GENERAL_CLOSING_TAG); // write the closing tag of general
	XML_WRITE_SINGLE_TAG(file, GAME_CLOSING_TAG); // write the closing tag of game

	fclose(file); // close the file
	return TURN_IS_NOT_DONE;
}
