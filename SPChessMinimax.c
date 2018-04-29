#include "SPChessMinimax.h"

/**
 * The function initiates the Minimax algorithm for choosing the best next Chess move for the current
 * player according to a given Chess game board.
 * The Minimax algorithm will use a naive scoring function and the "Alpha-beta Pruning" technique.
 * The algorithm is initiated by calling to the helper function alphaBetaPruning.
 * The current game state is not changed by this alogrithm (a copy is created each time a new node is created),
 * as well as the history of previous moves (actually when the game is copied, the history is not, and that's
 * because the algorithm doesn't undo moves - so no need to copy the history).
 *
 * @param game     - The current Chess game
 * @param maxDepth - The difficulty level of the game and the maximum depth of the Minimax tree
 *
 * @precondition - game is not over (i.e. game->status is not WHITE_PLAYER_WINS nor BLACK_PLAYER_WINS nor TIED_GAME)
 *
 * @return
 * 		NULL if either game is NULL or maxDepth <= 0 or an allocation memory occurred (denoted when the value that is returned
 * 		 from the helper function alphaBetaPruning equals INT_MIN).
 * 		On success, returns the move that was chosen for the player by the Minimax algorithm.
 */
Move* spChessMinimaxMove(SPChessGame* game, int maxDepth) {
	if ((game == NULL) || (maxDepth <= 0))
		return NULL;

	Move* minimaxMove = spCreateMove();
	if (minimaxMove == NULL)
		return NULL;

	bool memoryOccured = (alphaBetaPruning(game, minimaxMove, ROOT_DEPTH, maxDepth, INT_MIN, INT_MAX, game->currentPlayer) == INT_MIN);

	// if memory occured we free the move that was created and nullify the pointer
	if (memoryOccured) {
		free(minimaxMove);
		minimaxMove = NULL;
	}

	return minimaxMove;
}

/**
 * The function runs the MiniMax algorithm (using "Alpha-beta Pruning" technique), by building recursively and dynamically the Minimax
 * tree for reaching the best move to pick according the root node's game board for the player which is it's turn to play.
 * To create the relevant nodes, the function scans the game board bottom-up as main scan and left-to-right as secondary scan.
 * For each relevant piece (that belongs to the current player to play) the fuction creates all of it's possible moves
 * by calling the helper function getPossiblePieceMoves that sets these move in a given array list (the sorting is bottom-up
 * as main sort and left-to-right as secondary sort).
 * The "Alpha-beta Pruning" technique allows the algorithm avoid building nodes that can be seen as irrelevant and thus save time and memory.
 * The alpha and beta values might be updated at each depth of the recursion, while the move that will be returned as the best move to
 * choose is updated only at depth 0 (i.e. the root's depth).
 * The recursion ends when the Minimax tree gets to a leaf, which happens when either:
 * 1) The board represents a game that was over.
 * 2) The recursion reached its maximum depth according to the difficulty of the game.
 *
 * @param currentGame      - The current Chess game node
 * @param minimaxMove      - The move to be updated as the move that the original player that is maximing should select
 * @param currDepth        - The current depth of the Minimax tree
 * @param maxDepth         - The difficulty level of the game and the maximum depth for the Minimax tree
 * @param alpha            - The maximum lower bound of possible scoring function values seen that is received from the node's parent
 * @param beta             - The minimum upper bound of possible scoring function values seen that is received from the node's parent
 * @param maximizingPlayer - The player which is its turn to play according to the root node of the tree
 *
 * @return
 * 		When a memory error occurs:
 * 		 1) If the current depth of the tree is greater than 0, INT_MAX is returned in case of a maxmizing node
 * 		     and INT_MIN is returned in case of a minimizing node.
 * 		 2) If the current depth of the tree is 0 (the root's depth), INT_MIN is returned.
 *
 * 		On success:
 * 		 1) If the recursion reached a leaf node, returns the scoring function value.
 * 		 2) If the current depth of the tree is greater than 0 but the node is not a leaf node:
 * 		     the alpha value is returned in case of a maxmizing node, and the beta value is returned
 * 		     in case of a minimizing node.
 * 		 3) If the recursion got back to the root node, INT_MAX is returned (denotes for the chessMinimaxMove
 * 		     that a move was chosen successfully).
 */
int alphaBetaPruning(SPChessGame* currentGame, Move* minimaxMove, int currDepth, int maxDepth, int alpha, int beta, int maximizingPlayer) {
	if (currDepth > ROOT_DEPTH) {
		char currStatus = currentGame->status;
		if ((currStatus == WHITE_PLAYER_WINS)  || (currStatus == BLACK_PLAYER_WINS) || (currStatus == TIED_GAME) || (currDepth == maxDepth))
			return leafNodeResult(currentGame, maximizingPlayer);
	}

	bool choseMove = false; // denotes if so far at least one move was chosen as the move to be picked by the Minimax algorithm
	bool maximize = ((currDepth % 2) == 0);

	SPArrayList* pieceMoves = spArrayListCreate(MAX_MOVES_FOR_PIECE);
	if (pieceMoves == NULL)
		return freeMemory(pieceMoves, currDepth, maximize);

	int moveNum, numOfMoves, subtreeValue;

	for (int row = 0; ((alpha < beta) && (row < BOARD_LENGTH)); row++) {
		for (int col = 0; ((alpha < beta) && (col < BOARD_LENGTH)); col++) {
			char pieceType = currentGame->board[row][col];
			if (!currentPlayerPiece(currentGame->currentPlayer, pieceType))
				continue;

			if (getPossiblePieceMoves(currentGame, pieceMoves, row, col, pieceType, true, true) != SP_CHESS_GAME_SUCCESS)
				return freeMemory(pieceMoves, currDepth, maximize);

			numOfMoves = spArrayListSize(pieceMoves);

			moveNum = 0;
			while ((alpha < beta) && (moveNum < numOfMoves)) {
				SPChessGame* descendantGame = spChessGameCopy(currentGame);
				Move* currMove = spArrayListGetAt(pieceMoves, moveNum);

				if (   (descendantGame == NULL)
					|| (currMove == NULL)
					|| (spChessGameSetMove(descendantGame, currMove, true, true) != SP_CHESS_GAME_SUCCESS)) {

							spChessGameDestroy(descendantGame);
							return freeMemory(pieceMoves, currDepth, maximize);
				}

				// the recursive call
				subtreeValue = alphaBetaPruning(descendantGame, minimaxMove, currDepth + 1, maxDepth, alpha, beta, maximizingPlayer);

				// calls the function that is in charge of updating the alpha/beta values
				updateBoundsAndMove(currMove, minimaxMove, &(alpha), &(beta), maximize, subtreeValue, currDepth, &(choseMove));

				spChessGameDestroy(descendantGame);
				moveNum++;
			}
			spArrayListClear(pieceMoves);
		}
	}
	spArrayListDestroy(pieceMoves);

	if (currDepth == ROOT_DEPTH)
		return INT_MAX;

	return ((maximize) ? alpha : beta);
}

/**
 * The function updates (if needed) the alpha value or the beta value (respectively to what kind of node called to
 * the function - a maximizing node or a minimizing node).
 * In addition, if needed, updates the Minimax move (i.e. the move to be chosen by the Minimax algorithm) to be the
 * move that reached the highest scoring function so far.
 * That kind of updating of the Minimax move happens only when the function was called by the root node.
 *
 * @param currMove     - The move that created the current node (i.e. the node that called the function)
 * @param minimaxMove  - The move that is currently the chosen move by the Minimax algorithm
 * @param alpha        - The pointer to the current alpha value of the current node (i.e. the node that called the function)
 * @param beta         - The pointer to the current beta value of the current node (i.e. the node that called the function)
 * @param maximize     - Indicates if the the function was called from a maximizing node or from a minimizing node
 * @param subtreeValue - The value that was returned to the current node (i.e. the node that called the function)
 *                        by the current subtree that was created in the Minimax tree
 * @param currDepth    - The current depth of the Minimax tree
 * @param choseMove    - The pointer to the indicator that denotes if so far at least one move was chosen as the move to be
 * 						  picked by the Minimax algorithm
 *
 * @return
 * 		INT_MAX - in case of a maxmizing node.
 * 		INT_MIN - in case of a minimizing node.
 */
void updateBoundsAndMove(Move* currMove, Move* minimaxMove, int* alpha, int* beta, bool maximize, int subtreeValue, int currDepth, bool* choseMove) {
	if (   (maximize)
	    && ((subtreeValue > (*alpha)) || ((currDepth == ROOT_DEPTH) && !(*choseMove)))) {

				*alpha = subtreeValue;

				if (currDepth == ROOT_DEPTH) {
					spMoveCopyData(currMove, minimaxMove);
					*choseMove = true;
				}
	}
	else if (!maximize && (subtreeValue < (*beta)))
				*beta = subtreeValue;
}

/**
 * The function calculates the value for the leaf node (of the Minimax tree) that was received as a parameter.
 * If the game ended then the value is returned according to the game winner (or according to a tied game),
 * but if the leaf represents the fact that the maximum depth of the tree was reached (without that the being ended) -
 * then there will be a call to the scoring function.
 *
 * @param game             - A Chess game
 * @param maximizingPlayer - The player which is its turn to play according to the root node of the tree.
 *
 * @return
 * 		INT_MAX - If either the game ended with the white player as a winner and the white player is the maximizing player,
 * 		           or if the game ended with the white player as a winner and the white player is the maximizing player.
 * 		INT_MIN - if the game ended and the winner is player B.
 *
 * 		Otherwise, returns the result that comes back from the scoring function.
 */
int leafNodeResult(SPChessGame* game, int maximizingPlayer) {
	switch (game->status) {
		case WHITE_PLAYER_WINS:
			return ((maximizingPlayer == WHITE_PLAYER) ? INT_MAX : INT_MIN);
		case BLACK_PLAYER_WINS:
			return ((maximizingPlayer == BLACK_PLAYER) ? INT_MAX : INT_MIN);
		case TIED_GAME:
			return TIE_SCORE;
		default:
			return scoringFunction(game, maximizingPlayer);
	}
}

/**
 * The function calculates the scoring for the maximizing player.
 * The function uses the values of each type of piece (defined in macros) to calculate the weighted values of the white/black armies.
 * At the end, the function subtract from the weighted value of the maximizing player's army the weighted value of the rival army.
 * Note that no need to include in the calculation the king because:
 * 1) On one hand, a player cannot have more than 1 king.
 * 2) On the other hand, we cannot get to a game board where one of the player doesn't have it's king (if there is a "checkmate"
 *     the game finishes so no "eating" of the king is actually possible).
 *
 * @param game             - A Chess game
 * @param maximizingPlayer - The player which is its turn to play according to the root node of the tree.
 *
 * @return
 * 		The weighted value of the maximizing player's army minus the the weighted value of its rival's army.
 */
int scoringFunction(SPChessGame* game, int maximizingPlayer) {
	Army* whiteArmy = game->whiteArmy;
	Army* blackArmy = game->blackArmy;

	int whiteScore = (whiteArmy->numOfPawns * PAWN_SCORE) + (whiteArmy->numOfKnights * KNIGHT_SCORE) + (whiteArmy->numOfBishops * BISHOP_SCORE) +
					 (whiteArmy->numOfRooks * ROOK_SCORE) + (whiteArmy->numOfQueens  * QUEEN_SCORE);
	int blackScore = (blackArmy->numOfPawns * PAWN_SCORE) + (blackArmy->numOfKnights * KNIGHT_SCORE) + (blackArmy->numOfBishops * BISHOP_SCORE) +
			         (blackArmy->numOfRooks * ROOK_SCORE) + (blackArmy->numOfQueens  * QUEEN_SCORE);

	if (maximizingPlayer == WHITE_PLAYER) // White turn at the root node
		return (whiteScore - blackScore);
	else 	                              // Black turn at the root node
		return (blackScore - whiteScore);
}

/**
 * The function frees the memory allocation of the given array list (in case it is not NULL).
 *
 * @param pieceMoves - The array list that its memory should get freed
 * @param currDepth  - The current depth of the Minimax tree
 * @param maximize   - Indicates if the the function was called from a maximizing node or from a minimizing node
 *
 * @return
 * 		INT_MAX - in case of a maxmizing node.
 * 		INT_MIN - in case of a minimizing node.
 */
int freeMemory(SPArrayList* pieceMoves, int currDepth, bool maximize) {
	if (pieceMoves != NULL)
		spArrayListDestroy(pieceMoves);

	if (currDepth == ROOT_DEPTH)
		return INT_MIN;

	return ((maximize) ? INT_MIN : INT_MAX);
}
