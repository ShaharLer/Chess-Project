#include "SPChessConsoleManager.h"

/**
 * Starts a new game by first entering the setting state. After that, the function enters the game state.
 * If a reset command has been invoked during the game state,
 * then the function enters the setting state once again.
 * If the game has ended and not by a reset command, then the game is terminated.
 */
void consoleMainLoop() {
	setvbuf(stdout,NULL,_IONBF,0);
	setvbuf(stderr,NULL,_IONBF,0);
	setvbuf(stdin,NULL,_IONBF,0);

	GameSetting* setting = (GameSetting *) malloc(sizeof(GameSetting));
	if (setting == NULL){
		MEMORY_ALLOCATION_PERROR;
		return ;
	}
	SP_COMMAND cmd = SettingState(setting);
	if (cmd == SP_START){
		bool isNotResetCommand = GameState(setting);
		destroySettings(setting);
		if (!isNotResetCommand)
			consoleMainLoop();
	}
	else if (cmd == SP_QUIT || setting->isGameLoaded == MEMORY_ALLOCATION_ERROR_DURING_LOADING)
		destroySettings(setting);
}

/**
 * Runs the game state: creates a new game object if a game has not been loaded
 * in the setting state and manages the game state.
 *
 * @param setting - the setting that were defined in the setting state.
 * @precondition setting != NULL
 *
 * @return
 * True if the game-ending command is NOT "reset". Otherwise, false.
 */
bool GameState(GameSetting* setting){
	TurnStatus turnStatus = TURN_IS_NOT_DONE; // the status of the current turn

	if (setting->isGameLoaded != GAME_LOADED_SUCCESSFULLY){ // if a game has not been loaded in the setting state
		setting->game = spChessGameCreate(HISTORY_SIZE);
		if (setting->game == NULL){
			MEMORY_ALLOCATION_PERROR;
			return true;
		}
	}
	SPChessGame* game = setting->game;
	bool isComputerTurn = !(setting->gameMode == TWO_PLAYERS_MODE || game->currentPlayer == setting->userColor);

	if (setting->isGameLoaded == GAME_LOADED_SUCCESSFULLY)
		//if a game was loaded, we print a suitable message in case we loaded a checkmate state or a check state.
		printAfterTurnMessage(!isComputerTurn, game->status, game->currentPlayer);

	while ((game->status == GAME_NOT_FINISHED_CHECK || game->status == GAME_NOT_FINISHED_NO_CHECK)
			&& (turnStatus != QUIT_GAME) && (turnStatus != RESET_GAME)){
		// while the game has not ended and the user didn't invoke a reset command or a quit command
		isComputerTurn = !(setting->gameMode == TWO_PLAYERS_MODE || game->currentPlayer == setting->userColor);

		if (!isComputerTurn)
			turnStatus = userTurn(game, setting);
		else
			turnStatus = computerTurn(game, setting);

		if ((turnStatus != QUIT_GAME) && (turnStatus != RESET_GAME))
			printAfterTurnMessage(isComputerTurn, game->status, game->currentPlayer);
	}

	return (!(turnStatus == RESET_GAME));
}

/**
 * Prints the game board to the console at the beginning of the user's turn and gets the user's commands
 * until the end of his/her turn. After the board is printed, the game asks the relevant user to enter
 * his/her move by printing a message to the relevant user. The message will be printed each time a user
 * has to make a move. In case an error occurs (for example the user enters an invalid move), then
 * the message is printed again.
 *
 * @param game 	  - the game to be updated
 * @param setting - the setting of the game
 * @precondition game != NULL
 * @precondition setting != NULL
 *
 * @return
 * DONE 			if the user's turn is done
 * QUIT_GAME 		if the command is "quit" or an allocation error has occurred
 * 					and the game needs to be ended
 * RESET_GAME 		if the command is "reset" and the game needs to be reseted
 */
TurnStatus userTurn(SPChessGame* game, GameSetting* setting){
	SPCommand command;
	TurnStatus turnStatus;
	char str[SP_MAX_LINE_LENGTH];

	spChessGamePrintBoard(game);
	do {
		(game->currentPlayer == WHITE_PLAYER) ? PRINT_USER_TURN_MESSAGE(WHITE_PLAYER_LOWER_CASE_STRING):
				PRINT_USER_TURN_MESSAGE(BLACK_PLAYER_LOWER_CASE_STRING);
		fgets(str, SP_MAX_LINE_LENGTH, stdin);
		command = spParserParseLine(str, false);
		turnStatus = GameCommandHandler(game, command, setting);
	} while (turnStatus != DONE && turnStatus != QUIT_GAME && turnStatus != RESET_GAME);
	return turnStatus;
}

/**
 * After getting a move from the Minimax function, the function updates the game
 * and prints the computer's move.
 *
 * @param game 	    the game to be updated
 * @param setting   the setting of the game
 * @precondition 	game != NULL
 * @precondition 	setting != NULL
 *
 * @return
 * DONE 			if the computer's turn is done without errors
 * QUIT_GAME 		if memory allocation error has occurred
 */
TurnStatus computerTurn(SPChessGame* game, GameSetting* setting){
	int kingRow, kingColInt, srcRow, dstRow;
	char kingCol, srcCol, dstCol, *srcPiece;

	Move* move = spChessMinimaxMove(game, setting->difficulty);

	if (move != NULL && move->castleMove == true){
		kingRow = (game->currentPlayer == WHITE_PLAYER) ? (game->whiteKingRow+1) : (game->blackKingRow+1);
		kingColInt = (game->currentPlayer == WHITE_PLAYER) ? game->whiteKingCol : game->blackKingCol;
		kingCol = (char)(FIRST_COL_SYMBOL+kingColInt);
	}

	if (move == NULL || spChessGameSetMove(game, move, false, true) == SP_CHESS_GAME_MEMORY_FAILURE){
		MEMORY_ALLOCATION_PERROR;
		return QUIT_GAME;
	}

	srcPiece = formatSrcPiece(move->srcPiece);
	srcRow = move->srcRow+1; //the game board coordinated are 1-indexed, not 0-indexed
	srcCol = (char)(FIRST_COL_SYMBOL+move->srcCol);
	dstRow = move->dstRow+1;
	dstCol = (char)(FIRST_COL_SYMBOL+move->dstCol);

	if (move->pawnPromotion)
		PRINT_AI_PAWN_PROMOTION_MESSAGE(srcRow, srcCol, dstRow, dstCol, srcPiece);
	else if (!move->castleMove)
		PRINT_AI_REGULAR_MOVE_MESSAGE(srcPiece, srcRow, srcCol, dstRow, dstCol);
	else
		PRINT_AI_CASTLING_MESSAGE(kingRow, kingCol, srcRow, srcCol);

	free(move);
	return DONE;
}

/**
 * Given the user's command, this function handles it and updates the game accordingly.
 *
 * @param game 	  - the game to be updated
 * @param command - the user's command, after parsing it
 * @param setting - the setting of the game
 * @precondition game != NULL
 * @precondition setting != NULL
 * @precondition command != NULL
 *
 * @return
 * RETRY 			if an error has occurred (for example an invalid user input) and
 * 					the user needs to insert a command again.
 * DONE 			if the user's turn is done
 * TURN_IS_NOT_DONE if no error has occurred but the user's turn is still on going
 * QUIT_GAME 		if the command is "quit" or an allocation error has occurred
 * 					and the game needs to be ended
 * RESET_GAME 		if the command is "reset" and the game needs to be reseted
 */
TurnStatus GameCommandHandler(SPChessGame* game, SPCommand command, GameSetting* setting){
	if (command.cmd == SP_MOVE || command.cmd == SP_CASTLE)
		return moveOrCastleCommandHandler(game, command);

	if (command.cmd == SP_GET_MOVES)
		return getMovesCommandHander(setting, game, command.srcRow, command.srcCol);

	if (command.cmd == SP_SAVE)
		return saveGame((command.validStrArg ? command.strArg : NULL), game, setting);

	if (command.cmd == SP_UNDO_MOVE)
		return undoCommandHandler(setting, game);

	if (command.cmd == SP_RESET){
		PRINT_RESET_MESSAGE;
		return RESET_GAME;
	}

	if (command.cmd == SP_QUIT){
		PRINT_QUIT_MESSAGE;
		return QUIT_GAME;
	}

	INVALID_COMMAND_PERROR;
	return RETRY;
}

/**
 * Handles a move command or a castle command entered by the user and prints
 * a suitable message to the console.
 *
 * @param game 	  - the game to be updated
 * @param command - the user's command, after parsing it
 * @precondition game != NULL
 * @precondition command != NULL
 *
 * @return
 * RETRY 			if an error has occurred (for example an invalid user input) and
 * 					the user needs to insert a command again.
 * DONE 			if the user's turn is done with no errors
 * QUIT_GAME 		if an allocation error has occurred	and the game needs to be ended
 */
TurnStatus moveOrCastleCommandHandler(SPChessGame* game, SPCommand command){
	Move* move = createMoveFromCommand(command);
	SP_CHESS_GAME_MESSAGE message = spChessGameSetMove(game, move, false, false);

	if (message == SP_CHESS_GAME_MEMORY_FAILURE){
		MEMORY_ALLOCATION_PERROR;
		free(move);
		return QUIT_GAME;
	}
	if (message != SP_CHESS_GAME_SUCCESS){
		printMoveErrorMessage(message);
		free(move);
		return RETRY;
	}
	if ((move->pawnPromotion == true) && (performPawnPromotion(game, move) == SP_CHESS_GAME_MEMORY_FAILURE)){
		MEMORY_ALLOCATION_PERROR;
		free(move);
		return QUIT_GAME;
	}
	free(move);
	return DONE;
}

/**
 * When the user enters a move command such that a pawn reaches the 8th rank, that is,
 * it reached row number 8 from its opponent side, then a pawn promotion occurs.
 * The function prints a message to the console. Once the message is printed, the user must specify
 * the promotion of the pawn by entering the piece type. The function waits until the user enters a
 * valid string.
 *
 * @param game - the game to be updated
 * @param move - the move created from the user's command
 * @precondition game != NULL
 * @precondition move != NULL
 *
 * @return
 * SP_CHESS_GAME_MEMORY_FAILURE if a memory allocation error has occurred
 * 								during the pawn promotion setting.
 * SP_CHESS_GAME_SUCCESS 		otherwise
 */
SP_CHESS_GAME_MESSAGE performPawnPromotion(SPChessGame* game, Move* move){
	char str[SP_MAX_LINE_LENGTH];
	char chosenPiece;

	do {
		PRINT_PAWN_PROMOTION_MESSAGE;
		fgets(str, SP_MAX_LINE_LENGTH, stdin);
		chosenPiece = spPieceTypeParser(game->currentPlayer, str);
		if (chosenPiece == INVALID_TYPE)
			PAWN_PROMOTION_INVALID_TYPE_PERROR;
	} while (chosenPiece == INVALID_TYPE);

	move->srcPiece = chosenPiece;
	return spChessGameSetPawnPromotion(game, move, false);
}

/**
 * Prints a move error message.
 * In case of more than one error occurs, only one error message is printed,
 * according to the project's instructions.
 *
 * @param message - by this parameter the function knows which error to print
 */
void printMoveErrorMessage(SP_CHESS_GAME_MESSAGE message){
	if (message == SP_CHESS_GAME_INVALID_POSITION)
		INVALID_POSITION_PERROR;
	else if (message == SP_CHESS_GAME_INVALID_ARGUMENT)
		MOVE_INVALID_PIECE_PERROR;
	else if (message == SP_CHESS_GAME_ILLEGAL_MOVE)
		MOVE_ILLEGAL_FOR_PIECE_PERROR;
	else if (message == SP_CHESS_GAME_CASTLE_NO_ROOK)
		CASTLING_NO_ROOK_PERROR;
	else if (message == SP_CHESS_GAME_ILLEGAL_CASTLE_MOVE)
		CASTLING_ILLEGAL_PERROR;
}

/*
 * Prints a message to the console after a turn has been done, depending on the case.
 *
 * @param isComputerTurn   is set to true if it's the computer's turn
 * @param gameStatus	   the status of the game
 * @param currentPlayer	   the current player of the game
 * @precondition 		   the user didn't invoke a quit or a reset command before entering this function
 */
void printAfterTurnMessage(bool isComputerTurn, GAME_STATUS gameStatus, int currentPlayer){
	if (gameStatus == WHITE_PLAYER_WINS)
		PRINT_CHECKMATE_MESSAGE(WHITE_PLAYER_LOWER_CASE_STRING);
	else if (gameStatus == BLACK_PLAYER_WINS)
		PRINT_CHECKMATE_MESSAGE(BLACK_PLAYER_LOWER_CASE_STRING);
	else if (gameStatus == TIED_GAME)
		(isComputerTurn) ?	PRINT_AI_TIE_MESSAGE : PRINT_USER_TIE_MESSAGE;
	else if (gameStatus == GAME_NOT_FINISHED_CHECK){
		if (isComputerTurn == true)
			PRINT_AI_CHECK_MESSAGE;
		else {
			if (currentPlayer == WHITE_PLAYER)
				PRINT_USER_CHECK_MESSAGE(WHITE_PLAYER_LOWER_CASE_STRING);
			else //currentPlayer == BLACK_PLAYER
				PRINT_USER_CHECK_MESSAGE(BLACK_PLAYER_LOWER_CASE_STRING);
		}
	}
}

/**
 * Handles get_moves command. If an error occurs, the function prints a suitable message.
 * Otherwise, it prints all possible moves of the piece located in the position
 * inserted by the user.
 *
 * @param setting		  the game setting
 * @param game    		  the relevant game
 * @param commandSrcRow   the row inserted by the user
 * @param commandSrcCol   the col inserted by the user
 * @precondition 		  setting != NULL
 *
 * @return
 * RETRY 			if the command is not supported by the game settings
 * 					or if the user entered an invalid input
 * QUIT_GAME 		if a memory allocation error has occurred and the game must be ended
 * TURN_IS_NOT_DONE if the command has been executed successfully
 */
TurnStatus getMovesCommandHander(GameSetting* setting, SPChessGame* game, int commandSrcRow, int commandSrcCol){
	if (setting->gameMode == TWO_PLAYERS_MODE || (setting->difficulty != 2 && setting->difficulty != 1)){
		GET_MOVES_NOT_AVAILABLE_PERROR;
		return RETRY;
	}

	SPArrayList* possibleMoves = spArrayListCreate(MAX_MOVES_FOR_PIECE);
	SP_CHESS_GAME_MESSAGE message = spChessGetPossibleMoves(game, possibleMoves, commandSrcRow, commandSrcCol);

	if (message == SP_CHESS_GAME_MEMORY_FAILURE){
		MEMORY_ALLOCATION_PERROR;
		spArrayListDestroy(possibleMoves);
		return QUIT_GAME;
	}
	if (message == SP_CHESS_GAME_INVALID_POSITION){
		INVALID_POSITION_PERROR;
		spArrayListDestroy(possibleMoves);
		return RETRY;
	}
	if (message == SP_CHESS_GAME_INVALID_ARGUMENT){
		(game->currentPlayer) ? GET_MOVES_INVALID_PIECE_PERROR(WHITE_PLAYER_LOWER_CASE_STRING) :
								GET_MOVES_INVALID_PIECE_PERROR(BLACK_PLAYER_LOWER_CASE_STRING);
		spArrayListDestroy(possibleMoves);
		return RETRY;
	}

	printPossibleMoves(possibleMoves);
	spArrayListDestroy(possibleMoves);
	return TURN_IS_NOT_DONE;
}

/**
 * Prints all possible moves of a certain piece on the board.
 * Each possible move is printed in a separate line in the format specified in the instructions.
 *
 * @param possibleMoves a sorted array list of the possible moves
 * @precondition		possibleMove != NULL
 */
void printPossibleMoves(SPArrayList* possibleMoves){
	for (int i=0; i<possibleMoves->actualSize; i++){
		Move* currentMove = spArrayListGetAt(possibleMoves, i);
		char dstCol = (char)(FIRST_COL_SYMBOL+currentMove->dstCol);
		int dstRow = (currentMove->dstRow)+1;

		if (currentMove->castleMove == true){
			int srcRow = (currentMove->srcRow)+1;
			char srcCol = (char)(FIRST_COL_SYMBOL+currentMove->srcCol);
			PRINT_GET_MOVES_CASTLE(srcRow, srcCol);
		}
		else if ((currentMove->threatenedAfterMove == true) && (currentMove->dstPieceCaptured == true))
			PRINT_GET_MOVES_CAPTURES_AND_THREATENED(dstRow, dstCol);
		else if (currentMove->threatenedAfterMove == true)
			PRINT_GET_MOVES_THREATENED(dstRow, dstCol);
		else if (currentMove->dstPieceCaptured == true)
			PRINT_GET_MOVES_CAPTURES(dstRow, dstCol);
		else
			PRINT_GET_MOVES(dstRow, dstCol);
	}
}

/**
 * Handles an undo command and prints a suitable message.
 * After an undo command is executed, the board game is printed.
 * However, if there's a check from previous rounds, a message about that is NOT printed.
 *
 * @param setting		  the game setting
 * @param game    		  the game to be updated
 * @precondition 		  setting != NULL
 * @precondition		  game != NULL
 *
 * @return
 * RETRY 			if the command is not supported by the game settings
 * 					or if the game history is empty
 * TURN_IS_NOT_DONE if the command has been executed successfully
 */
TurnStatus undoCommandHandler(GameSetting* setting, SPChessGame* game){
	if (setting->gameMode == TWO_PLAYERS_MODE){
		UNDO_NOT_AVAILABLE_PERROR;
		return RETRY;
	}
	if (!spChessGameIsUndoPossible(game, setting->userColor)){
		UNDO_EMPTY_HISTORY_PERROR;
		return RETRY;
	}

	Move* lastMove = spArrayListGetLast(game->history);
	SP_CHESS_GAME_MESSAGE message = spChessGameUndoPrevMove(game, lastMove);
	Move* beforeLastMove = spArrayListGetLast(game->history);
	message = spChessGameUndoPrevMove(game, beforeLastMove);

	if (message == SP_CHESS_GAME_SUCCESS){
		PRINT_UNDO_MOVE_MESSAGE(game->currentPlayer ? BLACK_PLAYER_LOWER_CASE_STRING : WHITE_PLAYER_LOWER_CASE_STRING,
				lastMove->dstRow+1, (char)(FIRST_COL_SYMBOL+lastMove->dstCol), lastMove->srcRow+1, (char)(FIRST_COL_SYMBOL+lastMove->srcCol));
		PRINT_UNDO_MOVE_MESSAGE(!game->currentPlayer ? BLACK_PLAYER_LOWER_CASE_STRING : WHITE_PLAYER_LOWER_CASE_STRING,
				beforeLastMove->dstRow+1, (char)(FIRST_COL_SYMBOL+beforeLastMove->dstCol), beforeLastMove->srcRow+1, (char)(FIRST_COL_SYMBOL+beforeLastMove->srcCol));
	}
	spChessGamePrintBoard(game); // the board is printed after an undo command
	return TURN_IS_NOT_DONE;
}
