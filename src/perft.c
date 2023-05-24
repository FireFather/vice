// perft.c

#include "defs.h"
#include "stdio.h"

long leaf_nodes;

void perft(const int depth, s_board* pos)
{
	ASSERT(check_board(pos))

	if (depth == 0)
	{
		leaf_nodes++;
		return;
	}

	s_movelist list[1];
	generate_all_moves(pos, list);

	for (int move_num = 0; move_num < list->count; ++move_num)
	{
		if (!make_move(pos, list->moves[move_num].move))
		{
			continue;
		}
		perft(depth - 1, pos);
		take_move(pos);
	}
}

void perft_test(const int depth, s_board* pos)
{
	ASSERT(check_board(pos))

	print_board(pos);
	printf("\nrunning perft to depth %d\n", depth);
	leaf_nodes = 0;
	const int start = get_time_ms();
	s_movelist list[1];
	generate_all_moves(pos, list);

	for (int move_num = 0; move_num < list->count; ++move_num)
	{
		const int move = list->moves[move_num].move;
		if (!make_move(pos, move))
		{
			continue;
		}
		const long cumnodes = leaf_nodes;
		perft(depth - 1, pos);
		take_move(pos);
		const long oldnodes = leaf_nodes - cumnodes;
		printf("move %d : %s : %ld\n", move_num + 1, pr_move(move), oldnodes);
	}
	const int elapsed = get_time_ms() - start;
	printf("\nnodes: %ld", leaf_nodes);
	printf("\ntime : %d ms", elapsed);
	printf("\nnps  : %ld", leaf_nodes / elapsed);
}
