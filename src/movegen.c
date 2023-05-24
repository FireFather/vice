// movegen.c

#include "stdio.h"
#include "defs.h"

#define MOVE(f,t,ca,pro,fl) ( (f) | ((t) << 7) | ( (ca) << 14 ) | ( (pro) << 20 ) | (fl))
#define SQOFFBOARD(sq) (FilesBrd[(sq)]==OFFBOARD)

const int loop_slide_pce[8] = {
	wB, wR, wQ, 0, bB, bR, bQ, 0
};

const int loop_non_slide_pce[6] = {
	wN, wK, 0, bN, bK, 0
};

const int loop_slide_index[2] = {0, 4};
const int loop_non_slide_index[2] = {0, 3};

const int PceDir[13][8] = {
	{0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0},
	{-8, -19, -21, -12, 8, 19, 21, 12},
	{-9, -11, 11, 9, 0, 0, 0, 0},
	{-1, -10, 1, 10, 0, 0, 0, 0},
	{-1, -10, 1, 10, -9, -11, 11, 9},
	{-1, -10, 1, 10, -9, -11, 11, 9},
	{0, 0, 0, 0, 0, 0, 0},
	{-8, -19, -21, -12, 8, 19, 21, 12},
	{-9, -11, 11, 9, 0, 0, 0, 0},
	{-1, -10, 1, 10, 0, 0, 0, 0},
	{-1, -10, 1, 10, -9, -11, 11, 9},
	{-1, -10, 1, 10, -9, -11, 11, 9}
};

const int num_dir[13] = {
	0, 0, 8, 4, 4, 8, 8, 0, 8, 4, 4, 8, 8
};

/*
PV Move
Cap -> MvvLVA
Killers
HistoryScore

*/
const int victim_score[13] = {0, 100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600};
static int mvv_lva_scores[13][13];

void init_mvv_lva(void)
{
	for (int attacker = wP; attacker <= bK; ++attacker)
	{
		for (int victim = wP; victim <= bK; ++victim)
		{
			mvv_lva_scores[victim][attacker] = victim_score[victim] + 6 - victim_score[attacker] / 100;
		}
	}
}

int move_exists(s_board* pos, const int move)
{
	s_movelist list[1];
	generate_all_moves(pos, list);

	for (int move_num = 0; move_num < list->count; ++move_num)
	{
		if (!make_move(pos, list->moves[move_num].move))
		{
			continue;
		}
		take_move(pos);
		if (list->moves[move_num].move == move)
		{
			return true;
		}
	}
	return false;
}

static void add_quiet_move(const s_board* pos, const int move, s_movelist* list)
{
	ASSERT(sq_on_board(FROMSQ(move)))
	ASSERT(sq_on_board(TOSQ(move)))
	ASSERT(check_board(pos))
	ASSERT(pos->ply >=0 && pos->ply < MAXDEPTH)

	list->moves[list->count].move = move;

	if (pos->search_killers[0][pos->ply] == move)
	{
		list->moves[list->count].score = 900000;
	}
	else if (pos->search_killers[1][pos->ply] == move)
	{
		list->moves[list->count].score = 800000;
	}
	else
	{
		list->moves[list->count].score = pos->search_history[pos->pieces[FROMSQ(move)]][TOSQ(move)];
	}
	list->count++;
}

static void add_capture_move(const s_board* pos, const int move, s_movelist* list)
{
	ASSERT(sq_on_board(FROMSQ(move)))
	ASSERT(sq_on_board(TOSQ(move)))
	ASSERT(piece_valid(CAPTURED(move)))
	ASSERT(check_board(pos))

	list->moves[list->count].move = move;
	list->moves[list->count].score = mvv_lva_scores[CAPTURED(move)][pos->pieces[FROMSQ(move)]] + 1000000;
	list->count++;
}

static void add_en_passant_move(const s_board* pos, const int move, s_movelist* list)
{
	ASSERT(sq_on_board(FROMSQ(move)))
	ASSERT(sq_on_board(TOSQ(move)))
	ASSERT(check_board(pos))
	ASSERT((RanksBrd[TOSQ(move)]==rank_6 && pos->side == white) || (RanksBrd[TOSQ(move)]==rank_3 && pos->side == black))

	list->moves[list->count].move = move;
	list->moves[list->count].score = 105 + 1000000;
	list->count++;
}

static void add_white_pawn_cap_move(const s_board* pos, const int from, const int to, const int cap, s_movelist* list)
{
	ASSERT(piece_valid_empty(cap))
	ASSERT(sq_on_board(from))
	ASSERT(sq_on_board(to))
	ASSERT(check_board(pos))

	if (RanksBrd[from] == rank_7)
	{
		add_capture_move(pos, MOVE(from, to, cap, wQ, 0), list);
		add_capture_move(pos, MOVE(from, to, cap, wR, 0), list);
		add_capture_move(pos, MOVE(from, to, cap, wB, 0), list);
		add_capture_move(pos, MOVE(from, to, cap, wN, 0), list);
	}
	else
	{
		add_capture_move(pos, MOVE(from, to, cap, EMPTY, 0), list);
	}
}

static void add_white_pawn_move(const s_board* pos, const int from, const int to, s_movelist* list)
{
	ASSERT(sq_on_board(from))
	ASSERT(sq_on_board(to))
	ASSERT(check_board(pos))

	if (RanksBrd[from] == rank_7)
	{
		add_quiet_move(pos, MOVE(from, to, EMPTY, wQ, 0), list);
		add_quiet_move(pos, MOVE(from, to, EMPTY, wR, 0), list);
		add_quiet_move(pos, MOVE(from, to, EMPTY, wB, 0), list);
		add_quiet_move(pos, MOVE(from, to, EMPTY, wN, 0), list);
	}
	else
	{
		add_quiet_move(pos, MOVE(from, to, EMPTY, EMPTY, 0), list);
	}
}

static void add_black_pawn_cap_move(const s_board* pos, const int from, const int to, const int cap, s_movelist* list)
{
	ASSERT(piece_valid_empty(cap))
	ASSERT(sq_on_board(from))
	ASSERT(sq_on_board(to))
	ASSERT(check_board(pos))

	if (RanksBrd[from] == rank_2)
	{
		add_capture_move(pos, MOVE(from, to, cap, bQ, 0), list);
		add_capture_move(pos, MOVE(from, to, cap, bR, 0), list);
		add_capture_move(pos, MOVE(from, to, cap, bB, 0), list);
		add_capture_move(pos, MOVE(from, to, cap, bN, 0), list);
	}
	else
	{
		add_capture_move(pos, MOVE(from, to, cap, EMPTY, 0), list);
	}
}

static void add_black_pawn_move(const s_board* pos, const int from, const int to, s_movelist* list)
{
	ASSERT(sq_on_board(from))
	ASSERT(sq_on_board(to))
	ASSERT(check_board(pos))

	if (RanksBrd[from] == rank_2)
	{
		add_quiet_move(pos, MOVE(from, to, EMPTY, bQ, 0), list);
		add_quiet_move(pos, MOVE(from, to, EMPTY, bR, 0), list);
		add_quiet_move(pos, MOVE(from, to, EMPTY, bB, 0), list);
		add_quiet_move(pos, MOVE(from, to, EMPTY, bN, 0), list);
	}
	else
	{
		add_quiet_move(pos, MOVE(from, to, EMPTY, EMPTY, 0), list);
	}
}

void generate_all_moves(const s_board* pos, s_movelist* list)
{
	ASSERT(check_board(pos))

	list->count = 0;

	const int side = pos->side;
	int sq;
	int t_sq;
	int pce_num;
	int dir;
	int index;

	if (side == white)
	{
		for (pce_num = 0; pce_num < pos->pce_num[wP]; ++pce_num)
		{
			sq = pos->p_list[wP][pce_num];
			ASSERT(sq_on_board(sq))

			if (pos->pieces[sq + 10] == EMPTY)
			{
				add_white_pawn_move(pos, sq, sq + 10, list);
				if (RanksBrd[sq] == rank_2 && pos->pieces[sq + 20] == EMPTY)
				{
					add_quiet_move(pos, MOVE(sq, sq+20, EMPTY, EMPTY, mflagps), list);
				}
			}

			if (!SQOFFBOARD(sq + 9) && piece_col[pos->pieces[sq + 9]] == black)
			{
				add_white_pawn_cap_move(pos, sq, sq + 9, pos->pieces[sq + 9], list);
			}
			if (!SQOFFBOARD(sq + 11) && piece_col[pos->pieces[sq + 11]] == black)
			{
				add_white_pawn_cap_move(pos, sq, sq + 11, pos->pieces[sq + 11], list);
			}

			if (pos->enPas != no_sq)
			{
				if (sq + 9 == pos->enPas)
				{
					add_en_passant_move(pos, MOVE(sq, sq + 9, EMPTY, EMPTY, mflagep), list);
				}
				if (sq + 11 == pos->enPas)
				{
					add_en_passant_move(pos, MOVE(sq, sq + 11, EMPTY, EMPTY, mflagep), list);
				}
			}
		}

		if (pos->castlePerm & wkca)
		{
			if (pos->pieces[f1] == EMPTY && pos->pieces[g1] == EMPTY)
			{
				if (!sq_attacked(e1, black, pos) && !sq_attacked(f1, black, pos))
				{
					add_quiet_move(pos, MOVE(e1, g1, EMPTY, EMPTY, mflagca), list);
				}
			}
		}

		if (pos->castlePerm & wqca)
		{
			if (pos->pieces[d1] == EMPTY && pos->pieces[c1] == EMPTY && pos->pieces[b1] == EMPTY)
			{
				if (!sq_attacked(e1, black, pos) && !sq_attacked(d1, black, pos))
				{
					add_quiet_move(pos, MOVE(e1, c1, EMPTY, EMPTY, mflagca), list);
				}
			}
		}
	}
	else
	{
		for (pce_num = 0; pce_num < pos->pce_num[bP]; ++pce_num)
		{
			sq = pos->p_list[bP][pce_num];
			ASSERT(sq_on_board(sq))

			if (pos->pieces[sq - 10] == EMPTY)
			{
				add_black_pawn_move(pos, sq, sq - 10, list);
				if (RanksBrd[sq] == rank_7 && pos->pieces[sq - 20] == EMPTY)
				{
					add_quiet_move(pos, MOVE(sq, sq-20, EMPTY, EMPTY, mflagps), list);
				}
			}

			if (!SQOFFBOARD(sq - 9) && piece_col[pos->pieces[sq - 9]] == white)
			{
				add_black_pawn_cap_move(pos, sq, sq - 9, pos->pieces[sq - 9], list);
			}

			if (!SQOFFBOARD(sq - 11) && piece_col[pos->pieces[sq - 11]] == white)
			{
				add_black_pawn_cap_move(pos, sq, sq - 11, pos->pieces[sq - 11], list);
			}
			if (pos->enPas != no_sq)
			{
				if (sq - 9 == pos->enPas)
				{
					add_en_passant_move(pos, MOVE(sq, sq - 9, EMPTY, EMPTY, mflagep), list);
				}
				if (sq - 11 == pos->enPas)
				{
					add_en_passant_move(pos, MOVE(sq, sq - 11, EMPTY, EMPTY, mflagep), list);
				}
			}
		}

		// castling
		if (pos->castlePerm & bkca)
		{
			if (pos->pieces[f8] == EMPTY && pos->pieces[g8] == EMPTY)
			{
				if (!sq_attacked(e8, white, pos) && !sq_attacked(f8, white, pos))
				{
					add_quiet_move(pos, MOVE(e8, g8, EMPTY, EMPTY, mflagca), list);
				}
			}
		}

		if (pos->castlePerm & bqca)
		{
			if (pos->pieces[d8] == EMPTY && pos->pieces[c8] == EMPTY && pos->pieces[b8] == EMPTY)
			{
				if (!sq_attacked(e8, white, pos) && !sq_attacked(d8, white, pos))
				{
					add_quiet_move(pos, MOVE(e8, c8, EMPTY, EMPTY, mflagca), list);
				}
			}
		}
	}

	/* Loop for slide pieces */
	int pce_index = loop_slide_index[side];
	int pce = loop_slide_pce[pce_index++];
	while (pce != 0)
	{
		ASSERT(piece_valid(pce))

		for (pce_num = 0; pce_num < pos->pce_num[pce]; ++pce_num)
		{
			sq = pos->p_list[pce][pce_num];
			ASSERT(sq_on_board(sq))

			for (index = 0; index < num_dir[pce]; ++index)
			{
				dir = PceDir[pce][index];
				t_sq = sq + dir;

				while (!SQOFFBOARD(t_sq))
				{
					// black ^ 1 == white       white ^ 1 == black
					if (pos->pieces[t_sq] != EMPTY)
					{
						if (piece_col[pos->pieces[t_sq]] == (side ^ 1))
						{
							add_capture_move(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
						}
						break;
					}
					add_quiet_move(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);
					t_sq += dir;
				}
			}
		}

		pce = loop_slide_pce[pce_index++];
	}

	/* Loop for non slide */
	pce_index = loop_non_slide_index[side];
	pce = loop_non_slide_pce[pce_index++];

	while (pce != 0)
	{
		ASSERT(piece_valid(pce))

		for (pce_num = 0; pce_num < pos->pce_num[pce]; ++pce_num)
		{
			sq = pos->p_list[pce][pce_num];
			ASSERT(sq_on_board(sq))

			for (index = 0; index < num_dir[pce]; ++index)
			{
				dir = PceDir[pce][index];
				t_sq = sq + dir;

				if (SQOFFBOARD(t_sq))
				{
					continue;
				}

				// black ^ 1 == white       white ^ 1 == black
				if (pos->pieces[t_sq] != EMPTY)
				{
					if (piece_col[pos->pieces[t_sq]] == (side ^ 1))
					{
						add_capture_move(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
					}
					continue;
				}
				add_quiet_move(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);
			}
		}

		pce = loop_non_slide_pce[pce_index++];
	}

	ASSERT(move_list_ok(list,pos))
}

void generate_all_caps(const s_board* pos, s_movelist* list)
{
	ASSERT(check_board(pos))

	list->count = 0;

	const int side = pos->side;
	int sq;
	int t_sq;
	int pce_num;
	int dir;
	int index;

	if (side == white)
	{
		for (pce_num = 0; pce_num < pos->pce_num[wP]; ++pce_num)
		{
			sq = pos->p_list[wP][pce_num];
			ASSERT(sq_on_board(sq))

			if (!SQOFFBOARD(sq + 9) && piece_col[pos->pieces[sq + 9]] == black)
			{
				add_white_pawn_cap_move(pos, sq, sq + 9, pos->pieces[sq + 9], list);
			}
			if (!SQOFFBOARD(sq + 11) && piece_col[pos->pieces[sq + 11]] == black)
			{
				add_white_pawn_cap_move(pos, sq, sq + 11, pos->pieces[sq + 11], list);
			}

			if (pos->enPas != no_sq)
			{
				if (sq + 9 == pos->enPas)
				{
					add_en_passant_move(pos, MOVE(sq, sq + 9, EMPTY, EMPTY, mflagep), list);
				}
				if (sq + 11 == pos->enPas)
				{
					add_en_passant_move(pos, MOVE(sq, sq + 11, EMPTY, EMPTY, mflagep), list);
				}
			}
		}
	}
	else
	{
		for (pce_num = 0; pce_num < pos->pce_num[bP]; ++pce_num)
		{
			sq = pos->p_list[bP][pce_num];
			ASSERT(sq_on_board(sq))

			if (!SQOFFBOARD(sq - 9) && piece_col[pos->pieces[sq - 9]] == white)
			{
				add_black_pawn_cap_move(pos, sq, sq - 9, pos->pieces[sq - 9], list);
			}

			if (!SQOFFBOARD(sq - 11) && piece_col[pos->pieces[sq - 11]] == white)
			{
				add_black_pawn_cap_move(pos, sq, sq - 11, pos->pieces[sq - 11], list);
			}
			if (pos->enPas != no_sq)
			{
				if (sq - 9 == pos->enPas)
				{
					add_en_passant_move(pos, MOVE(sq, sq - 9, EMPTY, EMPTY, mflagep), list);
				}
				if (sq - 11 == pos->enPas)
				{
					add_en_passant_move(pos, MOVE(sq, sq - 11, EMPTY, EMPTY, mflagep), list);
				}
			}
		}
	}

	/* Loop for slide pieces */
	int pce_index = loop_slide_index[side];
	int pce = loop_slide_pce[pce_index++];
	while (pce != 0)
	{
		ASSERT(piece_valid(pce))

		for (pce_num = 0; pce_num < pos->pce_num[pce]; ++pce_num)
		{
			sq = pos->p_list[pce][pce_num];
			ASSERT(sq_on_board(sq))

			for (index = 0; index < num_dir[pce]; ++index)
			{
				dir = PceDir[pce][index];
				t_sq = sq + dir;

				while (!SQOFFBOARD(t_sq))
				{
					// black ^ 1 == white       white ^ 1 == black
					if (pos->pieces[t_sq] != EMPTY)
					{
						if (piece_col[pos->pieces[t_sq]] == (side ^ 1))
						{
							add_capture_move(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
						}
						break;
					}
					t_sq += dir;
				}
			}
		}

		pce = loop_slide_pce[pce_index++];
	}

	/* Loop for non slide */
	pce_index = loop_non_slide_index[side];
	pce = loop_non_slide_pce[pce_index++];

	while (pce != 0)
	{
		ASSERT(piece_valid(pce))

		for (pce_num = 0; pce_num < pos->pce_num[pce]; ++pce_num)
		{
			sq = pos->p_list[pce][pce_num];
			ASSERT(sq_on_board(sq))

			for (index = 0; index < num_dir[pce]; ++index)
			{
				dir = PceDir[pce][index];
				t_sq = sq + dir;

				if (SQOFFBOARD(t_sq))
				{
					continue;
				}

				// black ^ 1 == white       white ^ 1 == black
				if (pos->pieces[t_sq] != EMPTY)
				{
					if (piece_col[pos->pieces[t_sq]] == (side ^ 1))
					{
						add_capture_move(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
					}
				}
			}
		}

		pce = loop_non_slide_pce[pce_index++];
	}
	ASSERT(move_list_ok(list,pos))
}
