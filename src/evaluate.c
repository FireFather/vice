// evaluate.c

#include "stdio.h"
#include "defs.h"
#include "nnue_eval.h"

int material_draw(const s_board* pos)
{
	ASSERT(check_board(pos))

	if (!pos->pce_num[wR] && !pos->pce_num[bR] && !pos->pce_num[wQ] && !pos->pce_num[bQ])
	{
		if (!pos->pce_num[bB] && !pos->pce_num[wB])
		{
			if (pos->pce_num[wN] < 3 && pos->pce_num[bN] < 3) { return true; }
		}
		else if (!pos->pce_num[wN] && !pos->pce_num[bN])
		{
			if (abs(pos->pce_num[wB] - pos->pce_num[bB]) < 2) { return true; }
		}
		else if ((pos->pce_num[wN] < 3 && !pos->pce_num[wB]) || (pos->pce_num[wB] == 1 && !pos->pce_num[wN]))
		{
			if ((pos->pce_num[bN] < 3 && !pos->pce_num[bB]) || (pos->pce_num[bB] == 1 && !pos->pce_num[bN]))
			{
				return true;
			}
		}
	}
	else if (!pos->pce_num[wQ] && !pos->pce_num[bQ])
	{
		if (pos->pce_num[wR] == 1 && pos->pce_num[bR] == 1)
		{
			if (pos->pce_num[wN] + pos->pce_num[wB] < 2 && pos->pce_num[bN] + pos->pce_num[bB] < 2) { return true; }
		}
		else if (pos->pce_num[wR] == 1 && !pos->pce_num[bR])
		{
			if (pos->pce_num[wN] + pos->pce_num[wB] == 0 && (pos->pce_num[bN] + pos->pce_num[bB] == 1 || pos->pce_num
				[bN] + pos->pce_num[bB] == 2)) { return true; }
		}
		else if (pos->pce_num[bR] == 1 && !pos->pce_num[wR])
		{
			if (pos->pce_num[bN] + pos->pce_num[bB] == 0 && (pos->pce_num[wN] + pos->pce_num[wB] == 1 || pos->pce_num
				[wN] + pos->pce_num[wB] == 2)) { return true; }
		}
	}
	return false;
}

#define ENDGAME_MAT (1 * PieceVal[wR] + 2 * PieceVal[wN] + 2 * PieceVal[wP] + PieceVal[wK])

// Stockfish NNUE piece encoding
int nnue_pieces[13] = {0, 6, 5, 4, 3, 2, 1, 12, 11, 10, 9, 8, 7};

int eval_position(const s_board* pos)
{
	// NNUE probe arrays
	int pieces[33];
	int squares[33];

	// NNUE probe arrays index
	int index = 2;

	// loop over the pieces
	for (int piece = 1; piece < 13; piece++)
	{
		// loop over the corresponsding squares
		for (int pceNum = 0; pceNum < pos->pce_num[piece]; ++pceNum)
		{
			// case white king
			if (piece == wK)
			{
				// init pieces & squares arrays
				pieces[0] = nnue_pieces[piece];
				squares[0] = SQ64(pos->p_list[piece][pceNum]);
			}

			// case black king
			else if (piece == bK)
			{
				// init pieces & squares arrays
				pieces[1] = nnue_pieces[piece];
				squares[1] = SQ64(pos->p_list[piece][pceNum]);
			}

			// all the other pieces regardless of order
			else
			{
				// init pieces & squares arrays
				pieces[index] = nnue_pieces[piece];
				squares[index] = SQ64(pos->p_list[piece][pceNum]);

				// increment the index
				index++;
			}
		}
	}

	// end piece and square arrays with zero terminating characters
	pieces[index] = 0;
	squares[index] = 0;

	// return NNUE score and give a penalty for 50 move rule counter increasing
	// without this penakty engine might not mate in KQK or KRK endgames!
	return evaluate_nnue(pos->side, pieces, squares) * (100 - pos->fifty_move) / 100;
}
