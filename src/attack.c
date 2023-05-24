// attack.c

#include "stdio.h"
#include "defs.h"

const int kn_dir[8] = {-8, -19, -21, -12, 8, 19, 21, 12};
const int rk_dir[4] = {-1, -10, 1, 10};
const int bi_dir[4] = {-9, -11, 11, 9};
const int ki_dir[8] = {-1, -10, 1, 10, -9, -11, 11, 9};

int sq_attacked(const int sq, const int side, const s_board* pos)
{
	int pce, index, t_sq, dir;

	ASSERT(sq_on_board(sq))
	ASSERT(side_valid(side))
	ASSERT(check_board(pos))

	// pawns
	if (side == white)
	{
		if (pos->pieces[sq - 11] == wP || pos->pieces[sq - 9] == wP)
		{
			return true;
		}
	}
	else
	{
		if (pos->pieces[sq + 11] == bP || pos->pieces[sq + 9] == bP)
		{
			return true;
		}
	}

	// knights
	for (index = 0; index < 8; ++index)
	{
		pce = pos->pieces[sq + kn_dir[index]];
		ASSERT(pce_valid_empty_offbrd(pce))
		if (pce != OFFBOARD && IS_KN(pce) && piece_col[pce] == side)
		{
			return true;
		}
	}

	// rooks, queens
	for (index = 0; index < 4; ++index)
	{
		dir = rk_dir[index];
		t_sq = sq + dir;
		ASSERT(sq_is120(t_sq))
		pce = pos->pieces[t_sq];
		ASSERT(pce_valid_empty_offbrd(pce))
		while (pce != OFFBOARD)
		{
			if (pce != EMPTY)
			{
				if (IS_RQ(pce) && piece_col[pce] == side)
				{
					return true;
				}
				break;
			}
			t_sq += dir;
			ASSERT(sq_is120(t_sq))
			pce = pos->pieces[t_sq];
		}
	}

	// bishops, queens
	for (index = 0; index < 4; ++index)
	{
		dir = bi_dir[index];
		t_sq = sq + dir;
		ASSERT(sq_is120(t_sq))
		pce = pos->pieces[t_sq];
		ASSERT(pce_valid_empty_offbrd(pce))
		while (pce != OFFBOARD)
		{
			if (pce != EMPTY)
			{
				if (IS_BQ(pce) && piece_col[pce] == side)
				{
					return true;
				}
				break;
			}
			t_sq += dir;
			ASSERT(sq_is120(t_sq))
			pce = pos->pieces[t_sq];
		}
	}

	// kings
	for (index = 0; index < 8; ++index)
	{
		pce = pos->pieces[sq + ki_dir[index]];
		ASSERT(pce_valid_empty_offbrd(pce))
		if (pce != OFFBOARD && IS_KI(pce) && piece_col[pce] == side)
		{
			return true;
		}
	}

	return false;
}
