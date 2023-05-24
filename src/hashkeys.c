// hashkeys.c
#include "stdio.h"
#include "defs.h"

U64 generate_pos_key(const s_board* pos)
{
	U64 final_key = 0;

	// pieces
	for (int sq = 0; sq < brd_sq_num; ++sq)
	{
		const int piece = pos->pieces[sq];
		if (piece != no_sq && piece != EMPTY && piece != OFFBOARD)
		{
			ASSERT(piece>=wP && piece<=bK)
			final_key ^= PieceKeys[piece][sq];
		}
	}

	if (pos->side == white)
	{
		final_key ^= SideKey;
	}

	if (pos->enPas != no_sq)
	{
		ASSERT(pos->enPas>=0 && pos->enPas<brd_sq_num)
		ASSERT(sq_on_board(pos->enPas))
		ASSERT(RanksBrd[pos->enPas] == rank_3 || RanksBrd[pos->enPas] == rank_6)
		final_key ^= PieceKeys[EMPTY][pos->enPas];
	}

	ASSERT(pos->castlePerm>=0 && pos->castlePerm<=15)

	final_key ^= CastleKeys[pos->castlePerm];

	return final_key;
}
