// validate.c

#include "defs.h"
#include "stdio.h"
#include "string.h"

#ifdef DEBUG
int move_list_ok(const s_movelist* list, const s_board* pos)
{
	if (list->count < 0 || list->count >= maxpositionmoves)
	{
		return false;
	}

	for (int move_num = 0; move_num < list->count; ++move_num)
	{
		const int to = TOSQ(list->moves[move_num].move);
		const int from = FROMSQ(list->moves[move_num].move);
		if (!sq_on_board(to) || !sq_on_board(from))
		{
			return false;
		}
		if (!piece_valid(pos->pieces[from]))
		{
			print_board(pos);
			return false;
		}
	}

	return true;
}

int sq_is120(const int sq)
{
	return sq >= 0 && sq < 120;
}

int pce_valid_empty_offbrd(const int pce)
{
	return piece_valid_empty(pce) || pce == OFFBOARD;
}

int sq_on_board(const int sq)
{
	return FilesBrd[sq] == OFFBOARD ? 0 : 1;
}

int side_valid(const int side)
{
	return side == white || side == black ? 1 : 0;
}

int file_rank_valid(const int fr)
{
	return fr >= 0 && fr <= 7 ? 1 : 0;
}

int piece_valid_empty(const int pce)
{
	return pce >= EMPTY && pce <= bK ? 1 : 0;
}

int piece_valid(const int pce)
{
	return pce >= wP && pce <= bK ? 1 : 0;
}

void debug_analysis_test(s_board* pos, s_searchinfo* info, s_hashtable* table)
{
	FILE* file = fopen("lct2.epd", "r");
	char line_in[1024];

	info->depth = MAXDEPTH;
	info->timeset = true;

	if (file == NULL)
	{
		printf("File Not Found\n");
		return;
	}
	while (fgets(line_in, 1024, file) != NULL)
	{
		const int time = 1140000;
		info->starttime = get_time_ms();
		info->stoptime = info->starttime + time;
		clear_hash_table(table);
		parse_fen(line_in, pos);
		printf("\n%s\n", line_in);
		printf("time:%d start:%d stop:%d depth:%d timeset:%d\n",
		       time, info->starttime, info->stoptime, info->depth, info->timeset);
		search_position(pos, info, table);
		memset(&line_in[0], 0, sizeof line_in);
	}
}

void mirror_eval_test(s_board* pos)
{
	FILE* file = fopen("mirror.epd", "r");
	char line_in[1024];
	int positions = 0;
	if (file == NULL)
	{
		printf("File Not Found\n");
		return;
	}
	while (fgets(line_in, 1024, file) != NULL)
	{
		parse_fen(line_in, pos);
		positions++;
		const int ev1 = eval_position(pos);
		mirror_board(pos);
		const int ev2 = eval_position(pos);

		if (ev1 != ev2)
		{
			printf("\n\n\n");
			parse_fen(line_in, pos);
			print_board(pos);
			mirror_board(pos);
			print_board(pos);
			printf("\n\nMirror Fail:\n%s\n", line_in);
			getchar();
			return;
		}

		if (positions % 1000 == 0)
		{
			printf("position %d\n", positions);
		}

		memset(&line_in[0], 0, sizeof line_in);
	}
}
#endif
