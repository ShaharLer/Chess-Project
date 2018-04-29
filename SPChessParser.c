#include "SPChessParser.h"

/**
 * Checks if a specified string represents a valid integer.
 *
 * @return
 * true if the string represents a valid integer, and false otherwise.
 */
bool spParserIsInt(const char* str){
	if (str == NULL || strlen(str)==0)
		return false;
	for (unsigned int i=0; i<strlen(str); i++){
		if (!isdigit(str[i]))
			return false;
	}
	return true;
}

/**
 * Given a coordinate and a string, the function checks if the string is in the format
 * of <x,y>, where x and y are strings of single letters, representing the row number and the
 * column letter respectively and updates the position.
 * The column character is updated to be matched to the unicode of the column character on
 * the game board. If the string representing the column is not a single letter,
 * then the column number is updated to be DUMMY_COORDINATE (aka -1).
 * This function does not check the validity of the position on the board, but checks only
 * the validity of the format of the coordinate - <x,y>.
 *
 * @return
 * true if str represents a valid coordinate format, and otherwise false.
 */
bool spParserIsValidFormat(int* row, int* col, const char* str){
	char newStr[SP_MAX_LINE_LENGTH];
	char *firstArg, *secondArg;

	if (str == NULL)
		return false;

	if (str[0] != '<' || str[strlen(str)-1] != '>' || memchr(str, ',', strlen(str)+1) == NULL)
		return false;

	memcpy(newStr, &str[1], strlen(str));
	firstArg = strtok(newStr," \t\r\n,");
	secondArg = strtok(NULL," \t\r\n>");

	*row = atoi(firstArg)-1; // the board is 0-indexed, thus the subtracting
	if (strlen(secondArg) != 1){ // if the column argument is not a single character as it should be
		*col = DUMMY_COORDINATE; // insert an invalid column number
	}
	else
		*col = (int) (secondArg[0]-FIRST_COL_SYMBOL);

	return true;
}

/**
 * Parses a specified line. If the line is a command which has an argument
 * then the argument is parsed and is saved in the suitable field arg and the
 * field validArg is set to true. In any other case then 'validArg' is set to
 * false and the value 'arg' is undefined.
 *
 * @return
 * A parsed line such that:
 *   cmd - contains the command type, if the line is invalid then this field is
 *         set to INVALID_LINE
 *   validIntArg - is set to true if the command has an integer argument
 *   			   and the integer argument is valid.
 *   validStrArg - is set to true if the command is load/save and path argument!=NULL
 *   			   otherwise true
 *   intArg      - the integer argument in case validArg is set to true
 *   strArg		 - the string argument in case the command is save/load
 *   srcRow		 - the source row of the move/get_moves/castle command
 *   srcCol		 - the source column of the move/get_moves/castle command
 *   dstRow		 - the destination row of the move command
 *   dstCol	 	 - the destination column of the move command
 */
SPCommand spParserParseLine(const char* str, bool isSettingState){
	SPCommand command;
	char newStr[SP_MAX_LINE_LENGTH];
	char* first_word, *second_word, *third_word, *fourth_word;

	memcpy(newStr, str, strlen(str)+1);
	first_word = strtok(newStr," \t\r\n");

	command.cmd = spCommandParser(first_word, isSettingState);
	command.validIntArg = false;
	command.validStrArg = false;

	if (command.cmd != SP_INVALID_LINE){

		second_word = strtok(NULL," \t\r\n");

		if (command.cmd == SP_MOVE || command.cmd == SP_GET_MOVES || command.cmd == SP_CASTLE){

			third_word = strtok(NULL," \t\r\n");
			fourth_word = strtok(NULL," \t\r\n");

			if (spParserIsValidFormat(&command.srcRow, &command.srcCol, second_word)){
				//the source position  format is valid
				if (command.cmd == SP_MOVE){
					if (third_word == NULL || strcmp(third_word,TO_WORD_IN_MOVE)) //if the third word of the move command is not "to"
						command.cmd = SP_INVALID_LINE;
					else if (!(spParserIsValidFormat(&command.dstRow, &command.dstCol, fourth_word)))
						//the destination position is valid too - the positions given are valid
						command.cmd = SP_INVALID_LINE;
				}
			}
			else
				command.cmd = SP_INVALID_LINE;
		}

		else if ( (command.cmd == SP_DEFAULT || command.cmd == SP_QUIT || command.cmd == SP_PRINT_SETTING
				|| command.cmd == SP_START || command.cmd == SP_UNDO_MOVE
				|| command.cmd == SP_RESET) && second_word != NULL )
			//command has 2 words although the command entered requires one word at most
			command.cmd = SP_INVALID_LINE;

		else if (command.cmd == SP_GAME_MODE || command.cmd == SP_DIFFICULTY || command.cmd == SP_USER_COLOR ){
			//command requires one word and one integer argument
			if (spParserIsInt(second_word)){ //if 2nd word represents an integer
				command.validIntArg = true;
				command.intArg = atoi(second_word);
			}
		}

		else if ((command.cmd == SP_LOAD) || (command.cmd == SP_SAVE)){
			command.validStrArg = (second_word!=NULL);
			for (unsigned int i=0; command.validStrArg && i<(strlen(second_word)+1); i++)
				command.strArg[i] = second_word[i];
		}
	}

	return command;
}

/**
 * Parses a string and checks if it's a command. The function returns the suitable SP_COMMAND.
 *
 * @return
 * The command type represented by the tokens.
 * If the string is not a command then the return value is SP_INVALID_LINE
 */
SP_COMMAND spCommandParser(char* tokens, bool isSettingState){
	if (tokens == NULL)
		return SP_INVALID_LINE;
	if (!strcmp(tokens,QUIT))
		return SP_QUIT;

	if (isSettingState){
		if (!strcmp(tokens,GAME_MODE))
			return SP_GAME_MODE;
		if (!strcmp(tokens,DIFFICULTY))
			return SP_DIFFICULTY;
		if (!strcmp(tokens,USER_COLOR))
			return SP_USER_COLOR;
		if (!strcmp(tokens,LOAD))
			return SP_LOAD;
		if (!strcmp(tokens,DEFAULT))
			return SP_DEFAULT;
		if (!strcmp(tokens,PRINT_SETTING))
			return SP_PRINT_SETTING;
		if (!strcmp(tokens,START))
			return SP_START;
	}

	else {
		if (!strcmp(tokens,MOVE))
			return SP_MOVE;
		if (!strcmp(tokens,GET_MOVES))
			return SP_GET_MOVES;
		if (!strcmp(tokens,SAVE))
			return SP_SAVE;
		if (!strcmp(tokens,UNDO))
			return SP_UNDO_MOVE;
		if (!strcmp(tokens,RESET))
			return SP_RESET;
		if (!strcmp(tokens, CASTLE))
			return SP_CASTLE;
	}

	return SP_INVALID_LINE;

}

/**
 * Parses the possible inputs from the user, in case of a pawn promotion.
 *
 * @precondition ((str != NULL) && (currentPlayer == WHITE_PLAYER || currentPlayer == BLACK_PLAYER))
 *
 * @return
 * The character that representing the piece chosen by the player to be promoted to.
 */
char spPieceTypeParser(int currentPlayer, char* str){
	char newStr[SP_MAX_LINE_LENGTH];
	char* first_word, *second_word;

	memcpy(newStr, str, strlen(str)+1);
	first_word = strtok(newStr," \t\r\n");
	second_word = strtok(NULL," \t\r\n");
	
	if ((first_word == NULL) || (second_word != NULL))
		return INVALID_TYPE;
	if (!strcmp(first_word, PAWN))
		return (currentPlayer == WHITE_PLAYER) ? WHITE_PAWN : BLACK_PAWN;
	if (!strcmp(first_word, BISHOP))
		return (currentPlayer == WHITE_PLAYER) ? WHITE_BISHOP : BLACK_BISHOP;
	if (!strcmp(first_word, ROOK))
		return (currentPlayer == WHITE_PLAYER) ? WHITE_ROOK : BLACK_ROOK;
	if (!strcmp(first_word, KNIGHT))
		return (currentPlayer == WHITE_PLAYER) ? WHITE_KNIGHT : BLACK_KNIGHT;
	if (!strcmp(first_word, QUEEN))
		return (currentPlayer == WHITE_PLAYER) ? WHITE_QUEEN : BLACK_QUEEN;
	return INVALID_TYPE;
}

/**
 * Creates a move object from a move/get_moves command object.
 *
 * @param the command inserted
 * @precondition command != NULL, command.cmd == SP_MOVE || command.cmd == SP_GET_MOVES
 * @return
 * The move object, which is represented in the command.
 * If an allocation error occurs, NULL is returned.
 */
Move* createMoveFromCommand(SPCommand command){
	Move* move = spCreateMove();
	if (move == NULL)
		return NULL;

	move->srcRow = command.srcRow;
	move->srcCol = command.srcCol;
	move->dstRow = command.dstRow;
	move->dstCol = command.dstCol;
	move->castleMove = (command.cmd == SP_CASTLE);

	return move;
}

/**
 * Given a srcPiece, the function returns the string representing this piece.
 */
char* formatSrcPiece(char srcPiece){
	if (srcPiece == WHITE_QUEEN || srcPiece == BLACK_QUEEN)
		return QUEEN;
	if (srcPiece == WHITE_PAWN || srcPiece == BLACK_PAWN)
		return PAWN;
	if (srcPiece ==  WHITE_BISHOP || srcPiece == BLACK_BISHOP)
		return BISHOP;
	if (srcPiece == WHITE_KNIGHT || srcPiece == BLACK_KNIGHT)
		return KNIGHT;
	if (srcPiece == WHITE_ROOK || srcPiece == BLACK_ROOK)
		return ROOK;
	if (srcPiece == WHITE_KING || srcPiece == BLACK_KING)
		return KING;
	return "ERROR IN FUNCTION formatSrcPiece in SPChessParser. srcPiece is probably '_'"; // code should not visit here
}
