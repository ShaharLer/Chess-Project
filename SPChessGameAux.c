#include "SPChessGameAux.h"

/**
 * The function initializes A Chess game board (using the defined macros).
 *
 * @param board - The game board to initialize
 */
void initializeBoard(char board[][BOARD_LENGTH]) {
	// placing the empty squares on the board
	for (int row = FIRST_EMPTY_ROW_AT_INIT; row < (BOARD_LENGTH - FIRST_EMPTY_ROW_AT_INIT); row++)
		for (int col = 0; col < BOARD_LENGTH; col++)
			board[row][col] = EMPTY_POSITION;

	// placing the pieces on the board
	for (int col = 0; col < BOARD_LENGTH; col++) {
		board[WHITE_PAWNS_ROW][col] = WHITE_PAWN;
		board[BLACK_PAWNS_ROW][col] = BLACK_PAWN;

		switch (col) {
			case LEFT_ROOK_COL:
			case RIGHT_ROOK_COL:
				board[WHITE_FIRST_ROW][col] = WHITE_ROOK;
				board[BLACK_FIRST_ROW][col] = BLACK_ROOK;
				break;
			case LEFT_KNIGHT_COL:
			case RIGHT_KNIGHT_COL:
				board[WHITE_FIRST_ROW][col] = WHITE_KNIGHT;
				board[BLACK_FIRST_ROW][col] = BLACK_KNIGHT;
				break;
			case LEFT_BISHOP_COL:
			case RIGHT_BISHOP_COL:
				board[WHITE_FIRST_ROW][col] = WHITE_BISHOP;
				board[BLACK_FIRST_ROW][col] = BLACK_BISHOP;
				break;
			case QUEEN_COL:
				board[WHITE_FIRST_ROW][col] = WHITE_QUEEN;
				board[BLACK_FIRST_ROW][col] = BLACK_QUEEN;
				break;
			case KING_COL:
				board[WHITE_FIRST_ROW][col] = WHITE_KING;
				board[BLACK_FIRST_ROW][col] = BLACK_KING;
		}
	}
}

/**
 * The function checks the legality of a "regular" move (not a castle move, which has its own function for checking its legality).
 * The technique used for checking the move legality is terms of game logic is:
 * First, we verify that the piece to be moved from the source position is trying to do so without violating the Chess rules for its
 * specific movement rules by calling to the helper function legalMovementByPieceType (each type in chess has its own legal movement rules).
 * Second, we set the move (i.e. updating the game board as if the move was set) and then use the helper function rivalPlayerThreateningPosition
 * to see if the player puts himslef under "check" by executing this move (which is of course illegal according to Chess rules).
 * At the end, the game board is restored to the state it was before entering this function.
 *
 * @param game - A Chess game
 * @param move - The move to be set in the game
 *
 * @return
 * 		True  - If the move that was received as a parameter is legal.
 * 		False - Otherwise.
 */
bool legalRegularMove(SPChessGame* game, Move* move) {
	char srcPiece   = move->srcPiece;
	int  currPlayer = game->currentPlayer;
	bool isLegal    = false; // initializes a default value

	// updating the destination piece of the move in this function because its legality needs to be checked
	char dstPiece = move->dstPiece = game->board[move->dstRow][move->dstCol];

	// this condition also covers the check that the source position and destination position are not equal
	if (currentPlayerPiece(currPlayer, dstPiece))
		return false;

	if (!legalMovementByPieceType(game, move))
		return false;

	if (dstPiece != EMPTY_POSITION)
		move->dstPieceCaptured = true; // updating this indicator for the use of get_moves command by the user

	// update the board as if the move is executed.
	// We don't bother taking pawn promotion into consideration since this is irrelevant for the check legality test
	// (promoting a pawn of the current player that makes the move shouldn't cause the king of the current player be under check).
	game->board[move->srcRow][move->srcCol] = EMPTY_POSITION;
	game->board[move->dstRow][move->dstCol] = srcPiece;

	if ((srcPiece == WHITE_KING) || (srcPiece == BLACK_KING))
		isLegal = !rivalPlayerThreateningPosition(game, move->dstRow, move->dstCol); // sending new king's position
	else {
		int kingRow = (currPlayer == WHITE_PLAYER) ? game->whiteKingRow : game->blackKingRow;
		int kingCol = (currPlayer == WHITE_PLAYER) ? game->whiteKingCol : game->blackKingCol;
		isLegal = !rivalPlayerThreateningPosition(game, kingRow, kingCol);           // sending current king's position

		if (isLegal) {
			// updating this indicator for the use of get_moves command by the user
			move->threatenedAfterMove = rivalPlayerThreateningPosition(game, move->dstRow, move->dstCol);

			if (   ((currPlayer == WHITE_PLAYER) && (move->srcPiece == WHITE_PAWN) && (move->dstRow == BLACK_FIRST_ROW))
				|| ((currPlayer == BLACK_PLAYER) && (move->srcPiece == BLACK_PAWN) && (move->dstRow == WHITE_FIRST_ROW))) {

						move->pawnPromotion = true; // helps us informing the user a "pawn promotion" occured
			}
		}
	}

	// Restore the board to its original state
	game->board[move->srcRow][move->srcCol] = srcPiece;
	game->board[move->dstRow][move->dstCol] = dstPiece;

	return isLegal;
}

/**
 * The function calls the relevant helper function, according to the type of the piece to be moved from the source position,
 * to check if that piece is about to move without violating its specicfic Chess rules for movement on board.
 * The bishop, rook and queen are the only ones that can move towards a route - because of that, in order to check the legality
 * of their movement there is a special helper function that takes advantage of the similarities in their movement.
 *
 * @param game - A Chess game
 * @param move - The move to be set in the game
 *
 * @return
 * 		True  - If the movement by the piece is legal.
 * 		False - Otherwise.
 */
bool legalMovementByPieceType(SPChessGame* game, Move* move) {
	switch (move->srcPiece) {
		case WHITE_PAWN:
		case BLACK_PAWN:
			return legalMovementByPawn(game->board, move, game->currentPlayer);

		case WHITE_KNIGHT:
		case BLACK_KNIGHT:
			return legalMovementByKnight(move);

		case WHITE_BISHOP:
		case BLACK_BISHOP:
		case WHITE_ROOK:
		case BLACK_ROOK:
		case WHITE_QUEEN:
		case BLACK_QUEEN:
			return legalMovementByBishopRookQueen(game->board, move);

		case WHITE_KING:
		case BLACK_KING:
			return legalMovementByKing(move);

		default:
			return false;
	}
}

/**
 * The function checks if the movement of the pawn in the given move is legal according to Chess rules.
 * Pawn movement is special because of the fact that it is the only piece on board that cannot "eat" in the same direction that
 * it can move to an empty position ("eating" is diagonally, advancing to an empty position is by moving straight).
 *
 * @param board      - The current game board
 * @param move       - The move to be set with the pawn
 * @param currPlayer - The player that is trying to set the move
 *
 * @return
 * 		True  - If the movement by the pawn is legal.
 * 		False - Otherwise.
 */
bool legalMovementByPawn(char board[][BOARD_LENGTH], Move* move, int currPlayer) {
	int srcRow    = move->srcRow;
	int dstRow    = move->dstRow;
	int srcCol    = move->srcCol;
	int dstCol    = move->dstCol;
	char dstPiece = move->dstPiece;

	return !(    // white pawn cannot adavance more than 1 row upwards (can adavance 2 only if it is its first move)
				(   (currPlayer == WHITE_PLAYER)
			     && (dstRow != (srcRow + 1))
				 && !((dstRow == (srcRow + 2)) && (srcRow == WHITE_PAWNS_ROW) && (dstCol == srcCol) && (board[srcRow + 1][dstCol] == EMPTY_POSITION)))

				 // black pawn cannot adavance more than 1 row downwards (can adavance 2 only if it is its first move)
			 || (   (currPlayer == BLACK_PLAYER)
			     && (dstRow != (srcRow - 1))
			     && !((dstRow == (srcRow - 2)) && (srcRow == BLACK_PAWNS_ROW) && (dstCol == srcCol) && (board[srcRow - 1][dstCol] == EMPTY_POSITION)))

			     // a pawn cannot move (if either "eating" a rival piece or if not) more than 1 column to the left/right
			 || (   (dstCol < (srcCol - 1))
				 || (dstCol > (srcCol + 1)))

				 // a pawn cannot advance straight if it is "blocked" by another piece
			 || (   (dstCol == srcCol)
				 && (dstPiece != EMPTY_POSITION))

				 // a pawn cannot advance diagonally if it is not for "eating" a rival piece
			 || (   (dstCol != srcCol)
				 && !rivalPiece(currPlayer, dstPiece)));
}

/**
 * The function checks if the movement of the knight in the given move is legal according to Chess rules.
 * In general, the knight can move in an "L" shape to 8 different postions respectively to where it is located (even though
 * not always all of the 8 positions are possible in terms of their presence on board).
 * What is common to all of these 8 moves are the facts that the knight cannot move more than 2 rows/columns upwards/downwards,
 * and that's why in the function we calculate the absolute distances between the knight's row/column at the source position and
 * the knight's row/column at the destination position.
 *
 * @param move - The move to be set with the knight
 *
 * @return
 * 		True  - If the movement by the knight is legal (movement the creates "L" shape)
 * 		False - Otherwise.
 */
bool legalMovementByKnight(Move* move) {
	int rowDifference = abs(move->dstRow - move->srcRow);
	int colDifference = abs(move->dstCol - move->srcCol);

	return !(   ((rowDifference != 1) && (rowDifference != 2))   // knight cannot move more then 2 rows upwards/downwards
			 ||	((colDifference != 1) && (colDifference != 2))   // knight cannot move more then 2 columns upwards/downwards
			 || ((rowDifference == 1) && (colDifference != 2))   // if knight moved 1 row upwards/downwards - it must move 2 columns sidewards
			 || ((rowDifference == 2) && (colDifference != 1))); // if knight moved 2 rows upwards/downwards - it must move 1 column sidewards
}

/**
 * The function checks if the movement of the bishop/rook/queen in the given move is legal according to Chess rules.
 * The bishop, rook and queen are the only ones that can move towards a route - and this function is meant to take
 * advantage of the similarities in their movement.
 * In addition, the queen's movement is said to be the union of the bishop's and the rook's movements:
 * The bishop can only move diagonally on the board to each of in each of the four possible
 * directions, while the rook can only move a straight way in each of the four possible
 * directions (either horizontally ot vertically).
 * The function determines a delta row and delta column variables that represent the change between two sequential positions in
 * the direction the piece is trying to reach in the given move. These two variables will be sent to the helper function vacantRouteToDst
 * that will check the actual movement legality of the piece in the given move (i.e. that the route to the destination position is vacant).
 *
 * @param board - The current game board
 * @param move  - The move to be set with the either bishop, the rook of the queen
 *
 * @return
 * 		True  - If the movement by the bishop/rook/queen is legal (i.e. the route to the destination position is vacant and
 * 		         the destination position is not occupied by a piece that belongs to the same player).
 * 		False - Otherwise.
 */
bool legalMovementByBishopRookQueen(char board[][BOARD_LENGTH], Move* move) {
	int srcRow = move->srcRow, srcCol = move->srcCol, dstRow = move->dstRow, dstCol = move->dstCol;
	int srcPiece = board[srcRow][srcCol];
	int rowDifference = abs(dstRow - srcRow); // the absolute value of the distance between the source row and the destination row
	int colDifference = abs(dstCol - srcCol); // the absolute value of the distance between the source column and the destination column
	int deltaRow, deltaCol;

	switch (srcPiece) {
		case WHITE_BISHOP:
		case BLACK_BISHOP:
			if (rowDifference != colDifference) // should always be equal on a bishop movement (diagonal movement)
				return false;

			deltaRow = (dstRow > srcRow) ? 1 : (-1); // gets 1 for moving right in the diagonal, -1 for moving left in the diagonal
			deltaCol = (dstCol > srcCol) ? 1 : (-1); // gets 1 for moving up in the diagonal, -1 for moving down in the diagonal
			break;
		case WHITE_ROOK:
		case BLACK_ROOK:
			if ((srcRow != dstRow) && (srcCol != dstCol)) // should always end it's movement either on the same row or on the same column
				return false;

			if (dstRow == srcRow) {
				deltaRow = 0;
				deltaCol = (dstCol > srcCol) ? 1 : (-1); // gets 1 if the movements is to the right, -1 if the movement is to the left
			}
			else {
				deltaCol = 0;
				deltaRow = (dstRow > srcRow) ? 1 : (-1); // gets 1 if the movements is upwards, -1 if the movement is downwards
			}
			break;
		case WHITE_QUEEN:
		case BLACK_QUEEN:
			if (rowDifference == colDifference) {        // in case the queen moves similar to a bishop's movement
				deltaRow = (dstRow > srcRow) ? 1 : (-1);
				deltaCol = (dstCol > srcCol) ? 1 : (-1);
			}
			// illegal move if the movement by the queen is not similar to a bishop's movement nor similar to a rook's movement
			else if ((srcRow != dstRow) && (srcCol != dstCol))
				return false;
			else {                                       // in case the queen moves similar to a rooks's movement
				if (dstRow == srcRow) {
					deltaRow = 0;
					deltaCol = (dstCol > srcCol) ? 1 : (-1);
				}
				else {
					deltaCol = 0;
					deltaRow = (dstRow > srcRow) ? 1 : (-1);
				}
			}
			break;
		default:
			return false;
	}

	return (vacantRouteToDst(board, move, deltaRow, deltaCol));
}

/**
 * The function checks if the route from the source position to the destination position is vacant,
 * doing so by using its parameters deltaRow and daltaCol that indicate in which directions the route to the destination position is.
 *
 * @param board    - The current game board
 * @param move     - The move to be set with the either bishop, the rook of the queen
 * @param deltaRow - The direction of the checked route in terms of up/down (equals 1 for up, -1 for down and 0 for staying at the same row)
 * @param deltaCol - The direction of the checked route in terms of up/down (equals 1 for right, -1 for left and 0 for staying at the same column)
 *
 * @return
 * 		True  - If route from the source position to the destination position is vacant.
 * 		False - Otherwise.
 */
bool vacantRouteToDst(char board[][BOARD_LENGTH], Move* move, int deltaRow, int deltaCol) {
	int rowToCheck = move->srcRow;
	int colToCheck = move->srcCol;
	int dstRow = move->dstRow;
	int dstCol = move->dstCol;
	bool reachedDest;

	/* We advance along the direction, advancing by deltaRow,deltaCol in each iteration.
	   We will stop once we no longer hit an empty position or when we reached the destination position. */
	do {
		rowToCheck += deltaRow;
		colToCheck += deltaCol;
		reachedDest = ((rowToCheck == dstRow) && (colToCheck == dstCol));
	} while (!reachedDest && (board[rowToCheck][colToCheck] == EMPTY_POSITION));

	return reachedDest;
}

/**
 * The function checks if the movement of the knight in the given move is legal according to Chess rules.
 * In general, the knig can move to 8 different postions respectively to where it is located (even though not always
 * all of the 8 positions are possible in terms of their presence on board).
 * These 8 positions are all the positions on board that are 1 square away from the king's position (i.e. 1 step
 * upwards/downwards/to the left/to the right/diagonally).
 * For that reason, to verify the legality of the move, we calculate the absolute distances between the king's row/column
 * at the source position and the kins's row/column at the destination position.
 *
 * @param move - The move to be set with the knight
 *
 * @return
 * 		True  - If the movement by the king is legal (movement the creates "L" shape).
 * 		False - Otherwise.
 */
bool legalMovementByKing(Move* move) {
	int rowDifference = abs(move->dstRow - move->srcRow);
	int colDifference = abs(move->dstCol - move->srcCol);

	return ((rowDifference <= 1) && (colDifference <= 1));
}

/**
 * The function sets a "regular" move (not a castle move, which has its own set function).
 * After updating the game board, if the "source piece" (i.e. the piece that was moved by the player) is a rook or a king,
 * the function makes updates for the castling indicators of the game structure by either:
 * 1) call to the helper function updateCastlingIndicators if the piece is a rook.
 * 2) updating the relevant indicators here in the function (including and update of the new king's position).
 *
 * @param game - A Chess game
 * @param move - The move to be set in the game
 */
void setRegularMove(SPChessGame* game, Move* move) {
	game->board[move->srcRow][move->srcCol] = EMPTY_POSITION; // the position of the "source piece" gets empty
	game->board[move->dstRow][move->dstCol] = move->srcPiece; // the destination position gets filles by the "source piece"

	char srcPiece = move->srcPiece;

	// updates after regular move
	switch (srcPiece) {
		case WHITE_ROOK:
			updateCastlingIndicators(move, srcPiece, &(game->whiteLeftCastle), &(game->whiteRightCastle));
			break;

		case BLACK_ROOK:
			updateCastlingIndicators(move, srcPiece, &(game->blackLeftCastle), &(game->blackRightCastle));
			break;

		case WHITE_KING:
			game->whiteKingRow     = move->dstRow;
			game->whiteKingCol     = move->dstCol;

			// castling from both sides is not available anymore
			game->whiteRightCastle = false;
			game->whiteLeftCastle  = false;

			break;

		case BLACK_KING:
			game->blackKingRow     = move->dstRow;
			game->blackKingCol     = move->dstCol;

			// castling from both sides is not available anymore
			game->blackRightCastle = false;
			game->blackLeftCastle  = false;
	}
}

/**
 * The function checks the legality of a castle move.
 * The technique used for checking the move legality is terms of game logic is:
 * First, we verify that the player that will castle at this move is not currently
 * under "check" (we do so by checking the game status) - illegal according to the Chess rules.
 * Then, if the player is not under "check" verify that the option to make a castle move may still be executed (i.e. neither
 * the player's king moved already during the game nor the rook that the player try to castle with its king).
 * We do so by checking the relevant castling indicator.
 * At the end, if the intended castle night still be executed, we call the helper function legalCastleRoute that to determine
 * if the csatle move is legal.
 *
 * @param game - A Chess game
 * @param move - The castle move to be set in the game
 *
 * @precondition - The piece that is moved from the source position is either a white rook or a black rook
 *
 * @return
 * 		True  - If the move that was received as a parameter is a legal castle move.
 * 		False - Otherwise.
 */
bool legalCastle(SPChessGame* game, Move* move) {
	if (game->status == GAME_NOT_FINISHED_CHECK) // castle is not legal if player is under check
		return false;

	int srcRow = move->srcRow;
	int srcCol = move->srcCol;
	char rookType = move->srcPiece;
	int  rowToCheck;
	char kingType;
	bool leftCastle, rightCastle;

	if (rookType == WHITE_ROOK) {
		rowToCheck      = WHITE_FIRST_ROW;
		kingType        = WHITE_KING;
		leftCastle      = game->whiteLeftCastle;
		rightCastle     = game->whiteRightCastle;
	}
	else {
		rowToCheck      = BLACK_FIRST_ROW;
		kingType        = BLACK_KING;
		leftCastle      = game->blackLeftCastle;
		rightCastle     = game->blackRightCastle;
	}

	// check if castle is still possible
	if (   !((srcRow == rowToCheck) && (srcCol == LEFT_ROOK_COL)  && (leftCastle))
		&& !((srcRow == rowToCheck) && (srcCol == RIGHT_ROOK_COL) && (rightCastle))) {

				return false;
	}

	return legalCastleRoute(game, rookType, srcCol, kingType);
}

/**
 * The function checks if the route between the rook and the king in the intended castle move keeps A Chess game rules.
 * The technique used for checking the that legality of the route between the rook and the king is legal is:
 * First, we check if the castle that is tried to be executed is a left castle (i.e. a castle from the left side of game the board)
 * or a right castle (i.e. a castle from the right side of game the board).
 * Then, we go through every position on the route between the king and the relevant rook (not including their positions - the fact that the king
 * is not threatened at its place at the moment is a precondition of this function).
 * For each such position, we verify that it is empty and that in each one of them - the king is not threatened (i.e. the player
 * would not have been under check if the king was placed at that position).
 * The route is considered legal (and hence the castle move) if all the positions on
 * the relevant route fulfilled the conditions that were mentioned above.
 *
 * @param game     - A Chess game
 * @param rookType - The rook that belongs to the player, that is trying to make a castle with its king
 * @param rookCol  - The column of the rook that is represented in the rookType parameter
 * @param kingType - The king that belongs to the current player that plays the castle move
 *
 * @precondition - The player to make the move is not under "check" (i.e. its king is not threatened at the current game board)
 *
 * @return
 * 		True  - If the route between the rook and the king is a legal castle route.
 * 		False - Otherwise.
 */
bool legalCastleRoute(SPChessGame* game, int rookType, int rookCol, char kingType) {
	int rowOfCastle = (rookType == WHITE_ROOK) ? WHITE_FIRST_ROW    : BLACK_FIRST_ROW;
	int kingCol     = (rookType == WHITE_ROOK) ? game->whiteKingCol : game->blackKingCol;


	int  startOfKingCheckPath, endOfKingCheckPath;
	bool isLegal = false;

	if (rookCol < kingCol) {    // left castle
		startOfKingCheckPath = (rookCol + 1);
		endOfKingCheckPath   = (kingCol - 1);
	}
	else {                     // right castle
		startOfKingCheckPath = (kingCol + 1);
		endOfKingCheckPath   = (rookCol - 1);
	}

	// check if the path between the rook and the king is vacant, and the king is not threatened in each of the route's positions
	for (int colOfPath = startOfKingCheckPath; colOfPath <= endOfKingCheckPath; colOfPath++) {
		if (game->board[rowOfCastle][colOfPath] != EMPTY_POSITION)
			return false;

		// update the board to see if the king is threatened at the next position of the castle route
		game->board[rowOfCastle][kingCol]   = EMPTY_POSITION;
		game->board[rowOfCastle][colOfPath] = kingType;

		isLegal = !rivalPlayerThreateningPosition(game, rowOfCastle, colOfPath); // valid if king does not pass through or lands into check

		// restore game board
		game->board[rowOfCastle][kingCol]   = kingType;
		game->board[rowOfCastle][colOfPath] = EMPTY_POSITION;

		if (!isLegal)
			return false;
	}

	return isLegal;
}


/**
 * The function sets a castle move.
 * According to the roo'ks position we know if it is a left castle or a right castle,
 * and unpdate accordingly the relevant king's new position.
 *
 * @param game - A Chess game
 * @param move - The castle move to be set in the game
 */
void setCastleMove(SPChessGame* game, Move* move) {
	char rookType   = move->srcPiece;
	char kingType   = (rookType == WHITE_ROOK) ? WHITE_KING : BLACK_KING;
	int  castleRow  = move->srcRow;
	int  dstKingCol, dstRookCol;

	if (move->srcCol == LEFT_ROOK_COL) { // left castle
		dstKingCol = (KING_COL - 2);     // the new king's column after a left castle
		dstRookCol = (KING_COL - 1);     // the new rook's column after a left castle
	}
	else {                               // right castle
		dstKingCol = (KING_COL + 2);     // the new king's column after a right castle
		dstRookCol = (KING_COL + 1);     // the new rook's column after a right castle
	}

	// update the game board according to the castle move
	game->board[castleRow][dstKingCol]   = kingType ;
	game->board[castleRow][move->srcCol] = EMPTY_POSITION;
	game->board[castleRow][dstRookCol]   = rookType;
	game->board[castleRow][KING_COL]     = EMPTY_POSITION;

	move->dstCol = dstRookCol;

	// updates of the new king's column and the relevant castling indicators (by calling to the helper function)
	if (rookType == WHITE_ROOK) {
		game->whiteKingCol = dstKingCol;
		updateCastlingIndicators(move, rookType, &(game->whiteLeftCastle), &(game->whiteRightCastle));
	}
	else {
		game->blackKingCol = dstKingCol;
		updateCastlingIndicators(move, rookType, &(game->blackLeftCastle), &(game->blackRightCastle));
	}
}

/**
 * The function makes the relevant updates that should be made after a setting a move (every king of move).
 * If the move is not a pawn promotion move, the function calls at its end to the helper function changePlayerAndUpdateStatus,
 * that can fail if a memory error occurs (that's why the function returns an SP_CHESS_GAME_MESSAGE.
 * The upadtes of the next player to play and the game status happen after a pawn promotion move happen in its own set function.
 *
 * @param game        - A Chess game
 * @param move        - The move that was set
 * @param history     - The array list the includes the history of the game moves
 * @param minimaxMove - Indicates of the move was set during the Minimaax algorithm
 *
 * @return
 * 		If the move is a not a pawn promotion move - return the result that is returned from the
 * 		 helper function changePlayerAndUpdateStatus.
 *
 * 		SP_CHESS_GAME_SUCCESS - Otherwise.
 */
SP_CHESS_GAME_MESSAGE updatesAfterSettingMove(SPChessGame* game, Move* move, SPArrayList* history, bool minimaxMove) {
	// updates if a piece was "eaten" at the destination position
	if (move->dstPieceCaptured) {
		updatePiecesAmount(game, move->dstPiece, false);

		char dstPiece = move->dstPiece;

		if (dstPiece == WHITE_ROOK)      // an update for one of the white player's castling indicator
			updateCastlingIndicators(move, dstPiece, &(game->whiteLeftCastle), &(game->whiteRightCastle));
		else if (dstPiece == BLACK_ROOK) // an update for one of the black player's castling indicator
			updateCastlingIndicators(move, dstPiece, &(game->blackLeftCastle), &(game->blackRightCastle));
	}

	/**
	 * saving move to history (during the Minimax algorithm we don't save the
	 * moves in the history, because an undo is not executed during there
	 */
	if (!minimaxMove) {
		if (spArrayListIsFull(history))
			spArrayListRemoveFirst(history);

		spArrayListAddLast(history, move);
	}

	if (!move->pawnPromotion)
		return changePlayerAndUpdateStatus(game);

	return SP_CHESS_GAME_SUCCESS;
}

/**
 * The function makes the relevant updates that should be made after a setting a move (every king of move).
 * If the move is not a pawn promotion move, the function calls at its end to the helper function changePlayerAndUpdateStatus,
 * that can fail if a memory error occurs (that's why the function returns an SP_CHESS_GAME_MESSAGE.
 * The upadtes of the next player to play and the game status happen after a pawn promotion move happen in its own set function.
 *
 * @param game             - A Chess game
 * @param pieceType        - The type of Chess piece
 * @param isIncreaseAmount - Indicates if the amount of the given piece in the relevant "army" should be increased or decreased
 */
void updatePiecesAmount(SPChessGame* game, char pieceType, bool isIncreaseAmount) {
	switch (pieceType) {
		case WHITE_PAWN:
			(isIncreaseAmount) ? game->whiteArmy->numOfPawns++ : game->whiteArmy->numOfPawns--;
			break;

		case BLACK_PAWN:
			(isIncreaseAmount) ? game->blackArmy->numOfPawns++ : game->blackArmy->numOfPawns--;
			break;

		case WHITE_KNIGHT:
			(isIncreaseAmount) ? game->whiteArmy->numOfKnights++ : game->whiteArmy->numOfKnights--;
			break;

		case BLACK_KNIGHT:
			(isIncreaseAmount) ? game->blackArmy->numOfKnights++ : game->blackArmy->numOfKnights--;
			break;

		case WHITE_BISHOP:
			(isIncreaseAmount) ? game->whiteArmy->numOfBishops++ : game->whiteArmy->numOfBishops--;
			break;

		case BLACK_BISHOP:
			(isIncreaseAmount) ? game->blackArmy->numOfBishops++ : game->blackArmy->numOfBishops--;
			break;

		case WHITE_ROOK:
			(isIncreaseAmount) ? game->whiteArmy->numOfRooks++ : game->whiteArmy->numOfRooks--;
			break;

		case BLACK_ROOK:
			(isIncreaseAmount) ? game->blackArmy->numOfRooks++ : game->blackArmy->numOfRooks--;
			break;

		case WHITE_QUEEN:
			(isIncreaseAmount) ? game->whiteArmy->numOfQueens++ : game->whiteArmy->numOfQueens--;
			break;

		case BLACK_QUEEN:
			(isIncreaseAmount) ? game->blackArmy->numOfQueens++ : game->blackArmy->numOfQueens--;
	}
}

/**
 * The function update the relevant casling indicators if needed: Either the indicator of the left castle or the right castle.
 *
 * @param move         - The move that was set
 * @param pieceToCheck - The piece that its type will be checked to decide which update should be done
 * @param leftCastle   - A pointer to the left castle indicator
 * @param rightCastle  - A pointer to the right castle indicator
 */
void updateCastlingIndicators(Move* move, char pieceToCheck, bool* leftCastle, bool* rightCastle) {
	if (move->castleMove) { // if the move is a castle, both indicators are set to false
		*(leftCastle)  = false;
		*(rightCastle) = false;
	}
	else {
		int srcRow = move->srcRow;
		int srcCol = move->srcCol;
		int dstRow = move->dstRow;
		int dstCol = move->dstCol;
		char srcPiece  = move->srcPiece;
		char dstPiece  = move->dstPiece;
		int rowToCheck = (pieceToCheck == WHITE_ROOK) ? WHITE_FIRST_ROW : BLACK_FIRST_ROW;

		// the conditions that would make the left castle indicator to be set to false
		if (   (*(leftCastle))
			&&
			   (   ((pieceToCheck == srcPiece) && (srcRow == rowToCheck) && (srcCol == LEFT_ROOK_COL))
			    || ((pieceToCheck == dstPiece) && (dstRow == rowToCheck) && (dstCol == LEFT_ROOK_COL)))) {

						*(leftCastle)  = false;
		}
		// the conditions that would make the right castle indicator to be set to false
		else if (   (*(rightCastle))
				 &&
				    (   ((pieceToCheck == srcPiece) && (srcRow == rowToCheck) && (srcCol == RIGHT_ROOK_COL))
				     || ((pieceToCheck == dstPiece) && (dstRow == rowToCheck) && (dstCol == RIGHT_ROOK_COL)))) {

							*(rightCastle)  = false;
		}
	}
}

/**
 * The function removes from the game board a castle move.
 * According to the rook's color, the move's source column and the move's destination column -
 * we know which castle was made and which king position should we update.
 *
 * @param game - A Chess game
 * @param move - The castle move that was set and should be removed
 */
void undoCastleMove(SPChessGame* game, Move* move) {
	char kingType, rookType;
	int castleRow, currKingCol;
	int srcRookCol  = move->srcCol;
	int currRookCol = move->dstCol;

	if (move->srcPiece == WHITE_ROOK) {
		kingType    = WHITE_KING;
		rookType    = WHITE_ROOK;
		castleRow   = WHITE_FIRST_ROW;
		currKingCol = game->whiteKingCol;
	}
	else {
		kingType    = BLACK_KING;
		rookType    = BLACK_ROOK;
		castleRow   = BLACK_FIRST_ROW;
		currKingCol = game->blackKingCol;
	}

	// recover the game board to the state before the castle move
	game->board[castleRow][KING_COL]    = kingType;
	game->board[castleRow][currKingCol] = EMPTY_POSITION;
	game->board[castleRow][srcRookCol]  = rookType;
	game->board[castleRow][currRookCol] = EMPTY_POSITION;

	// update the relevant king position
	if (kingType == WHITE_KING)
		game->whiteKingCol = KING_COL;
	else
		game->blackKingCol = KING_COL;
}

/**
 * The function update the next player to play a move.
 * In addition, the function updates the current game status, doing so by calling the helper function getGameStatus.
 *
 * @param game - A Chess game
 *
 * @return
 * 		SP_CHESS_MEMORT_FAILURE - If there eas a memory error while calculating the
 * 		                           current game status by the helper function.
 * 		SP_CHESS_GAME_SUCCESS   - Otherwise.
 */
SP_CHESS_GAME_MESSAGE changePlayerAndUpdateStatus(SPChessGame* game) {
	game->currentPlayer = !(game->currentPlayer); // because white/black players are represented with 1/0

	game->status = (game->currentPlayer == WHITE_PLAYER) ? getGameStatus(game, game->whiteKingRow, game->whiteKingCol)
														 : getGameStatus(game, game->blackKingRow, game->blackKingCol);

	if (game->status == MEMORY_FAILURE)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	return SP_CHESS_GAME_SUCCESS;
}

/**
 * The function checks what is the current game status and returns it.
 * The technique to do so is to:
 * First, call a helper function that returns if the next player to play is under "check" at the current game state.
 * Second, by callig the helper function getPossiblePieceMoves, we see if the the next player to play has at least 1 possible move to make.
 * If the next player to play has no possible moves either he has lost (under "checkmate") or the game is tied ("stalemate").
 * Otherwise, either he is under "check" or there is no "check".
 *
 * @param game    - A Chess game
 * @param kingRow - The row of the king that belongs to the next player to play
 * @param kingCol - The column of the king that belongs to the next player to play
 *
 * @return
 * 		MEMORY_FAILURE             - If there eas a memory error during the run of the function
 * 		                              itself or during the run of its helper function.
 * 		WHITE_PLAYER_WINS          - If the white player is the winner of the game.
 * 		BLACK_PLAYER_WINS          - If the black player is the winner of the game.
 * 		GAME_NOT_FINISHED_CHECK    - If the next player to play is under "check" (i.e. his king is currently threatened)
 * 								      but he can still produce at least one legal move.
 * 		GAME_NOT_FINISHED_NO_CHECK - If the next player to play is not under "check" (i.e. his king is not tcurrently threatened)
 * 								      and he can still produce at least one legal move.
 * 		TIED_GAME                  - If the next player to play is not under "check" (i.e. his king is not tcurrently threatened)
 * 								      but he cannot produce anymore legal moves.
 */
GAME_STATUS getGameStatus(SPChessGame* game, int kingRow, int kingCol) {
	bool thereIsPossibleMove = false;
	bool isCheck = rivalPlayerThreateningPosition(game, kingRow, kingCol);

	SPArrayList* pieceMoves = spArrayListCreate(MAX_MOVES_FOR_PIECE); // the highest number of moves
	if (pieceMoves == NULL)
		return MEMORY_FAILURE;

	// we stop the iteration if the next player to play has at least one possible move to make
	for (int row = 0; (!thereIsPossibleMove && (row < BOARD_LENGTH)); row++) {
		for (int col = 0; (!thereIsPossibleMove && (col < BOARD_LENGTH)); col++) {
			char pieceType = game->board[row][col];
			if (!currentPlayerPiece(game->currentPlayer, pieceType))
					continue;

			if (getPossiblePieceMoves(game, pieceMoves, row, col, pieceType, false, false) == SP_CHESS_GAME_MEMORY_FAILURE) {
				spArrayListDestroy(pieceMoves);
				return MEMORY_FAILURE;
			}

			thereIsPossibleMove = !spArrayListIsEmpty(pieceMoves);
		}

		spArrayListClear(pieceMoves);
	}

	spArrayListDestroy(pieceMoves); // free the memory of the array list we allocated for the possible moves

	if (thereIsPossibleMove) {
		if (isCheck)
			return GAME_NOT_FINISHED_CHECK;          // "check" situation
		else
			return GAME_NOT_FINISHED_NO_CHECK;       // still to play
	}
	else {
		if (isCheck) {
			if (game->currentPlayer == WHITE_PLAYER)
				return BLACK_PLAYER_WINS;            // "checkmate" - black player wins
			else
				return WHITE_PLAYER_WINS;            // "checkmate" - white player wins
		}
		else
			return TIED_GAME;                        // tie
	}
}

/**
 * The function checks if given position (occupied by a piece of the currently playing player) is currently threatened by the rival player.
 * The function uses the helper function rivalPieceThreateningPosition, by calling it each time with another type of piece that
 * belongs to the rival - these way we check all the posssibilities to threaten the position.
 *
 * @param game        - A Chess game
 * @param positionRow - The row of the position that we need to check if threatend by the rival player
 * @param positionCol - The column of the position that we need to check if threatend by the rival player
 *
 * @return
 * 		TRUE  - If the position is threatend by at least one piece that belongs to the rival.
 * 		FALSE - Otherwise.
 */
bool rivalPlayerThreateningPosition(SPChessGame* game, int positionRow, int positionCol) {
	char pawnToCheck, knightToCheck, bishopToCheck, rookToCheck, queenToCheck, kingToCheck;
	int currPlayer = game->currentPlayer;

	if (currPlayer == WHITE_PLAYER) {
		pawnToCheck   = BLACK_PAWN;
		knightToCheck = BLACK_KNIGHT;
		bishopToCheck = BLACK_BISHOP;
		rookToCheck   = BLACK_ROOK;
		queenToCheck  = BLACK_QUEEN;
		kingToCheck   = BLACK_KING;
	}
	else {
		pawnToCheck   = WHITE_PAWN;
		knightToCheck = WHITE_KNIGHT;
		bishopToCheck = WHITE_BISHOP;
		rookToCheck   = WHITE_ROOK;
		queenToCheck  = WHITE_QUEEN;
		kingToCheck   = WHITE_KING;
	}

	// check if king is under threat
	return (   rivalPieceThreateningPosition(game->board, positionRow, positionCol, pawnToCheck)
			|| rivalPieceThreateningPosition(game->board, positionRow, positionCol, knightToCheck)
			|| rivalPieceThreateningPosition(game->board, positionRow, positionCol, bishopToCheck)
			|| rivalPieceThreateningPosition(game->board, positionRow, positionCol, rookToCheck)
			|| rivalPieceThreateningPosition(game->board, positionRow, positionCol, queenToCheck)
			|| rivalPieceThreateningPosition(game->board, positionRow, positionCol, kingToCheck));
}

/**
 * The function checks if the given piece threatens the given position.
 * By using a "switch case", we make a special check for every king of piece, according to its legal movement on a Chess board.
 * Note that because the queen can move like the bishop and also like the rook, we check if it threatens the position by moving
 * like a bishop or by moving like a rook.
 *
 * @param board     - A Chess game board
 * @param srcRow    - The row of the position that we need to check if threatend
 * @param srcCol    - The column of the position that we need to check if threatend
 * @param pieceType - The type of piece that we need to check if threatens the given position
 *
 * @return
 * 		TRUE  - If the given position is indeed threatend by the given piece (using the helper function pieceOccupyingPosition).
 * 		FALSE - Otherwise.
 */
bool rivalPieceThreateningPosition(char board[][BOARD_LENGTH], int srcRow, int srcCol, char pieceType) {
	switch (pieceType) {
		case WHITE_PAWN:
			// a white pawn might threaten the position from one row downwards (and by "eating" diagonally)
			return (   pieceOccupyingPosition(board, srcRow - 1, srcCol - 1, pieceType)
					|| pieceOccupyingPosition(board, srcRow - 1, srcCol + 1, pieceType));
		case BLACK_PAWN:
			// a black pawn might threaten the position from one row upwards (and by "eating" diagonally)
			return (   pieceOccupyingPosition(board, srcRow + 1, srcCol - 1, pieceType)
					|| pieceOccupyingPosition(board, srcRow + 1, srcCol + 1, pieceType));
		case WHITE_KNIGHT:
		case BLACK_KNIGHT:
			// both a white and a black knights might threaten the position from the same 8 different position (moving in the shape of an "L")
			return (   pieceOccupyingPosition(board, srcRow - 2, srcCol - 1, pieceType)
					|| pieceOccupyingPosition(board, srcRow - 2, srcCol + 1, pieceType)
					|| pieceOccupyingPosition(board, srcRow - 1, srcCol - 2, pieceType)
					|| pieceOccupyingPosition(board, srcRow - 1, srcCol + 2, pieceType)
					|| pieceOccupyingPosition(board, srcRow + 1, srcCol - 2, pieceType)
					|| pieceOccupyingPosition(board, srcRow + 1, srcCol + 2, pieceType)
					|| pieceOccupyingPosition(board, srcRow + 2, srcCol - 1, pieceType)
					|| pieceOccupyingPosition(board, srcRow + 2, srcCol + 1, pieceType));
		case WHITE_BISHOP:
		case BLACK_BISHOP:
			return  bishopOrQueenThreat(board, srcRow, srcCol, pieceType);
		case WHITE_ROOK:
		case BLACK_ROOK:
			return  rookOrQueenThreat(board, srcRow, srcCol, pieceType);
		case WHITE_QUEEN:
		case BLACK_QUEEN:
			return (   bishopOrQueenThreat(board, srcRow, srcCol, pieceType)
					|| rookOrQueenThreat(board, srcRow, srcCol, pieceType));
		case WHITE_KING:
		case BLACK_KING:
			/* both a white and a black king might threaten the position from the same 8 different position (by moving to
			   a square on the that surrounds the given position */
			return (   pieceOccupyingPosition(board, srcRow - 1, srcCol - 1, pieceType)
					|| pieceOccupyingPosition(board, srcRow - 1, srcCol    , pieceType)
					|| pieceOccupyingPosition(board, srcRow - 1, srcCol + 1, pieceType)
					|| pieceOccupyingPosition(board, srcRow    , srcCol - 1, pieceType)
					|| pieceOccupyingPosition(board, srcRow    , srcCol + 1, pieceType)
					|| pieceOccupyingPosition(board, srcRow + 1, srcCol - 1, pieceType)
					|| pieceOccupyingPosition(board, srcRow + 1, srcCol    , pieceType)
					|| pieceOccupyingPosition(board, srcRow + 1, srcCol + 1, pieceType));
	}

	return false;
}

/**
 * The function checks if the given bishop/queen threatens the given position.
 * We take advantage of the fact that the queen can move also like the rook.
 * The function is doing the checking by calling the helper function directionalThreat each time
 * with a different direction (we go from the given position to the possibly threatening piece):
 * 1) By moving diagonally downwards to the left
 * 2) By moving diagonally downwards to the right
 * 3) By moving diagonally upwards to the left
 * 4) By moving diagonally upwards to the right
 *
 * @param board     - A Chess game board
 * @param srcRow    - The row of the position that we need to check if threatend
 * @param srcCol    - The column of the position that we need to check if threatend
 * @param pieceType - The type of piece that we need to check if threatens the given position
 *
 * @return
 * 		TRUE  - If the position is threatend by the given bishop/queen in at least one of the possible directions.
 * 		FALSE - Otherwise.
 */
bool bishopOrQueenThreat(char board[][BOARD_LENGTH], int srcRow, int srcCol, char pieceType) {
	return (   directionalThreat(board, srcRow, srcCol, GOING_DOWNWARDS, GOING_LEFT , pieceType)
			|| directionalThreat(board, srcRow, srcCol, GOING_DOWNWARDS, GOING_RIGHT, pieceType)
			|| directionalThreat(board, srcRow, srcCol, GOING_UPWARDS  , GOING_LEFT , pieceType)
			|| directionalThreat(board, srcRow, srcCol, GOING_UPWARDS  , GOING_RIGHT, pieceType));
}

/**
 * The function checks if the given rook/queen threatens the given position.
 * We take advantage of the fact that the queen can move also like the rook.
 * The function is doing the checking by calling the helper function directionalThreat each time with a different
 * direction (we go from the given position to the possibly threatening piece):
 * 1) By moving vertically downwards
 * 2) By moving horizontally to the left
 * 3) By moving horizontally to the right
 * 4) By moving vertically upwards
 *
 * @param board     - A Chess game board
 * @param srcRow    - The row of the position that we need to check if threatend
 * @param srcCol    - The column of the position that we need to check if threatend
 * @param pieceType - The type of piece that we need to check if threatens the given position
 *
 * @return
 * 		TRUE  - If the position is threatend by the given rook/queen in at least one of the possible directions.
 * 		FALSE - Otherwise.
 */
bool rookOrQueenThreat(char board[][BOARD_LENGTH], int srcRow, int srcCol, char pieceType) {
	return (   directionalThreat(board, srcRow, srcCol, GOING_DOWNWARDS, SAME_COL    , pieceType)
			|| directionalThreat(board, srcRow, srcCol, SAME_ROW       , GOING_LEFT  , pieceType)
			|| directionalThreat(board, srcRow, srcCol, SAME_ROW       , GOING_RIGHT , pieceType)
			|| directionalThreat(board, srcRow, srcCol, GOING_UPWARDS  , SAME_COL    , pieceType));
}

/**
 * The function checks if the given type of piece threatens the given position in the given direction.
 * We do this checking by starting from the given position and going sqaure-by-square on the given board
 * according to the given direction: We stop when we either reached an invalid position or when we encountered
 * a piece (and in this case we check if it from the type of piece we received as a parameter.
 *
 * @param board     - A Chess game board
 * @param srcRow    - The row of the position that we need to check if threatend
 * @param srcCol    - The column of the position that we need to check if threatend
 * @param deltaRow  - The movement in terms of row that should be made when going to the next position sqare-by-square:
 * 					   Either one row up, one row down or staying at the same row
 * @param deltaCol  - The movement in terms of column that should be made when going to the next position sqare-by-square:
 * 					   Either one column up, one column down or staying at the same column
 * @param pieceType - The type of piece that we check if threatens the given position
 *
 * @return
 * 		TRUE  - If the track from the given positino was stopped on a valid position that is occupied by the given type of piece.
 * 		FALSE - Otherwise.
 */
bool directionalThreat(char board[][BOARD_LENGTH], int srcRow, int srcCol, int deltaRow, int deltaCol, char pieceType) {
	int currRowToCheck = srcRow;
	int currColToCheck = srcCol;
	bool validPos;

	// We advance along the direction, advancing by deltaRow,deltaCol - in each iteration.
	// We will stop once we no longer hit an empty square or when we reached an invalid position.
	do {
		currRowToCheck += deltaRow;
		currColToCheck += deltaCol;
		validPos = validPosition(currRowToCheck, currColToCheck);
	} while (validPos && (board[currRowToCheck][currColToCheck] == EMPTY_POSITION));

	return (validPos && pieceOccupyingPosition(board, currRowToCheck, currColToCheck, pieceType));
}

/**
 * The function calculates all the possible moves from the a given position on the board by the given piece.
 * For each kind of piece, the function calls to a different helper function.
 *
 * @param game      - A Chess game
 * @param moves     - The array list where the possible moves will be added to
 * @param srcRow    - The row of the given piece's position
 * @param srcCol    - The column of the given piece's position
 * @param pieceType - The type of the given piece
 * @param sort      - Indicates if the possible moves hould be sorted
 * @param minimax   - Indicates if the moves are for the Minimax algorithm
 *
 * @return
 *		SP_CHESS_GAME_MEMORY_FAILURE   - If a memory failure occurred in the helper function that was called.
 *		SP_CHESS_GAME_INVALID_ARGUMENT - returned as a default (shouldn't get to that point in the function).
 *		Otherwise - returns the SP_CHESS_GAME_MESSAGE that got back from the relevant helper function.
 */
SP_CHESS_GAME_MESSAGE getPossiblePieceMoves(SPChessGame* game, SPArrayList* moves, int srcRow, int srcCol, char pieceType, bool sort, bool minimax) {
	switch (pieceType) {
		case WHITE_PAWN:
		case BLACK_PAWN:
			return getPossiblePawnMoves(game, moves, srcRow, srcCol, minimax);

		case WHITE_KNIGHT:
		case BLACK_KNIGHT:
			return getPossibleKnightMoves(game, moves, srcRow, srcCol);

		case WHITE_BISHOP:
		case BLACK_BISHOP:
			return getPossibleBishopOrQueenMoves(game, moves, srcRow, srcCol, sort);

		case WHITE_ROOK:
		case BLACK_ROOK:
			return getPossibleRookOrQueenMoves(game, moves, srcRow, srcCol, sort);

		case WHITE_QUEEN:
		case BLACK_QUEEN:
			// first we get the moves when moving like a bishop
			if (getPossibleBishopOrQueenMoves(game, moves, srcRow, srcCol, sort) == SP_CHESS_GAME_MEMORY_FAILURE)
				return SP_CHESS_GAME_MEMORY_FAILURE;

			// than we get the moves when moving like a rook
			return getPossibleRookOrQueenMoves(game, moves, srcRow, srcCol, sort);

		case WHITE_KING:
		case BLACK_KING:
			return getPossibleKingMoves(game, moves, srcRow, srcCol);

		default:
			return SP_CHESS_GAME_INVALID_ARGUMENT; // just to prevent logical error had to return something, shouldn't get here
	}
}

/**
 * The function calculates all the possible moves for the pawn that is in the given position.
 * Every move is added to the array list by calling the helper function addPieceMove.
 * The moves are added in a sorted way (rows from low to high as the main order, and columns from low to high as a secondary order).
 *
 * @param game    - A Chess game
 * @param moves   - The array list where the possible moves will be added to
 * @param srcRow  - The row of the given pawn's position
 * @param srcCol  - The column of the given pawn's position
 * @param minimax - Indicates if the moves are for the Minimax algorithm
 *
 * @return
 *		SP_CHESS_GAME_MEMORY_FAILURE - If a memory failure occurred in one of the calls to the helper function addPieceMove.
 *		SP_CHESS_GAME_SUCCESS        - Otherwise.
 */
SP_CHESS_GAME_MESSAGE getPossiblePawnMoves(SPChessGame* game, SPArrayList* moves, int srcRow, int srcCol, bool minimax) {
	int dstRow;

	// moving a pawn 2 rows downwards for the black player
	if (   (game->currentPlayer == BLACK_PLAYER)
		&& (addPieceMove(game, moves, srcRow, srcCol, (srcRow - 2),  srcCol, false, false) == SP_CHESS_GAME_MEMORY_FAILURE)) {

				return SP_CHESS_GAME_MEMORY_FAILURE;
	}

	// white pawn advances upwards, black pawn advances downwards
	dstRow = (game->currentPlayer == WHITE_PLAYER) ? (srcRow + 1) : (srcRow - 1);

	// eating to the left
	if (addPieceMove(game, moves, srcRow, srcCol, dstRow, (srcCol - 1), false, minimax) == SP_CHESS_GAME_MEMORY_FAILURE)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	// going forwards/backwards 1 row
	if (addPieceMove(game, moves, srcRow, srcCol, dstRow,  srcCol     , false, minimax) == SP_CHESS_GAME_MEMORY_FAILURE)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	// eating to the right
	if (addPieceMove(game, moves, srcRow, srcCol, dstRow, (srcCol + 1), false, minimax) == SP_CHESS_GAME_MEMORY_FAILURE)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	// moving a pawn 2 rows upwards for the white player
	if (   (game->currentPlayer == WHITE_PLAYER)
		&& (addPieceMove(game, moves, srcRow, srcCol, (srcRow + 2), srcCol, false, false) == SP_CHESS_GAME_MEMORY_FAILURE)) {

				return SP_CHESS_GAME_MEMORY_FAILURE;
	}

	return SP_CHESS_GAME_SUCCESS;
}

/**
 * The function calculates all the possible moves for the knight that is in the given position.
 * Every move is added to the array list by calling the helper function addPieceMove.
 * The moves are added in a sorted way (rows from low to high as the main order, and columns from low to high as a secondary order).
 *
 * @param game   - A Chess game
 * @param moves  - The array list where the possible moves will be added to
 * @param srcRow - The row of the given pawn's position
 * @param srcCol - The column of the given pawn's position
 *
 * @return
 *		SP_CHESS_GAME_MEMORY_FAILURE - If a memory failure occurred in one of the calls to the helper function addPieceMove.
 *		SP_CHESS_GAME_SUCCESS        - Otherwise.
 */
SP_CHESS_GAME_MESSAGE getPossibleKnightMoves(SPChessGame* game, SPArrayList* moves, int srcRow, int srcCol) {

	// All the 8 moves that might be possible by a knight sorted according to what is explained above

	if (addPieceMove(game, moves, srcRow, srcCol, (srcRow - 2), (srcCol - 1), false, false) == SP_CHESS_GAME_MEMORY_FAILURE)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	if (addPieceMove(game, moves, srcRow, srcCol, (srcRow - 2), (srcCol + 1), false, false) == SP_CHESS_GAME_MEMORY_FAILURE)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	if (addPieceMove(game, moves, srcRow, srcCol, (srcRow - 1), (srcCol - 2), false, false) == SP_CHESS_GAME_MEMORY_FAILURE)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	if (addPieceMove(game, moves, srcRow, srcCol, (srcRow - 1), (srcCol + 2), false, false) == SP_CHESS_GAME_MEMORY_FAILURE)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	if (addPieceMove(game, moves, srcRow, srcCol, (srcRow + 1), (srcCol - 2), false, false) == SP_CHESS_GAME_MEMORY_FAILURE)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	if (addPieceMove(game, moves, srcRow, srcCol, (srcRow + 1), (srcCol + 2), false, false) == SP_CHESS_GAME_MEMORY_FAILURE)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	if (addPieceMove(game, moves, srcRow, srcCol, (srcRow + 2), (srcCol - 1), false, false) == SP_CHESS_GAME_MEMORY_FAILURE)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	if (addPieceMove(game, moves, srcRow, srcCol, (srcRow + 2), (srcCol + 1), false, false) == SP_CHESS_GAME_MEMORY_FAILURE)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	return SP_CHESS_GAME_SUCCESS;
}

/**
 * The function calculates all the possible for the bishop/queen that is in the given position.
 * We take advantage of the fact that the queen can move also like the bishop.
 * The function calls a helper function (getDirectionalMoves) that adds all the possible moves by the relevant piece,
 * each time along a different direcional route:
 * 1) By moving diagonally downwards to the left
 * 2) By moving diagonally downwards to the right
 * 3) By moving diagonally upwards to the left
 * 4) By moving diagonally upwards to the right
 *
 * The moves are added then sorted if the function is ordered to do so (The sort order is: rows from low to high as the main order,
 * and columns from low to high as a secondary order).
 *
 * @param game   - A Chess game
 * @param moves  - The array list where the possible moves will be added to
 * @param srcRow - The row of the given pawn's position
 * @param srcCol - The column of the given pawn's position
 * @param sort   - Indicated if the moves that were calculated should get sorted
 *
 * @return
 *		SP_CHESS_GAME_MEMORY_FAILURE - If a memory failure occurred in one of the calls to the helper function addPieceMove
 *										or when calling the helper function sortMoves.
 *		SP_CHESS_GAME_SUCCESS        - Otherwise.
 */
SP_CHESS_GAME_MESSAGE getPossibleBishopOrQueenMoves(SPChessGame* game, SPArrayList* moves, int srcRow, int srcCol, bool sort) {
	// down and left
	if (getDirectionalMoves(game, moves, srcRow, srcCol, GOING_DOWNWARDS, GOING_LEFT)  == SP_CHESS_GAME_MEMORY_FAILURE)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	// down and right
	if (getDirectionalMoves(game, moves, srcRow, srcCol, GOING_DOWNWARDS, GOING_RIGHT) == SP_CHESS_GAME_MEMORY_FAILURE)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	// up and left
	if (getDirectionalMoves(game, moves, srcRow, srcCol, GOING_UPWARDS  , GOING_LEFT)  == SP_CHESS_GAME_MEMORY_FAILURE)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	// up and right
	if (getDirectionalMoves(game, moves, srcRow, srcCol, GOING_UPWARDS  , GOING_RIGHT) == SP_CHESS_GAME_MEMORY_FAILURE)
		return SP_CHESS_GAME_MEMORY_FAILURE;


	if (sort) {
		int movesAmount = spArrayListSize(moves);

		// sort the moves if there are enough moves so sorting will be relevant
		if (movesAmount >= MIN_MOVES_AMOUNT_TO_SORT)
			return sortMoves(moves, movesAmount);
	}

	return SP_CHESS_GAME_SUCCESS;
}

/**
 * The function calculates all the possible for the rook/queen that is in the given position.
 * We take advantage of the fact that the queen can move also like the rook.
 * The function calls a helper function (getDirectionalMoves) that adds all the possible moves by the relevant piece,
 * each time along a different direcional route:
 * 1) By moving vertically downwards
 * 2) By moving horizontally to the left
 * 3) By moving horizontally to the right
 * 4) By moving vertically upwards
 *
 * If the given piece is a rook we also call the helper function addPieceMove to try to add a castle move.
 *
 * The moves are added then sorted if the function is ordered to do so (The sort order is: rows from low to high as the main order,
 * and columns from low to high as a secondary order). The castle move will appear last.
 *
 * @param game   - A Chess game
 * @param moves  - The array list where the possible moves will be added to
 * @param srcRow - The row of the given pawn's position
 * @param srcCol - The column of the given pawn's position
 * @param sort   - Indicated if the moves that were calculated should get sorted
 *
 * @return
 *		SP_CHESS_GAME_MEMORY_FAILURE - If a memory failure occurred in one of the calls to the helper function addPieceMove
 *										or when calling the helper function sortMoves.
 *		SP_CHESS_GAME_SUCCESS        - Otherwise.
 */
SP_CHESS_GAME_MESSAGE getPossibleRookOrQueenMoves(SPChessGame* game, SPArrayList* moves, int srcRow, int srcCol, bool sort) {

	// downwards
	if (getDirectionalMoves(game, moves, srcRow, srcCol, GOING_DOWNWARDS, SAME_COL) == SP_CHESS_GAME_MEMORY_FAILURE)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	// to the left
	if (getDirectionalMoves(game, moves, srcRow, srcCol, SAME_ROW       , GOING_LEFT) == SP_CHESS_GAME_MEMORY_FAILURE)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	// to the right
	if (getDirectionalMoves(game, moves, srcRow, srcCol, SAME_ROW       , GOING_RIGHT) == SP_CHESS_GAME_MEMORY_FAILURE)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	// upwards
	if (getDirectionalMoves(game, moves, srcRow, srcCol, GOING_UPWARDS  , SAME_COL) == SP_CHESS_GAME_MEMORY_FAILURE)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	if (sort) {
		int movesAmount = spArrayListSize(moves);

		// sort the moves if there are enough moves so sorting will be relevant
		if ((movesAmount >= MIN_MOVES_AMOUNT_TO_SORT) && (sortMoves(moves, movesAmount) != SP_CHESS_GAME_SUCCESS))
			return SP_CHESS_GAME_MEMORY_FAILURE;
	}

	/* if possible, castle move appears last (we send DUMMY_COORDINATE as the destination position's row and column because we
	   don't use the fields dstRow and dstCol of the move structure when setting a castle move) */
	char piece = game->board[srcRow][srcCol];
	if (   ((piece == WHITE_ROOK) || (piece == BLACK_ROOK))
		&& (addPieceMove(game, moves, srcRow, srcCol, DUMMY_COORDINATE, DUMMY_COORDINATE, true, false) == SP_CHESS_GAME_MEMORY_FAILURE)) {

				return SP_CHESS_GAME_MEMORY_FAILURE;
	}

	return SP_CHESS_GAME_SUCCESS;
}

/**
 * The function calculates all the possible for the given piece along the given direction.
 * We do this checking by starting from the given position and going sqaure-by-square on the given game board,
 * according to the given direction: the move is added at the time we get to a new position, and the itereation
 * that represnetd the square-by-square walk stops when it either reached an invalid position or when it encountered
 * a non empty position. The non empty position is added as destination position to move to only if the piece in that
 * position belongs to the rival (otherwise it would be illegal for the piece to move there).
 *
 * @param game     - A Chess game
 * @param moves    - The array list where the possible moves will be added to
 * @param srcRow   - The row of the position where the given piece is currently at
 * @param srcCol   - The column of the position where the given piece is currently at
 * @param deltaRow - The movement in terms of row that should be made when going to the next position sqare-by-square:
 * 					  Either one row up, one row down or staying at the same row
 * @param deltaCol - The movement in terms of column that should be made when going to the next position sqare-by-square:
 * 					   Either one column up, one column down or staying at the same column
 *
 * @return
 * 		SP_CHESS_GAME_MEMORY_FAILURE - If a memory failure occurred in one of the calls to the helper function addPieceMove.
 *      SP_CHESS_GAME_SUCCESS        - Otherwise.
 */
SP_CHESS_GAME_MESSAGE getDirectionalMoves(SPChessGame* game, SPArrayList* moves, int srcRow, int srcCol, int deltaRow, int deltaCol) {
	// the first position to check
	int currRowToCheck = (srcRow + deltaRow);
	int currColToCheck = (srcCol + deltaCol);

	char dstPiece = EMPTY_POSITION; // default value to ensure it is initialized

	bool keepGoing = validPosition(currRowToCheck, currColToCheck);

	/* We advance along the direction, advancing by deltaRow rows and deltaCol columns in each iteration.
	   We will once we no longer hit an empty square or when we reache an invalid position */
	while (keepGoing) {
		dstPiece = game->board[currRowToCheck][currColToCheck];

		if (dstPiece != EMPTY_POSITION)
			break;

		if (addPieceMove(game, moves, srcRow, srcCol, currRowToCheck, currColToCheck, false, false) == SP_CHESS_GAME_MEMORY_FAILURE)
			return SP_CHESS_GAME_MEMORY_FAILURE;

		currRowToCheck += deltaRow;
		currColToCheck += deltaCol;

		keepGoing = validPosition(currRowToCheck, currColToCheck);
	}

	// If the reason we stopped iterating was we encountered an enemy piece, we might get an additional move: an "eating" move.
	if ((dstPiece != EMPTY_POSITION) && rivalPiece(game->currentPlayer, dstPiece))
		return addPieceMove(game, moves, srcRow, srcCol, currRowToCheck, currColToCheck, false, false);

	return SP_CHESS_GAME_SUCCESS;
}

/**
 * The function sorts the given array list of moves. The sorting method is the famous "Bubble Sort" according
 * to the move's destination position, while sort order is:
 * Rows from low to high as the main order, and columns from low to high as a secondary order.
 *
 * @param moves       - The array list where the possible moves will be added to
 * @param movesAmount - The number of moves to sort in the array list
 *
 * @return
 * 		SP_CHESS_GAME_MEMORY_FAILURE - If a memory failure occurred furing the execution of the function.
 *      SP_CHESS_GAME_SUCCESS        - Otherwise.
 */
SP_CHESS_GAME_MESSAGE sortMoves(SPArrayList* moves, int movesAmount) {
	Move *moveA, *moveB;
	bool swaped;

	for (int i = 0; i < (movesAmount-1); i++) {
		swaped = false;

		for (int j = 0; j < (movesAmount-1); j++) {
			moveA = spMoveCopy(spArrayListGetAt(moves, j));
			if (moveA == NULL)
				return SP_CHESS_GAME_MEMORY_FAILURE;

			moveB = spArrayListGetAt(moves, j+1);

			if (   (moveA->dstRow > moveB->dstRow)
			    || ((moveA->dstRow == moveB->dstRow) && (moveA->dstCol > moveB->dstCol))) {

						spArrayListRemoveAt(moves, j);
						spArrayListAddAt(moves, moveA, j+1);
						swaped = true;
			}
			free(moveA);
		}

		if (!swaped)
			return SP_CHESS_GAME_SUCCESS;
	}

	return SP_CHESS_GAME_SUCCESS;
}

/**
 * The function calculates all the possible for the king that is in the given position.
 * Every move is added to the array list by calling the helper function addPieceMove.
 * The moves are added in a sorted way (rows from low to high as the main order,
 * and columns from low to high as a secondary order), and castle moves (if there are possible ones) are added last.
 *
 * @param game   - A Chess game
 * @param moves  - The array list where the possible moves will be added to
 * @param srcRow - The row of the given pawn's position
 * @param srcCol - The column of the given pawn's position
 *
 * @return
 *		SP_CHESS_GAME_MEMORY_FAILURE - If a memory failure occurred in one of the calls to the helper function addPieceMove.
 *		SP_CHESS_GAME_SUCCESS        - Otherwise.
 */
SP_CHESS_GAME_MESSAGE getPossibleKingMoves(SPChessGame* game, SPArrayList* moves, int srcRow, int srcCol) {

	// All the 8 moves that might be possible by a king sorted according to what is explained above

	if (addPieceMove(game, moves, srcRow, srcCol, (srcRow - 1), (srcCol - 1), false, false) == SP_CHESS_GAME_MEMORY_FAILURE)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	if (addPieceMove(game, moves, srcRow, srcCol, (srcRow - 1),  srcCol     , false, false) == SP_CHESS_GAME_MEMORY_FAILURE)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	if (addPieceMove(game, moves, srcRow, srcCol, (srcRow - 1), (srcCol + 1), false, false) == SP_CHESS_GAME_MEMORY_FAILURE)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	if (addPieceMove(game, moves, srcRow, srcCol,  srcRow     , (srcCol - 1), false, false) == SP_CHESS_GAME_MEMORY_FAILURE)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	if (addPieceMove(game, moves, srcRow, srcCol,  srcRow     , (srcCol + 1), false, false) == SP_CHESS_GAME_MEMORY_FAILURE)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	if (addPieceMove(game, moves, srcRow, srcCol, (srcRow + 1), (srcCol - 1), false, false) == SP_CHESS_GAME_MEMORY_FAILURE)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	if (addPieceMove(game, moves, srcRow, srcCol, (srcRow + 1),  srcCol     , false, false) == SP_CHESS_GAME_MEMORY_FAILURE)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	if (addPieceMove(game, moves, srcRow, srcCol, (srcRow + 1), (srcCol + 1), false, false) == SP_CHESS_GAME_MEMORY_FAILURE)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	// king castle moves
	if (game->currentPlayer == WHITE_PLAYER)
		return getPossibleKingCastleMoves(game, moves, WHITE_ROOK, WHITE_FIRST_ROW);
	else
		return getPossibleKingCastleMoves(game, moves, BLACK_ROOK, BLACK_FIRST_ROW);
}

/**
 * The function calculates the possible castle moves that the given king can make with its rooks (first with the left rook,
 * then with the right rook).
 * Every possible castle move is added to the array list by calling the helper function addPieceMove.
 *
 * @param game           - A Chess game
 * @param moves          - The array list where the possible moves will be added to
 * @param pieceType      - The type of king that we try to find for it possible castle moves
 * @param playerFirstRow - The "first rank" on board of the player that plays a move (helps to figure out if the king is at its original position)
 *
 * @return
 *		SP_CHESS_GAME_MEMORY_FAILURE - If a memory failure occurred in one of the calls to the helper function addPieceMove.
 *		SP_CHESS_GAME_SUCCESS        - Otherwise.
 */
SP_CHESS_GAME_MESSAGE getPossibleKingCastleMoves(SPChessGame* game, SPArrayList* moves, int pieceType, int playerFirstRow) {
	/* first left castle (we send DUMMY_COORDINATE as the destination position's row and column because we
	   don't use the fields dstRow and dstCol of the move structure when setting a castle move) */
	if (   (game->board[playerFirstRow][LEFT_ROOK_COL] == pieceType)
		&& (addPieceMove(game, moves, playerFirstRow, LEFT_ROOK_COL , DUMMY_COORDINATE, DUMMY_COORDINATE, true, false) == SP_CHESS_GAME_MEMORY_FAILURE)) {

				return SP_CHESS_GAME_MEMORY_FAILURE;
	}

	/* then right castle (we send DUMMY_COORDINATE as the destination position's row and column because we
	   don't use the fields dstRow and dstCol of the move structure when setting a castle move) */
	if (   (game->board[playerFirstRow][RIGHT_ROOK_COL] == pieceType)
		&& (addPieceMove(game, moves, playerFirstRow, RIGHT_ROOK_COL, DUMMY_COORDINATE, DUMMY_COORDINATE, true, false) == SP_CHESS_GAME_MEMORY_FAILURE)) {

				return SP_CHESS_GAME_MEMORY_FAILURE;
	}

	return SP_CHESS_GAME_SUCCESS;
}

/**
 * The function creates a move structure according to the parameters it receives, and after verifying the move is
 * legal - adds it to the given array list.
 * If the move would be a pawn promotion move (that was received from the Minimax algorithm), the function calls to the helper function
 * addPawnPromotionsMoves in order to add all the possible promotions as possible moves.
 *
 * @param game    - A Chess game
 * @param moves   - The array list where the possible moves will be added to
 * @param srcRow  - The row of the position where the given piece is going to move from
 * @param srcCol  - The column of the position where the given piece is going to move from
 * @param dstRow  - The row of the position where the given piece is going to move to
 * @param dstCol  - The column of the position where the given piece is going to move to
 * @param castle  - indicates if the move that we try to add is a castle move
 * @param minimax - Indicates if the move is for the Minimax algorithm
 *
 * @return
 *		SP_CHESS_GAME_MEMORY_FAILURE - If a memory failure occurred in one of the calls to the helper function addPieceMove.
 *		SP_CHESS_GAME_SUCCESS        - Otherwise (even if the move was not added - not adding the move is ok if it not because of memory failure.
 */
SP_CHESS_GAME_MESSAGE addPieceMove(SPChessGame* game, SPArrayList* moves, int srcRow, int srcCol, int dstRow, int dstCol, bool castle, bool minimax) {
	if (!castle && !validPosition(dstRow, dstCol))
		return SP_CHESS_GAME_SUCCESS;

	Move* move = spCreateMove();
	if (move == NULL)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	move->srcRow   = srcRow;
	move->srcCol   = srcCol;
	move->srcPiece = game->board[srcRow][srcCol];

	if (castle) {
		if (!legalCastle(game, move)) {
			free(move);
			return SP_CHESS_GAME_SUCCESS;
		}

		move->castleMove = true;
	}
	else {
		move->dstRow   = dstRow;
		move->dstCol   = dstCol;

		if (!legalRegularMove(game, move)) {
			free(move);
			return SP_CHESS_GAME_SUCCESS;
		}

		// adding all the possible promotions as possible moves for the Minimax algorithm
		if (minimax && (move->pawnPromotion)) {
			SP_CHESS_GAME_MESSAGE message = addPawnPromotionsMoves(moves, move, game->currentPlayer);
			free(move);
			return message;
		}
	}

	spArrayListAddLast(moves, move);
	free(move);

	return SP_CHESS_GAME_SUCCESS;
}

/**
 * The function adds all the possible promotions as possible moves to the given array list.
 * According to the current player to plays the move, we choose the types of queen/rook/bishop/knight that the pawn will be promoted to.
 *
 * @param moves      - The array list where the possible moves will be added to
 * @param move       - The move details that each pawn promtion move will get its details from
 * @param currPlayer - The current player to make a move
 *
 * @return
 *		SP_CHESS_GAME_MEMORY_FAILURE - If a memory failure occurred in one of the calls to the helper function addPieceMove.
 *		SP_CHESS_GAME_SUCCESS        - Otherwise (even if the move was not added - not adding the move is ok if it not because of memory failure).
 */
SP_CHESS_GAME_MESSAGE addPawnPromotionsMoves(SPArrayList* moves, Move* move, char currPlayer) {
	char queenType  = (currPlayer == WHITE_PLAYER) ? WHITE_QUEEN  : BLACK_QUEEN;
	char rookType   = (currPlayer == WHITE_PLAYER) ? WHITE_ROOK   : BLACK_ROOK;
	char bishopType = (currPlayer == WHITE_PLAYER) ? WHITE_BISHOP : BLACK_BISHOP;
	char knightType = (currPlayer == WHITE_PLAYER) ? WHITE_KNIGHT : BLACK_KNIGHT;

	// pawn promotion to queen
	Move* queenMove = spMoveCopy(move);
	if (queenMove == NULL)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	queenMove->srcPiece = queenType;
	spArrayListAddLast(moves, queenMove);
	free(queenMove);


	// pawn promotion to rook
	Move* rookMove = spMoveCopy(move);
	if (rookMove == NULL)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	rookMove->srcPiece = rookType;
	spArrayListAddLast(moves, rookMove);
	free(rookMove);


	// pawn promotion to bishop
	Move* bishopMove = spMoveCopy(move);
	if (bishopMove == NULL)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	bishopMove->srcPiece = bishopType;
	spArrayListAddLast(moves, bishopMove);
	free(bishopMove);


	// pawn promotion to knight
	Move* knightMove = spMoveCopy(move);
	if (knightMove == NULL)
		return SP_CHESS_GAME_MEMORY_FAILURE;

	knightMove->srcPiece = knightType;
	spArrayListAddLast(moves, knightMove);
	free(knightMove);

	return SP_CHESS_GAME_SUCCESS;
}

/**
 * The function checks if the given piece belongs to the player that is its turn to play.
 *
 * @param currPlayer - The player that it is its turn to play
 * @param piece      - The piece that its type is checked
 *
 * @return
 *		True  - If the given piece belongs the player that it is its turn to play.
 *		False - Otherwise.
 */
bool currentPlayerPiece(int currPlayer, char piece) {
	// in case it is the white player's turn
	bool whitePlayerWhitePiece = ((currPlayer == WHITE_PLAYER) && ((piece == WHITE_PAWN)   ||
																   (piece == WHITE_KNIGHT) ||
																   (piece == WHITE_BISHOP) ||
																   (piece == WHITE_ROOK)   ||
																   (piece == WHITE_QUEEN)  ||
																   (piece == WHITE_KING)));

	// in case it is the black player's turn
	bool blackPlayerBlackPiece = ((currPlayer == BLACK_PLAYER) && ((piece == BLACK_PAWN)   ||
																   (piece == BLACK_KNIGHT) ||
																   (piece == BLACK_BISHOP) ||
																   (piece == BLACK_ROOK)   ||
																   (piece == BLACK_QUEEN)  ||
																   (piece == BLACK_KING)));

	return (whitePlayerWhitePiece || blackPlayerBlackPiece);
}

/**
 * The function checks if the given piece belongs to the rival player (i.e. the player that it is not its turn to play).
 *
 * @param currPlayer - The player that it is its turn to play
 * @param piece      - The piece that its type is checked
 *
 * @return
 *		True  - If the given piece belongs the rival player.
 *		False - Otherwise.
 */
bool rivalPiece(int currPlayer, char piece) {
	bool whitePlayerBlackPiece = ((currPlayer == WHITE_PLAYER) && ((piece == BLACK_PAWN)   ||
																   (piece == BLACK_KNIGHT) ||
															       (piece == BLACK_BISHOP) ||
																   (piece == BLACK_ROOK)   ||
																   (piece == BLACK_QUEEN)  ||
																   (piece == BLACK_KING)));

	bool blackPlayerWhitePiece = ((currPlayer == BLACK_PLAYER) && ((piece == WHITE_PAWN)   ||
																   (piece == WHITE_KNIGHT) ||
																   (piece == WHITE_BISHOP) ||
																   (piece == WHITE_ROOK)   ||
																   (piece == WHITE_QUEEN)  ||
																   (piece == WHITE_KING)));

	return (whitePlayerBlackPiece || blackPlayerWhitePiece);
}

/**
 * The function checks if the given position is occupied by the given piece.
 * Before checking if the piece ocuupies the position we first check if it valid by calling the helper function validPosition.
 *
 * @param board - The current Chess game board
 * @param row   - The row of the given position
 * @param col   - The column of the given position
 * @param piece - The piece that we check if it occupies the given position
 *
 * @return
 *		True  - If given piece is occupying the given position.
 *		False - Otherwise.
 */
bool pieceOccupyingPosition(char board[][BOARD_LENGTH], int row, int col, char piece) {
	return ((validPosition(row, col)) && (board[row][col] == piece));
}

/**
 * The function checks if the given position is valid (i.e. located on the Chess board).
 *
 * @param row - The row of the given position
 * @param col - The column of the given position
 *
 * @return
 *		True  - If the position is valid.
 *		False - Otherwise.
 */
bool validPosition(int row, int col) {
	return ((row >= FIRST_ROW_AND_COL) && (row < BOARD_LENGTH) && (col >= FIRST_ROW_AND_COL) && (col < BOARD_LENGTH));
}

/**
 * The function nullifies the white army counting (i.e. the counter for number of pieces of each type) and the black army.
 *
 * @param game - A Chess Game
 */
void nullifyArmies(SPChessGame* game) {
	if (game == NULL)
		return;

	Army* whiteArmy = game->whiteArmy;
	Army* blackArmy = game->blackArmy;

	whiteArmy->numOfPawns   = 0;
	whiteArmy->numOfKnights = 0;
	whiteArmy->numOfBishops = 0;
	whiteArmy->numOfRooks   = 0;
	whiteArmy->numOfQueens  = 0;

	blackArmy->numOfPawns   = 0;
	blackArmy->numOfKnights = 0;
	blackArmy->numOfBishops = 0;
	blackArmy->numOfRooks   = 0;
	blackArmy->numOfQueens  = 0;
}
