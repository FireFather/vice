// search.c

#include "stdio.h"
#include "string.h"
#include "defs.h"
#include "tinycthread.h"

int root_depth;
thrd_t worker_threads[maxthreads];

static void check_up(s_searchinfo* info)
{
	// .. check if time up, or interrupt from GUI
	if (info->timeset == true && get_time_ms() > info->stoptime)
	{
		info->stopped = true;
	}
}

static void pick_next_move(const int move_num, s_movelist* list)
{
	int best_score = 0;
	int best_num = move_num;

	for (int index = move_num; index < list->count; ++index)
	{
		if (list->moves[index].score > best_score)
		{
			best_score = list->moves[index].score;
			best_num = index;
		}
	}

	ASSERT(move_num>=0 && move_num<list->count)
	ASSERT(best_num>=0 && best_num<list->count)
	ASSERT(best_num>=move_num)

	const s_move temp = list->moves[move_num];
	list->moves[move_num] = list->moves[best_num];
	list->moves[best_num] = temp;
}

static int is_repetition(const s_board* pos)
{
	for (int index = pos->his_ply - pos->fifty_move; index < pos->his_ply - 1; ++index)
	{
		ASSERT(index >= 0 && index < maxgamemoves)
		if (pos->posKey == pos->history[index].pos_key)
		{
			return true;
		}
	}
	return false;
}

static void clear_for_search(s_board* pos, s_searchinfo* info, s_hashtable* table)
{
	int index;
	int index2;

	for (index = 0; index < 13; ++index)
	{
		for (index2 = 0; index2 < brd_sq_num; ++index2)
		{
			pos->search_history[index][index2] = 0;
		}
	}

	for (index = 0; index < 2; ++index)
	{
		for (index2 = 0; index2 < MAXDEPTH; ++index2)
		{
			pos->search_killers[index][index2] = 0;
		}
	}

	table->over_write = 0;
	table->hit = 0;
	table->cut = 0;
	pos->ply = 0;
	table->current_age++;

	info->stopped = 0;
	info->nodes = 0;
	info->fh = 0;
	info->fhf = 0;
}

static int quiescence(int alpha, const int beta, s_board* pos, s_searchinfo* info)
{
	ASSERT(check_board(pos))
	ASSERT(beta>alpha)
	if ((info->nodes & 2047) == 0)
	{
		check_up(info);
	}

	info->nodes++;

	if (is_repetition(pos) || pos->fifty_move >= 100)
	{
		return 0;
	}

	if (pos->ply > MAXDEPTH - 1)
	{
		return eval_position(pos);
	}

	int score = eval_position(pos);

	ASSERT(score>-AB_BOUND && score<AB_BOUND)

	if (score >= beta)
	{
		return beta;
	}

	if (score > alpha)
	{
		alpha = score;
	}

	s_movelist list[1];
	generate_all_caps(pos, list);

	int legal = 0;

	for (int move_num = 0; move_num < list->count; ++move_num)
	{
		pick_next_move(move_num, list);

		if (!make_move(pos, list->moves[move_num].move))
		{
			continue;
		}

		legal++;
		score = -quiescence(-beta, -alpha, pos, info);
		take_move(pos);

		if (info->stopped == true)
		{
			return 0;
		}

		if (score > alpha)
		{
			if (score >= beta)
			{
				if (legal == 1)
				{
					info->fhf++;
				}
				info->fh++;
				return beta;
			}
			alpha = score;
		}
	}

	return alpha;
}

static int alpha_beta(int alpha, const int beta, int depth, s_board* pos, s_searchinfo* info, s_hashtable* table,
                      const int do_null)
{
	ASSERT(check_board(pos))
	ASSERT(beta>alpha)
	ASSERT(depth>=0)

	if (depth <= 0)
	{
		return quiescence(alpha, beta, pos, info);
		// return EvalPosition(pos);
	}

	if ((info->nodes & 2047) == 0)
	{
		check_up(info);
	}

	info->nodes++;

	if ((is_repetition(pos) || pos->fifty_move >= 100) && pos->ply)
	{
		return 0;
	}

	if (pos->ply > MAXDEPTH - 1)
	{
		return eval_position(pos);
	}

	const int in_check = sq_attacked(pos->king_sq[pos->side], pos->side ^ 1, pos);

	if (in_check == true)
	{
		depth++;
	}

	int score = -AB_BOUND;
	int pv_move = nomove;

	if (probe_hash_entry(pos, table, &pv_move, &score, alpha, beta, depth) == true)
	{
		table->cut++;
		return score;
	}

	if (do_null && !in_check && pos->ply && pos->big_pce[pos->side] > 1 && depth >= 4)
	{
		make_null_move(pos);
		score = -alpha_beta(-beta, -beta + 1, depth - 4, pos, info, table, false);
		take_null_move(pos);
		if (info->stopped == true)
		{
			return 0;
		}

		if (score >= beta && abs(score) < ISMATE)
		{
			info->null_cut++;
			return beta;
		}
	}

	s_movelist list[1];
	generate_all_moves(pos, list);

	int move_num;
	int legal = 0;
	const int old_alpha = alpha;
	int best_move = nomove;

	int best_score = -AB_BOUND;

	score = -AB_BOUND;

	if (pv_move != nomove)
	{
		for (move_num = 0; move_num < list->count; ++move_num)
		{
			if (list->moves[move_num].move == pv_move)
			{
				list->moves[move_num].score = 2000000;
				//printf("Pv move found \n");
				break;
			}
		}
	}

	for (move_num = 0; move_num < list->count; ++move_num)
	{
		pick_next_move(move_num, list);

		if (!make_move(pos, list->moves[move_num].move))
		{
			continue;
		}

		legal++;
		score = -alpha_beta(-beta, -alpha, depth - 1, pos, info, table, true);
		take_move(pos);

		if (info->stopped == true)
		{
			return 0;
		}
		if (score > best_score)
		{
			best_score = score;
			best_move = list->moves[move_num].move;
			if (score > alpha)
			{
				if (score >= beta)
				{
					if (legal == 1)
					{
						info->fhf++;
					}
					info->fh++;

					if (!(list->moves[move_num].move & mflagcap))
					{
						pos->search_killers[1][pos->ply] = pos->search_killers[0][pos->ply];
						pos->search_killers[0][pos->ply] = list->moves[move_num].move;
					}

					store_hash_entry(pos, table, best_move, beta, hfbeta, depth);

					return beta;
				}
				alpha = score;

				if (!(list->moves[move_num].move & mflagcap))
				{
					pos->search_history[pos->pieces[FROMSQ(best_move)]][TOSQ(best_move)] += depth;
				}
			}
		}
	}

	if (legal == 0)
	{
		if (in_check)
		{
			return -AB_BOUND + pos->ply;
		}
		return 0;
	}

	ASSERT(alpha>=old_alpha)

	if (alpha != old_alpha)
	{
		store_hash_entry(pos, table, best_move, best_score, hfexact, depth);
	}
	else
	{
		store_hash_entry(pos, table, best_move, alpha, hfalpha, depth);
	}

	return alpha;
}

int search_position_thread(void* data)
{
	const s_search_thread_data* search_data = (s_search_thread_data*)data;
	s_board* pos = malloc(sizeof(s_board));
	memcpy(pos, search_data->original_position, sizeof(s_board));
	search_position(pos, search_data->info, search_data->ttable);
	free(pos);
	//printf("Freed\n");
	return 0;
}

void iterative_deepen(s_search_worker_data* worker_data)
{
	worker_data->best_move = nomove;

	for (int current_depth = 1; current_depth <= worker_data->info->depth; ++current_depth)
	{
		root_depth = current_depth;
		const int best_score = alpha_beta(-AB_BOUND, AB_BOUND, current_depth, worker_data->pos, worker_data->info,
		                                  worker_data->ttable,
		                                  true);

		if (worker_data->info->stopped == true)
		{
			break;
		}

		if (worker_data->thread_number == 0)
		{
			const int pv_moves = get_pv_line(current_depth, worker_data->pos, worker_data->ttable);
			worker_data->best_move = worker_data->pos->pv_array[0];
			printf("info score cp %d depth %d nodes %ld time %d pv",
			       best_score, current_depth, worker_data->info->nodes, get_time_ms() - worker_data->info->starttime);

			for (int pv_num = 0; pv_num < pv_moves; ++pv_num)
			{
				printf(" %s", pr_move(worker_data->pos->pv_array[pv_num]));
			}
			printf("\n");
		}
	}
}

int start_worker_thread(void* data)
{
	s_search_worker_data* worker_data = data;
	//printf("Thread:%d Starts\n", worker_data->thread_number);
	iterative_deepen(worker_data);
	//printf("Thread:%d Ends\n", worker_data->thread_number);
	if (worker_data->thread_number == 0)
	{
		printf("bestmove %s\n", pr_move(worker_data->best_move));
	}
	free(data);
	return 0;
}

void setup_worker(const int thread_num, thrd_t* worker_th, const s_board* pos, s_searchinfo* info, s_hashtable* table)
{
	s_search_worker_data* p_worker_data = malloc(sizeof(s_search_worker_data));
	p_worker_data->pos = malloc(sizeof(s_board));
	memcpy(p_worker_data->pos, pos, sizeof(s_board));
	p_worker_data->info = info;
	p_worker_data->ttable = table;
	p_worker_data->thread_number = thread_num;
	thrd_create(worker_th, &start_worker_thread, p_worker_data);
}

void create_search_workers(const s_board* pos, s_searchinfo* info, s_hashtable* table)
{
	//printf("CreateSearchWorkers:%d\n", info->thread_num);
	for (int i = 0; i < info->thread_num; i++)
	{
		setup_worker(i, &worker_threads[i], pos, info, table);
	}
}

void search_position(s_board* pos, s_searchinfo* info, s_hashtable* table)
{
	const int best_move = nomove;

	clear_for_search(pos, info, table);

	// iterative deepening
	if (best_move == nomove)
	{
		create_search_workers(pos, info, table);
	}

	for (int i = 0; i < info->thread_num; i++)
	{
		thrd_join(worker_threads[i], NULL);
	}
}
