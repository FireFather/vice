// board.c

#include "stdio.h"
#include "defs.h"

void update_lists_material(s_board* pos)
{
	for (int index = 0; index < brd_sq_num; ++index)
	{
		const int sq = index;
		const int piece = pos->pieces[index];
		ASSERT(pce_valid_empty_offbrd(piece))
		if (piece != OFFBOARD && piece != EMPTY)
		{
			const int colour = piece_col[piece];
			ASSERT(side_valid(colour))

			if (piece_big[piece] == true) pos->big_pce[colour]++;
			if (piece_min[piece] == true) pos->min_pce[colour]++;
			if (piece_maj[piece] == true) pos->maj_pce[colour]++;

			pos->material[colour] += PieceVal[piece];

			ASSERT(pos->pce_num[piece] < 10 && pos->pce_num[piece] >= 0)

			pos->p_list[piece][pos->pce_num[piece]] = sq;
			pos->pce_num[piece]++;

			if (piece == wK) pos->king_sq[white] = sq;
			if (piece == bK) pos->king_sq[black] = sq;

			if (piece == wP)
			{
				SETBIT(pos->pawns[white], SQ64(sq));
				SETBIT(pos->pawns[both], SQ64(sq));
			}
			else if (piece == bP)
			{
				SETBIT(pos->pawns[black], SQ64(sq));
				SETBIT(pos->pawns[both], SQ64(sq));
			}
		}
	}
}

int parse_fen(const char* fen, s_board* pos)
{
	ASSERT(fen!=NULL)
	ASSERT(pos!=NULL)

	int rank = rank_8;
	int file = file_a;
	int piece;
	int i;

	reset_board(pos);

	while (rank >= rank_1 && *fen)
	{
		int count = 1;
		switch (*fen)
		{
		case 'p': piece = bP;
			break;
		case 'r': piece = bR;
			break;
		case 'n': piece = bN;
			break;
		case 'b': piece = bB;
			break;
		case 'k': piece = bK;
			break;
		case 'q': piece = bQ;
			break;
		case 'P': piece = wP;
			break;
		case 'R': piece = wR;
			break;
		case 'N': piece = wN;
			break;
		case 'B': piece = wB;
			break;
		case 'K': piece = wK;
			break;
		case 'Q': piece = wQ;
			break;

		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
			piece = EMPTY;
			count = *fen - '0';
			break;

		case '/':
		case ' ':
			rank--;
			file = file_a;
			fen++;
			continue;

		default:
			return -1;
		}

		for (i = 0; i < count; i++)
		{
			const int sq64 = rank * 8 + file;
			const int sq120 = SQ120(sq64);
			if (piece != EMPTY)
			{
				pos->pieces[sq120] = piece;
			}
			file++;
		}
		fen++;
	}

	ASSERT(*fen == 'w' || *fen == 'b')

	pos->side = *fen == 'w' ? white : black;
	fen += 2;

	for (i = 0; i < 4; i++)
	{
		if (*fen == ' ')
		{
			break;
		}
		switch (*fen)
		{
		case 'K': pos->castlePerm |= wkca;
			break;
		case 'Q': pos->castlePerm |= wqca;
			break;
		case 'k': pos->castlePerm |= bkca;
			break;
		case 'q': pos->castlePerm |= bqca;
			break;
		default: break;
		}
		fen++;
	}
	fen++;

	ASSERT(pos->castlePerm>=0 && pos->castlePerm <= 15)

	if (*fen != '-')
	{
		file = fen[0] - 'a';
		rank = fen[1] - '1';

		ASSERT(file>=file_a && file <= file_h)
		ASSERT(rank>=rank_1 && rank <= rank_8)

		pos->enPas = FR2_SQ(file, rank);
	}

	pos->posKey = generate_pos_key(pos);

	update_lists_material(pos);

	return 0;
}

void reset_board(s_board* pos)
{
	int index;

	for (index = 0; index < brd_sq_num; ++index)
	{
		pos->pieces[index] = OFFBOARD;
	}

	for (index = 0; index < 64; ++index)
	{
		pos->pieces[SQ120(index)] = EMPTY;
	}

	for (index = 0; index < 2; ++index)
	{
		pos->big_pce[index] = 0;
		pos->maj_pce[index] = 0;
		pos->min_pce[index] = 0;
		pos->material[index] = 0;
	}

	for (index = 0; index < 3; ++index)
	{
		pos->pawns[index] = 0ULL;
	}

	for (index = 0; index < 13; ++index)
	{
		pos->pce_num[index] = 0;
	}

	pos->king_sq[white] = pos->king_sq[black] = no_sq;

	pos->side = both;
	pos->enPas = no_sq;
	pos->fifty_move = 0;

	pos->ply = 0;
	pos->his_ply = 0;

	pos->castlePerm = 0;

	pos->posKey = 0ULL;
}

void print_board(const s_board* pos)
{
	int file;

	for (int rank = rank_8; rank >= rank_1; rank--)
	{
		printf("%d  ", rank + 1);
		for (file = file_a; file <= file_h; file++)
		{
			const int sq = FR2_SQ(file, rank);
			const int piece = pos->pieces[sq];
			printf("%3c", pce_char[piece]);
		}
		printf("\n");
	}

	printf("\n   ");
	for (file = file_a; file <= file_h; file++)
	{
		printf("%3c", 'a' + file);
	}
	printf("\n");
	printf("toplay: %c\n", side_char[pos->side]);
	printf("enpass: %d\n", pos->enPas);
	printf("castle: %c%c%c%c\n",
	       pos->castlePerm & wkca ? 'K' : '-',
	       pos->castlePerm & wqca ? 'Q' : '-',
	       pos->castlePerm & bkca ? 'k' : '-',
	       pos->castlePerm & bqca ? 'q' : '-'
	);
	printf("poskey: %llX\n", pos->posKey);
}

#ifdef DEBUG
void mirror_board(s_board* pos)
{
	int temp_pieces_array[64];
	const int temp_side = pos->side ^ 1;
	const int swap_piece[13] = {EMPTY, bP, bN, bB, bR, bQ, bK, wP, wN, wB, wR, wQ, wK};
	int temp_castle_perm = 0;
	int temp_en_pas = no_sq;

	int sq;

	if (pos->castlePerm & wkca) temp_castle_perm |= bkca;
	if (pos->castlePerm & wqca) temp_castle_perm |= bqca;

	if (pos->castlePerm & bkca) temp_castle_perm |= wkca;
	if (pos->castlePerm & bqca) temp_castle_perm |= wqca;

	if (pos->enPas != no_sq)
	{
		temp_en_pas = SQ120(Mirror64[SQ64(pos->enPas)]);
	}

	for (sq = 0; sq < 64; sq++)
	{
		temp_pieces_array[sq] = pos->pieces[SQ120(Mirror64[sq])];
	}

	reset_board(pos);

	for (sq = 0; sq < 64; sq++)
	{
		const int tp = swap_piece[temp_pieces_array[sq]];
		pos->pieces[SQ120(sq)] = tp;
	}

	pos->side = temp_side;
	pos->castlePerm = temp_castle_perm;
	pos->enPas = temp_en_pas;

	pos->posKey = generate_pos_key(pos);

	update_lists_material(pos);

	ASSERT(check_board(pos))
}

int pce_list_ok(const s_board* pos)
{
	int pce;
	for (pce = wP; pce <= bK; ++pce)
	{
		if (pos->pce_num[pce] < 0 || pos->pce_num[pce] >= 10) return false;
	}

	if (pos->pce_num[wK] != 1 || pos->pce_num[bK] != 1) return false;

	for (pce = wP; pce <= bK; ++pce)
	{
		for (int num = 0; num < pos->pce_num[pce]; ++num)
		{
			const int sq = pos->p_list[pce][num];
			if (!sq_on_board(sq)) return false;
		}
	}
	return true;
}

int check_board(const s_board* pos)
{
	int t_pce_num[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int t_big_pce[2] = {0, 0};
	int t_maj_pce[2] = {0, 0};
	int t_min_pce[2] = {0, 0};
	int t_material[2] = {0, 0};

	int sq64, t_piece, sq120;

	U64 t_pawns[3] = {0ULL, 0ULL, 0ULL};

	t_pawns[white] = pos->pawns[white];
	t_pawns[black] = pos->pawns[black];
	t_pawns[both] = pos->pawns[both];

	// check piece lists
	for (t_piece = wP; t_piece <= bK; ++t_piece)
	{
		for (int pcenum = 0; pcenum < pos->pce_num[t_piece]; ++pcenum)
		{
			sq120 = pos->p_list[t_piece][pcenum];
			ASSERT(pos->pieces[sq120] == t_piece)
		}
	}

	// check piece count and other counters
	for (sq64 = 0; sq64 < 64; ++sq64)
	{
		sq120 = SQ120(sq64);
		t_piece = pos->pieces[sq120];
		t_pce_num[t_piece]++;
		const int colour = piece_col[t_piece];
		if (piece_big[t_piece] == true) t_big_pce[colour]++;
		if (piece_min[t_piece] == true) t_min_pce[colour]++;
		if (piece_maj[t_piece] == true) t_maj_pce[colour]++;

		t_material[colour] += PieceVal[t_piece];
	}

	for (t_piece = wP; t_piece <= bK; ++t_piece)
	{
		ASSERT(t_pce_num[t_piece] == pos->pce_num[t_piece])
	}

	// check bitboards count
	int pcount = CNT(t_pawns[white]);
	ASSERT(pcount == pos->pce_num[wP])
	pcount = CNT(t_pawns[black]);
	ASSERT(pcount == pos->pce_num[bP])
	pcount = CNT(t_pawns[both]);
	ASSERT(pcount == pos->pce_num[bP] + pos->pce_num[wP])

	// check bitboards squares
	while (t_pawns[white])
	{
		sq64 = POP(&t_pawns[white]);
		ASSERT(pos->pieces[SQ120(sq64)] == wP)
	}

	while (t_pawns[black])
	{
		sq64 = POP(&t_pawns[black]);
		ASSERT(pos->pieces[SQ120(sq64)] == bP)
	}

	while (t_pawns[both])
	{
		sq64 = POP(&t_pawns[both]);
		ASSERT((pos->pieces[SQ120(sq64)] == bP) || (pos->pieces[SQ120(sq64)] == wP))
	}

	ASSERT(t_material[white] == pos->material[white] && t_material[black] == pos->material[black])
	ASSERT(t_min_pce[white] == pos->min_pce[white] && t_min_pce[black] == pos->min_pce[black])
	ASSERT(t_maj_pce[white] == pos->maj_pce[white] && t_maj_pce[black] == pos->maj_pce[black])
	ASSERT(t_big_pce[white] == pos->big_pce[white] && t_big_pce[black] == pos->big_pce[black])

	ASSERT(pos->side == white || pos->side == black)
	ASSERT(generate_pos_key(pos) == pos->posKey)

	ASSERT(pos->enPas == no_sq || (RanksBrd[pos->enPas] == rank_6 && pos->side == white)
		|| (RanksBrd[pos->enPas] == rank_3 && pos->side == black))

	ASSERT(pos->pieces[pos->king_sq[white]] == wK)
	ASSERT(pos->pieces[pos->king_sq[black]] == bK)

	ASSERT(pos->castlePerm >= 0 && pos->castlePerm <= 15)

	ASSERT(pce_list_ok(pos))

	return true;
}
#endif
