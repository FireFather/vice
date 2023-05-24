// io.c

#include "stdio.h"
#include "defs.h"

char* pr_move(const int move)
{
	static char mv_str[6];

	const int ff = FilesBrd[FROMSQ(move)];
	const int rf = RanksBrd[FROMSQ(move)];
	const int ft = FilesBrd[TOSQ(move)];
	const int rt = RanksBrd[TOSQ(move)];

	const int promoted = PROMOTED(move);

	if (promoted)
	{
		char pchar = 'q';
		if (IS_KN(promoted))
		{
			pchar = 'n';
		}
		else if (IS_RQ(promoted) && !IS_BQ(promoted))
		{
			pchar = 'r';
		}
		else if (!IS_RQ(promoted) && IS_BQ(promoted))
		{
			pchar = 'b';
		}
		sprintf(mv_str, "%c%c%c%c%c", 'a' + ff, '1' + rf, 'a' + ft, '1' + rt, pchar);
	}
	else
	{
		sprintf(mv_str, "%c%c%c%c", 'a' + ff, '1' + rf, 'a' + ft, '1' + rt);
	}

	return mv_str;
}

int parse_move(const char* ptr_char, const s_board* pos)
{
	ASSERT(check_board(pos))

	if (ptr_char[1] > '8' || ptr_char[1] < '1') return nomove;
	if (ptr_char[3] > '8' || ptr_char[3] < '1') return nomove;
	if (ptr_char[0] > 'h' || ptr_char[0] < 'a') return nomove;
	if (ptr_char[2] > 'h' || ptr_char[2] < 'a') return nomove;

	const int from = FR2_SQ(ptr_char[0] - 'a', ptr_char[1] - '1');
	const int to = FR2_SQ(ptr_char[2] - 'a', ptr_char[3] - '1');

	ASSERT(sq_on_board(from) && sq_on_board(to))

	s_movelist list[1];
	generate_all_moves(pos, list);

	for (int move_num = 0; move_num < list->count; ++move_num)
	{
		const int move = list->moves[move_num].move;
		if (FROMSQ(move) == from && TOSQ(move) == to)
		{
			const int prom_pce = PROMOTED(move);
			if (prom_pce != EMPTY)
			{
				if (IS_RQ(prom_pce) && !IS_BQ(prom_pce) && ptr_char[4] == 'r')
				{
					return move;
				}
				if (!IS_RQ(prom_pce) && IS_BQ(prom_pce) && ptr_char[4] == 'b')
				{
					return move;
				}
				if (IS_RQ(prom_pce) && IS_BQ(prom_pce) && ptr_char[4] == 'q')
				{
					return move;
				}
				if (IS_KN(prom_pce) && ptr_char[4] == 'n')
				{
					return move;
				}
				continue;
			}
			return move;
		}
	}

	return nomove;
}

#ifdef DEBUG
char* pr_sq(const int sq)
{
	static char sq_str[3];

	const int file = FilesBrd[sq];
	const int rank = RanksBrd[sq];

	sprintf(sq_str, "%c%c", 'a' + file, '1' + rank);

	return sq_str;
}

void print_move_list(const s_movelist* list)
{
	printf("MoveList:\n");

	for (int index = 0; index < list->count; ++index)
	{
		const int move = list->moves[index].move;
		const int score = list->moves[index].score;

		printf("Move:%d > %s (score:%d)\n", index + 1, pr_move(move), score);
	}
	printf("MoveList Total %d Moves:\n\n", list->count);
}
#endif
