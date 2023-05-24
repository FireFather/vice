// makemove.c

#include <stdio.h>

#include "defs.h"

#define HASH_PCE(pce,sq) (pos->posKey ^= (PieceKeys[(pce)][(sq)]))
#define HASH_CA (pos->posKey ^= (CastleKeys[(pos->castlePerm)]))
#define HASH_SIDE (pos->posKey ^= (SideKey))
#define HASH_EP (pos->posKey ^= (PieceKeys[EMPTY][(pos->enPas)]))

const int castle_perm[120] = {
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 13, 15, 15, 15, 12, 15, 15, 14, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 7, 15, 15, 15, 3, 15, 15, 11, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
	15, 15, 15, 15, 15, 15, 15, 15, 15, 15
};

static void clear_piece(const int sq, s_board* pos)
{
	ASSERT(sq_on_board(sq))
	ASSERT(check_board(pos))

	const int pce = pos->pieces[sq];

	ASSERT(piece_valid(pce))

	const int col = piece_col[pce];
	int t_pce_num = -1;

	ASSERT(side_valid(col))

	HASH_PCE(pce, sq);

	pos->pieces[sq] = EMPTY;
	pos->material[col] -= PieceVal[pce];

	if (piece_big[pce])
	{
		pos->big_pce[col]--;
		if (piece_maj[pce])
		{
			pos->maj_pce[col]--;
		}
		else
		{
			pos->min_pce[col]--;
		}
	}
	else
	{
		CLRBIT(pos->pawns[col], SQ64(sq));
		CLRBIT(pos->pawns[both], SQ64(sq));
	}

	for (int index = 0; index < pos->pce_num[pce]; ++index)
	{
		if (pos->p_list[pce][index] == sq)
		{
			t_pce_num = index;
			break;
		}
	}

	ASSERT(t_pce_num != -1)
	ASSERT(t_pce_num>=0&&t_pce_num<10)

	pos->pce_num[pce]--;

	pos->p_list[pce][t_pce_num] = pos->p_list[pce][pos->pce_num[pce]];
}

static void add_piece(const int sq, s_board* pos, const int pce)
{
	ASSERT(piece_valid(pce))
	ASSERT(sq_on_board(sq))

	const int col = piece_col[pce];
	ASSERT(side_valid(col))

	HASH_PCE(pce, sq);

	pos->pieces[sq] = pce;

	if (piece_big[pce])
	{
		pos->big_pce[col]++;
		if (piece_maj[pce])
		{
			pos->maj_pce[col]++;
		}
		else
		{
			pos->min_pce[col]++;
		}
	}
	else
	{
		SETBIT(pos->pawns[col], SQ64(sq));
		SETBIT(pos->pawns[both], SQ64(sq));
	}

	pos->material[col] += PieceVal[pce];
	pos->p_list[pce][pos->pce_num[pce]++] = sq;
}

static void move_piece(const int from, const int to, s_board* pos)
{
	ASSERT(sq_on_board(from))
	ASSERT(sq_on_board(to))

	const int pce = pos->pieces[from];
	const int col = piece_col[pce];
	ASSERT(side_valid(col))
	ASSERT(piece_valid(pce))

#ifdef DEBUG
	int t_piece_num = false;
#endif

	HASH_PCE(pce, from);
	pos->pieces[from] = EMPTY;

	HASH_PCE(pce, to);
	pos->pieces[to] = pce;

	if (!piece_big[pce])
	{
		CLRBIT(pos->pawns[col], SQ64(from));
		CLRBIT(pos->pawns[both], SQ64(from));
		SETBIT(pos->pawns[col], SQ64(to));
		SETBIT(pos->pawns[both], SQ64(to));
	}

	for (int index = 0; index < pos->pce_num[pce]; ++index)
	{
		if (pos->p_list[pce][index] == from)
		{
			pos->p_list[pce][index] = to;
#ifdef DEBUG
			t_piece_num = true;
#endif
			break;
		}
	}
	ASSERT(t_piece_num)
}

int make_move(s_board* pos, const int move)
{
	ASSERT(check_board(pos))

	const int from = FROMSQ(move);
	const int to = TOSQ(move);
	const int side = pos->side;

	ASSERT(sq_on_board(from))
	ASSERT(sq_on_board(to))
	ASSERT(side_valid(side))
	ASSERT(piece_valid(pos->pieces[from]))
	ASSERT(pos->his_ply >= 0 && pos->his_ply < maxgamemoves)
	ASSERT(pos->ply >= 0 && pos->ply < MAXDEPTH)

	pos->history[pos->his_ply].pos_key = pos->posKey;

	if (move & mflagep)
	{
		if (side == white)
		{
			clear_piece(to - 10, pos);
		}
		else
		{
			clear_piece(to + 10, pos);
		}
	}
	else if (move & mflagca)
	{
		switch (to)
		{
		case c1:
			move_piece(a1, d1, pos);
			break;
		case c8:
			move_piece(a8, d8, pos);
			break;
		case g1:
			move_piece(h1, f1, pos);
			break;
		case g8:
			move_piece(h8, f8, pos);
			break;
		default: ASSERT(false);
		}
	}

	if (pos->enPas != no_sq)
		HASH_EP;
	HASH_CA;

	pos->history[pos->his_ply].move = move;
	pos->history[pos->his_ply].fifty_move = pos->fifty_move;
	pos->history[pos->his_ply].en_pas = pos->enPas;
	pos->history[pos->his_ply].castle_perm = pos->castlePerm;

	pos->castlePerm &= castle_perm[from];
	pos->castlePerm &= castle_perm[to];
	pos->enPas = no_sq;

	HASH_CA;

	const int captured = CAPTURED(move);
	pos->fifty_move++;

	if (captured != EMPTY)
	{
		ASSERT(piece_valid(captured))
		clear_piece(to, pos);
		pos->fifty_move = 0;
	}

	pos->his_ply++;
	pos->ply++;

	ASSERT(pos->his_ply >= 0 && pos->his_ply < maxgamemoves)
	ASSERT(pos->ply >= 0 && pos->ply < MAXDEPTH)

	if (piece_pawn[pos->pieces[from]])
	{
		pos->fifty_move = 0;
		if (move & mflagps)
		{
			if (side == white)
			{
				pos->enPas = from + 10;
				ASSERT(RanksBrd[pos->enPas]==rank_3)
			}
			else
			{
				pos->enPas = from - 10;
				ASSERT(RanksBrd[pos->enPas]==rank_6)
			}
			HASH_EP;
		}
	}

	move_piece(from, to, pos);

	const int pr_pce = PROMOTED(move);
	if (pr_pce != EMPTY)
	{
		ASSERT(piece_valid(pr_pce) && !piece_pawn[pr_pce])
		clear_piece(to, pos);
		add_piece(to, pos, pr_pce);
	}

	if (PieceKing[pos->pieces[to]])
	{
		pos->king_sq[pos->side] = to;
	}

	pos->side ^= 1;
	HASH_SIDE;

	ASSERT(check_board(pos))

	if (sq_attacked(pos->king_sq[side], pos->side, pos))
	{
		take_move(pos);
		return false;
	}

	return true;
}

void take_move(s_board* pos)
{
	ASSERT(check_board(pos))

	pos->his_ply--;
	pos->ply--;

	ASSERT(pos->his_ply >= 0 && pos->his_ply < maxgamemoves)
	ASSERT(pos->ply >= 0 && pos->ply < MAXDEPTH)

	const int move = pos->history[pos->his_ply].move;
	const int from = FROMSQ(move);
	const int to = TOSQ(move);

	ASSERT(sq_on_board(from))
	ASSERT(sq_on_board(to))

	if (pos->enPas != no_sq)
		HASH_EP;
	HASH_CA;

	pos->castlePerm = pos->history[pos->his_ply].castle_perm;
	pos->fifty_move = pos->history[pos->his_ply].fifty_move;
	pos->enPas = pos->history[pos->his_ply].en_pas;

	if (pos->enPas != no_sq)
		HASH_EP;
	HASH_CA;

	pos->side ^= 1;
	HASH_SIDE;

	if (mflagep & move)
	{
		if (pos->side == white)
		{
			add_piece(to - 10, pos, bP);
		}
		else
		{
			add_piece(to + 10, pos, wP);
		}
	}
	else if (mflagca & move)
	{
		switch (to)
		{
		case c1: move_piece(d1, a1, pos);
			break;
		case c8: move_piece(d8, a8, pos);
			break;
		case g1: move_piece(f1, h1, pos);
			break;
		case g8: move_piece(f8, h8, pos);
			break;
		default: ASSERT(false);
		}
	}

	move_piece(to, from, pos);

	if (PieceKing[pos->pieces[from]])
	{
		pos->king_sq[pos->side] = from;
	}

	const int captured = CAPTURED(move);
	if (captured != EMPTY)
	{
		ASSERT(piece_valid(captured))
		add_piece(to, pos, captured);
	}

	if (PROMOTED(move) != EMPTY)
	{
		ASSERT(piece_valid(PROMOTED(move)) && !piece_pawn[PROMOTED(move)])
		clear_piece(from, pos);
		add_piece(from, pos, piece_col[PROMOTED(move)] == white ? wP : bP);
	}

	ASSERT(check_board(pos))
}

void make_null_move(s_board* pos)
{
	ASSERT(check_board(pos))
	ASSERT(!sq_attacked(pos->king_sq[pos->side],pos->side^1,pos))

	pos->ply++;
	pos->history[pos->his_ply].pos_key = pos->posKey;

	if (pos->enPas != no_sq)
		HASH_EP;

	pos->history[pos->his_ply].move = nomove;
	pos->history[pos->his_ply].fifty_move = pos->fifty_move;
	pos->history[pos->his_ply].en_pas = pos->enPas;
	pos->history[pos->his_ply].castle_perm = pos->castlePerm;
	pos->enPas = no_sq;

	pos->side ^= 1;
	pos->his_ply++;
	HASH_SIDE;

	ASSERT(check_board(pos))
	ASSERT(pos->his_ply >= 0 && pos->his_ply < maxgamemoves)
	ASSERT(pos->ply >= 0 && pos->ply < MAXDEPTH)
} // MakeNullMove

void take_null_move(s_board* pos)
{
	ASSERT(check_board(pos))

	pos->his_ply--;
	pos->ply--;

	if (pos->enPas != no_sq)
		HASH_EP;

	pos->castlePerm = pos->history[pos->his_ply].castle_perm;
	pos->fifty_move = pos->history[pos->his_ply].fifty_move;
	pos->enPas = pos->history[pos->his_ply].en_pas;

	if (pos->enPas != no_sq)
		HASH_EP;
	pos->side ^= 1;
	HASH_SIDE;

	ASSERT(check_board(pos))
	ASSERT(pos->his_ply >= 0 && pos->his_ply < maxgamemoves)
	ASSERT(pos->ply >= 0 && pos->ply < MAXDEPTH)
}
