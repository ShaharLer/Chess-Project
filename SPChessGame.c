#include "SPChessGame.h"

/**
 * The function creates a new game instant with a specified history size.
 * All the game variables that should be initialized get their default values,
 * and the game board gets initialized at the helper function initializeBoard.
 *
 * @param historySize - The total number of moves to undo, a player can undo
 * 						at most historySize turns (which is actually (historySize / 2))
 * @return
 * 		NULL if either a memory allocation failure occurs or (historySize <= 0).
 * 		Otherwise, a new game instant is returned.
 */
SPChessGame* spChessGameCreate(int historySize) {
	if (historySize <= 0)
		return NULL;

	SPChessGame* game = (SPChessGame*) malloc(sizeof(SPChessGame));
	if (game == NULL)
		return NULL;

	game->history = spArrayListCreate(historySize);
	if (game->history == NULL) {
		free(game);
		return NULL;
	}

	game->whiteArmy = (Army*) malloc(sizeof(Army));
	if (game->whiteArmy == NULL) {
		spArrayListDestroy(game->history);
		free(game);
		return NULL;
	}

	game->blackArmy = (Army*) malloc(sizeof(Army));
	if (game->blackArmy == NULL) {
		free(game->whiteArmy);
		spArrayListDestroy(game->history);
		free(game);
		return NULL;
	}

	initializeBoard(game->board);

	game->currentPlayer           = WHITE_PLAYER;
	game->whiteLeftCastle         = game->whiteRightCastle = true;
	game->blackLeftCastle         = game->blackRightCastle = true;
	game->whiteKingRow            = WHITE_FIRST_ROW;
	game->blackKingRow            = BLACK_FIRST_ROW;
	game->whiteKingCol            = game->blackKingCol            = KING_COL;
	game->whiteArmy->numOfPawns   = game->blackArmy->numOfPawns   = INIT_PAWNS;
	game->whiteArmy->numOfKnights = game->blackArmy->numOfKnights = INIT_KNIGHTS;
	game->whiteArmy->numOfBishops = game->blackArmy->numOfBishops = INIT_BISHOPS;
	game->whiteArmy->numOfRooks   = game->blackArmy->numOfRooks   = INIT_ROOKS;
	game->whiteArmy->numOfQueens  = game->blackArmy->numOfQueens  = INIT_QUEENS;
	game->status                  = GAME_NOT_FINISHED_NO_CHECK;

	return game;
}

/**
 * The function creates a copy of a given game.
 * No need to copy the history of the game, because only the Minimax module uses this function,
 * and we are not updating the history in the Minimax algorithm (for more details look at the documentation of spChessMinimax module).
 *
 * @param src - The source Chess game which will be copied
 *
 * @return
 * 		NULL if either src is NULL or a memory allocation failure occurred.
 * 		Otherwise, a new copy of the source game is returned.
 */
SPChessGame* spChessGameCopy(SPChessGame* src) {
	if (src == NULL)
		return NULL;

	SPChessGame* dest = spChessGameCreate(src->history->maxSize);
	if (dest == NULL)
		return NULL;

	// copies the game board
	for (int row = 0; row<BOARD_LENGTH; row++)
		for (int col = 0; col < BOARD_LENGTH; col++)
			dest->board[row][col] = src->board[row][col];

	dest->currentPlayer		      = src->currentPlayer;

	dest->whiteKingRow			  = src->whiteKingRow;
	dest->whiteKingCol			  = src->whiteKingCol;
	dest->blackKingRow			  = src->blackKingRow;
	dest->blackKingCol			  = src->blackKingCol;

	dest->whiteLeftCastle         = src->whiteLeftCastle;
	dest->whiteRightCastle        = src->whiteRightCastle;
	dest->blackLeftCastle         = src->blackLeftCastle;
	dest->blackRightCastle        = src->blackRightCastle;

	dest->whiteArmy->numOfPawns   = src->whiteArmy->numOfPawns;
	dest->whiteArmy->numOfKnights = src->whiteArmy->numOfKnights;
	dest->whiteArmy->numOfBishops = src->whiteArmy->numOfBishops;
	dest->whiteArmy->numOfRooks   = src->whiteArmy->numOfRooks;
	dest->whiteArmy->numOfQueens  = src->whiteArmy->numOfQueens;

	dest->blackArmy->numOfPawns   = src->blackArmy->numOfPawns;
	dest->blackArmy->numOfKnights = src->blackArmy->numOfKnights;
	dest->blackArmy->numOfBishops = src->blackArmy->numOfBishops;
	dest->blackArmy->numOfRooks   = src->blackArmy->numOfRooks;
	dest->blackArmy->numOfQueens  = src->blackArmy->numOfQueens;

	dest->status				  = src->status;

	return dest;
}

/**
 * The function frees all the memory that was allocated and is associated with a given game.
 * If (game == NULL) the function does nothing.
 *
 * @param game - A Chess game
 */
void spChessGameDestroy(SPChessGame* game) {
	if (game == NULL)
		return;

	free(game->blackArmy);
	free(game->whiteArmy);
	spArrayListDestroy(game->history);
	free(game);
}

/**
 * The function prints the game board.
 * The upper case letters represent the black player's pieces, while the lower case letters represent the white player's pieces.
 *
 * @param game - A Chess game
 *
 * @return
 * 		SP_CHESS_GAME_INVALID_ARGUMENT - If game is NULL.
 * 		SP_CHESS_GAME_SUCCESS          - Otherwise.
 */
SP_CHESS_GAME_MESSAGE spChessGamePrintBoard(SPChessGame* game) {
	if (game == NULL)
		return SP_CHESS_GAME_INVALID_ARGUMENT;

	int i,j;

	for (i = BOARD_LENGTH; i > 0; i--) {
		printf("%d|", i);

		for (j = 0; j < BOARD_LENGTH; j++)
			printf(" %c", game->board[i-1][j]);

		printf(" |\n");
	}

	printf("  ");

	for (i = 0; i < ((BOARD_LENGTH * 2) + 1); i++)
		printf("-");

	printf("\n  ");

	for (j = 0; j < BOARD_LENGTH; j++)
		printf(" %c", (char)('A' + j));

	printf("\n");

	return SP_CHESS_GAME_SUCCESS;
}

/**
 * The function sets the next move in a given game according to the move that is received.
 * Move is either a castle move, a pawn promotion move or a regular move:
 * In case of a castle move         - We check its validity by calling the the helper function legalCastle
 *                                    and then if valid, setting the move by calling the helper function setCastleMove.
 * In case of a pawn promotion move - This case happens in this function only if it is a computer move, so its validity was
 * 									  already checked when as part of the Minimax algorithm. That's why in this case the function
 * 									  calls the spChessGameSetPawnPromotion.
 * 									  If the user tries to make a pawn promotion move, it first goes through this function as a
 * 									  regular move and after finish this function returns to the console manager while indicating
 * 									  pawn promotion is to be made.
 * In case of a regular move        - We check its validity by calling the the helper function legalRegularMove
 *                                    and then if valid, setting the move by calling the helper function setRegularMove.
 *                                    Again, a pawn promotion move by the user goes through this "route".
 *
 * @param game         - A Chess game
 * @param move         - The move to be set in the game
 * @param minimaxMove  - Indicates if it is a move that is tried to be set as part of the Minimax algorithm tree
 * @param computerMove - Indicates if it is a move done by the computer or not (i.e. by the user), might get true only on game mode 1
 *
 * @return
 * 		SP_CHESS_GAME_INVALID_ARGUMENT    - If either game is NULL or move is NULL or the piece at the source position doesn't
 * 									    	 belong to the current player.
 * 		SP_CHESS_GAME_INVALID_POSITION    - If either the source position or the destination position of the move is not valid.
 * 		SP_CHESS_GAME_CASTLE_NO_ROOK      - If a castle move is to be set but there is no rook at the source position.
 * 		SP_CHESS_GAME_ILLEGAL_CASTLE_MOVE - If a castle move is to be set but it is not a legal castle move.
 * 		SP_CHESS_GAME_ILLEGAL_MOVE        - If a regular (not castle) move is to be set but it is not a legal move.
 * 		SP_CHESS_GAME_MEMORY_FAILURE      - If a pawn promotion move (by the computer) is to be set but the retruned SP_CHESS_GAME_MESSAGE
 * 									   		 from the function spChessGameSetPawnPromotion didn't finish successfully.
 *
 * 		Otherwise, returns the SP_CHESS_GAME_MESSAGE that was returned from the function updatesAfterSettingMove.
 */
SP_CHESS_GAME_MESSAGE spChessGameSetMove(SPChessGame* game, Move* move, bool minimaxMove, bool computerMove) {
	if ((game == NULL) || (move == NULL))
		return SP_CHESS_GAME_INVALID_ARGUMENT;

	// checking the destination validity for castle move is not needed because we do not update the dstRow/dstCol in the castling move instant
	if (   !validPosition(move->srcRow, move->srcCol)
		|| ((!move->castleMove) && !validPosition(move->dstRow, move->dstCol))) {

				return SP_CHESS_GAME_INVALID_POSITION;
	}

	// for pawn promotion move, source piece is assigned before entering this function. Otherwise, we set the source piece according to the board
	if (!move->pawnPromotion)
		move->srcPiece = game->board[move->srcRow][move->srcCol];

	if (!currentPlayerPiece(game->currentPlayer, move->srcPiece))
		return SP_CHESS_GAME_INVALID_ARGUMENT;

	// saving the game castling indicators before the move is set (will serve us in the "undo" command)
	move->whiteLeftCastle  = game->whiteLeftCastle;
	move->whiteRightCastle = game->whiteRightCastle;
	move->blackLeftCastle  = game->blackLeftCastle;
	move->blackRightCastle = game->blackRightCastle;

	if (move->castleMove) {
		if ((move->srcPiece != WHITE_ROOK) && (move->srcPiece != BLACK_ROOK))
			return SP_CHESS_GAME_CASTLE_NO_ROOK;

		// no need to check here validity for a computer because it was alredy verified as part of the Minimax algorithm
		if (!computerMove && !legalCastle(game, move))
			return SP_CHESS_GAME_ILLEGAL_CASTLE_MOVE;

		setCastleMove(game, move);
	}
	else if (move->pawnPromotion) {      // pawn promotion by the computer
		if (spChessGameSetPawnPromotion(game, move, true) != SP_CHESS_GAME_SUCCESS)
			return SP_CHESS_GAME_MEMORY_FAILURE;
	}
	else {
		// no need to check here validity for a computer because it was alredy verified as part of the Minimax algorithm
		if (!computerMove && !legalRegularMove(game, move))
			return SP_CHESS_GAME_ILLEGAL_MOVE;

		setRegularMove(game, move);
	}

	return updatesAfterSettingMove(game, move, game->history, minimaxMove);
}

/**
 * The function sets a pawn promotion move.
 * A pawn promotion move by the computer enters this function after being sent by the function spChessGameSetMove.
 * A pawn promotion move by the user (either game mode 1 or 2) enters this function after first being set as a "regular move"
 * to the function spChessGameSetMove, and then after returning to the console manager - this function is called.
 * In addition, we update the relevant player's army.
 *
 * @param game         - A Chess game
 * @param move         - The pawn promotion move to be set in the game
 * @param computerMove - Indicates if it is a move done by the computer or not (i.e. by the user), might get true only on game mode 1
 *
 * @return
 * 		The SP_CHESS_GAME_MESSAGE that was returned from the function changePlayerAndUpdateStatus.
 */
SP_CHESS_GAME_MESSAGE spChessGameSetPawnPromotion(SPChessGame* game, Move* move, bool computerMove) {
	char pawnType;
	char piece = move->srcPiece;

	// updates position because didn't go through the "regular move route"
	if (computerMove)
		game->board[move->srcRow][move->srcCol] = EMPTY_POSITION;

	pawnType = (game->currentPlayer == WHITE_PLAYER) ? WHITE_PAWN : BLACK_PAWN;

	// update the game board with the piece that the pawn was promoted to
	game->board[move->dstRow][move->dstCol] = piece;

	// increasing the number of the piece replacing the pawn in the player's army (sending true means increasing)
	updatePiecesAmount(game, move->srcPiece, true);

	// decreasing the number of pawns in the player's army (sending false means decreasing)
	updatePiecesAmount(game, pawnType, false);

	return changePlayerAndUpdateStatus(game);
}

/**
 * The function checks if an "undo" command can be executed.
 *
 * @param game      - A Chess game.
 * @param userColor - The player color of the user
 *
 * @precondition - game is not NULL
 *
 * @return
 * 		False - If the history of the game is empty, or if the user playing against the computer plays the BLACK player
 * 				 and there was only one move made so far (by the computer).
 * 		True  - Otherwise.
 */
bool spChessGameIsUndoPossible(SPChessGame* game, int userColor) {
	return !(   (spArrayListIsEmpty(game->history))
			 || ((userColor == BLACK_PLAYER) && (spArrayListSize(game->history) == ONLY_ONE_MOVE)));
}

/**
 * The function removes from the game board the last chess move that was set.
 * In case of we remove a castle move, we update the board when calling the helper function undoCastleMove.
 * Otherwise, we remove we update the board in this function.
 * In addition, we update the castling indicators, the king position (if relevant, i.e.
 * if a king was involved in the move), the current player, the game status and the relevant player's army.
 *
 * @param game - A Chess game
 * @param move - The last move that was set and should be removed
 *
 * @return
 * 		SP_CHESS_GAME_INVALID_ARGUMENT - If game is NULL or move is NULL
 * 		SP_CHESS_GAME_SUCCESS          - Otherwise
 */
SP_CHESS_GAME_MESSAGE spChessGameUndoPrevMove(SPChessGame* game, Move* move) {
	if ((game == NULL) || (move == NULL))
		return SP_CHESS_GAME_INVALID_ARGUMENT;

	// updating the game board
	if (move->castleMove)
		undoCastleMove(game, move);
	else {
		char originalSrcPiece;

		if (move->pawnPromotion)
			originalSrcPiece = ((!(game->currentPlayer)) == WHITE_PLAYER) ? WHITE_PAWN : BLACK_PAWN;
		else
			originalSrcPiece = move->srcPiece;

		game->board[move->srcRow][move->srcCol] = originalSrcPiece;
		game->board[move->dstRow][move->dstCol] = move->dstPiece;
	}

	// recovering the castling indicatiors that were saved when the move was set
	game->whiteLeftCastle  = move->whiteLeftCastle;
	game->whiteRightCastle = move->whiteRightCastle;
	game->blackLeftCastle  = move->blackLeftCastle;
	game->blackRightCastle = move->blackRightCastle;

	// recover the relevant king's position if it was moved
	if (move->srcPiece == WHITE_KING) {
		game->whiteKingRow = move->srcRow;
		game->whiteKingCol = move->srcCol;
	}
	else if (move->srcPiece == BLACK_KING) {
		game->blackKingRow = move->srcRow;
		game->blackKingCol = move->srcCol;
	}

	spArrayListRemoveLast(game->history); // removing the last move from the game history

	// if not a castle and a piece was eaten - recounting the number of pieces for the "eaten" player
	if (!(move->castleMove)) {
		if (move->dstPieceCaptured)
			updatePiecesAmount(game, move->dstPiece, true);

		if (move->pawnPromotion) {
			updatePiecesAmount(game, move->srcPiece, false);

			char pawnType = (game->currentPlayer == WHITE_PLAYER) ? WHITE_PAWN : BLACK_PAWN;
			updatePiecesAmount(game, pawnType, true);
		}
	}

	// updating the cuurent player to be the opposite player
	char currPlayer = game->currentPlayer = !(game->currentPlayer);
	int kingRow = (currPlayer == WHITE_PLAYER) ? game->whiteKingRow : game->blackKingRow ;
	int kingCol = (currPlayer == WHITE_PLAYER) ? game->whiteKingCol : game->blackKingCol ;

	// update the new game status (for sure iw was not finished before the removed move)
	game->status = (rivalPlayerThreateningPosition(game, kingRow, kingCol)) ? GAME_NOT_FINISHED_CHECK : GAME_NOT_FINISHED_NO_CHECK;

	return SP_CHESS_GAME_SUCCESS;
}

/**
 * The function sets the possible moves for the piece that is located in a given position on board.
 * For doing so, the function calls to the helper function getPossiblePieceMoves.
 *
 * @param game               - A Chess game
 * @param piecePossibleMoves - The array list that will be filled with possible moves by the relevant piece
 * @param row                - The row in the game board (0-based) of the piece's position to get its possible moves
 * @param col                - The column in the game board (0-based) of the piece's position to get its possible moves
 *
 * @return
 * 		SP_CHESS_GAME_MEMORY_FAILURE   - If game is NULL or piecePossibleMoves is NULL
 * 		SP_CHESS_GAME_INVALID_POSITION - If the position the was received (row,col) is not valid
 * 		SP_CHESS_GAME_INVALID_ARGUMENT - If the piece at the given position (row,col) doesn't contain a piece that belongs to the current player
 *
 * 		Otherwise, returns the SP_CHESS_GAME_MESSAGE that was returned from the function getPossiblePieceMoves.
 */
SP_CHESS_GAME_MESSAGE spChessGetPossibleMoves(SPChessGame* game, SPArrayList* piecePossibleMoves, int row, int col) {
	if ((game == NULL) || (piecePossibleMoves == NULL))
		return SP_CHESS_GAME_MEMORY_FAILURE;

	if (!validPosition(row, col))
		return SP_CHESS_GAME_INVALID_POSITION;

	char pieceType  = game->board[row][col];

	if (!currentPlayerPiece(game->currentPlayer, pieceType))
		return SP_CHESS_GAME_INVALID_ARGUMENT;

	/* True is sent at the 6th parameter so the moves will get sorted.
	   False is sent at the last parameter function is not called from the Minimax algorithm */
	return getPossiblePieceMoves(game, piecePossibleMoves, row, col, pieceType, true, false);
}
