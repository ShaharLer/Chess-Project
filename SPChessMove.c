#include "SPChessMove.h"

/**
 * Creates a Move object with default values.
 *
 * @return the new Move object.
 * If an allocation error occurs, NULL is returned.
 */
Move* spCreateMove(){
	Move* move = (Move*) malloc(sizeof(Move));
	if (move == NULL) // allocation error
		return NULL;

	move->dstPieceCaptured = false;
	move->pawnPromotion = false;
	move->threatenedAfterMove = false;
	move->castleMove = false;

	return move;
}

/**
 *	Creates an exact copy of the src move object. Data in the new copy will
 *	be the same as they appeared in the source move.
 *	@param src - the source move.
 *	@return
 *	NULL if either an allocation error occurs or src == NULL.
 *	A new copy of the source move, otherwise.
 */
Move* spMoveCopy(Move* src) {
	if (src == NULL)
		return NULL;
	Move* dst = (Move*)malloc(sizeof(Move));
	if (dst == NULL)
		return NULL;

	spMoveCopyData(src, dst);

	return dst;
}

/**
 *	Copies the data of the src move object to the dst move object.
 *	@param src - the source move object pointer
 *		   dst - the destination move object pointer
 *
 */
void spMoveCopyData(Move* src, Move* dst) {
	dst->srcRow              = src->srcRow;
	dst->srcCol              = src->srcCol;
	dst->dstRow              = src->dstRow;
	dst->dstCol              = src->dstCol;
	dst->srcPiece            = src->srcPiece;
	dst->dstPiece            = src->dstPiece;
	dst->whiteLeftCastle     = src->whiteLeftCastle;
	dst->whiteRightCastle    = src->whiteRightCastle;
	dst->blackLeftCastle     = src->blackLeftCastle;
	dst->blackRightCastle    = src->blackRightCastle;
	dst->castleMove          = src->castleMove;
	dst->pawnPromotion       = src->pawnPromotion;
	dst->dstPieceCaptured    = src->dstPieceCaptured;
	dst->threatenedAfterMove = src->threatenedAfterMove;
}
