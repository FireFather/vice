// pvtable.c

#include "stdio.h"
#include "defs.h"

#define EXTRACT_SCORE(x) (((x) & 0xFFFF) - INF_BOUND)
#define EXTRACT_DEPTH(x) (((x) >> 16) & 0x3F)
#define EXTRACT_FLAGS(x) (((x) >> 23) & 0x3)
#define EXTRACT_MOVE(x) ((int)((x) >> 25))

#define FOLD_DATA(sc, de, fl, mv) ( ((sc) + INF_BOUND) | ((de) << 16) | ((fl) << 23)  | ((U64)(mv) << 25))

s_hashtable hash_table[1];

int get_pv_line(const int depth, s_board* pos, const s_hashtable* table)
{
	ASSERT(depth < MAXDEPTH && depth >= 1)

	int move = probe_pv_move(pos, table);
	int count = 0;

	while (move != nomove && count < depth)
	{
		ASSERT(count < MAXDEPTH)

		if (move_exists(pos, move))
		{
			make_move(pos, move);
			pos->pv_array[count++] = move;
		}
		else
		{
			break;
		}
		move = probe_pv_move(pos, table);
	}

	while (pos->ply > 0)
	{
		take_move(pos);
	}

	return count;
}

void clear_hash_table(s_hashtable* table)
{
	for (s_hashentry* table_entry = table->p_table; table_entry < table->p_table + table->num_entries; table_entry++)
	{
		table_entry->age = 0;
		table_entry->smp_data = 0ULL;
		table_entry->smp_key = 0ULL;
	}
	table->new_write = 0;
	table->current_age = 0;
}

void init_hash_table(s_hashtable* table, const int mb)
{
	const int hash_size = 0x100000 * mb;
	table->num_entries = hash_size / sizeof(s_hashentry);
	table->num_entries -= 2;
	//table->num_entries = 1000000;

	if (table->p_table != NULL)
	{
		free(table->p_table);
	}

	table->p_table = (s_hashentry*)malloc(table->num_entries * sizeof(s_hashentry));
	if (table->p_table == NULL)
	{
		//printf("Hash Allocation Failed, trying %dMB...\n", mb / 2);
		init_hash_table(table, mb / 2);
	}
	else
	{
		clear_hash_table(table);
		//printf("HashTable init complete with %d entries\n", table->num_entries);
	}
}

int probe_hash_entry(const s_board* pos, s_hashtable* table, int* move, int* score, const int alpha, const int beta,
                     const int depth)
{
	const int index = pos->posKey % table->num_entries;

	ASSERT(index >= 0 && index <= table->num_entries - 1)
	ASSERT(depth>=1&&depth<MAXDEPTH)
	ASSERT(alpha<beta)
	ASSERT(alpha>=-AB_BOUND&&alpha<=AB_BOUND)
	ASSERT(beta>=-AB_BOUND&&beta<=AB_BOUND)
	ASSERT(pos->ply>=0&&pos->ply<MAXDEPTH)

	const U64 test_key = pos->posKey ^ table->p_table[index].smp_data;
	if (table->p_table[index].smp_key == test_key)
	{
		const int smp_depth = EXTRACT_DEPTH(table->p_table[index].smp_data);
		const int smp_move = EXTRACT_MOVE(table->p_table[index].smp_data);
		const int smp_score = EXTRACT_SCORE(table->p_table[index].smp_data);
		const int smp_flags = EXTRACT_FLAGS(table->p_table[index].smp_data);

		*move = smp_move;
		if (smp_depth >= depth)
		{
			table->hit++;

			//ASSERT(table->pTable[index].depth>=1&&table->pTable[index].depth<MAXDEPTH);
			//ASSERT(table->pTable[index].flags>=hfalpha&&table->pTable[index].flags<=hfexact);

			*score = smp_score;
			if (*score > ISMATE) *score -= pos->ply;
			else if (*score < -ISMATE) *score += pos->ply;

			switch (smp_flags)
			{
			case hfalpha: if (*score <= alpha)
				{
					*score = alpha;
					return true;
				}
				break;
			case hfbeta: if (*score >= beta)
				{
					*score = beta;
					return true;
				}
				break;
			case hfexact:
				return true;
			default: ASSERT(false)
				break;
			}
		}
	}

	return false;
}

void store_hash_entry(const s_board* pos, s_hashtable* table, const int move, int score, const int flags,
                      const int depth)
{
	const int index = pos->posKey % table->num_entries;

	/*if (table->pTable[index].depth > 8 && depth < 8) { 
		printf("new_key:%llX old_key:%llX index:%d depth:%d replace:%d\n", 
		pos->posKey, table->pTable[index].posKey, index, depth, table->pTable[index].depth); 
	}*/

	ASSERT(index >= 0 && index <= table->num_entries - 1)
	ASSERT(depth>=1&&depth<MAXDEPTH)
	ASSERT(flags>=hfalpha&&flags<=hfexact)
	ASSERT(score>=-AB_BOUND&&score<=AB_BOUND)
	ASSERT(pos->ply>=0&&pos->ply<MAXDEPTH)

	int replace = false;

	if (table->p_table[index].smp_key == 0)
	{
		table->new_write++;
		replace = true;
	}
	else
	{
		if (table->p_table[index].age < table->current_age)
		{
			replace = true;
		}
		else if (EXTRACT_DEPTH(table->p_table[index].smp_data) <= depth)
		{
			replace = true;
		}
	}

	if (replace == false) return;

	if (score > ISMATE) score += pos->ply;
	else if (score < -ISMATE) score -= pos->ply;

	const U64 smp_data = FOLD_DATA(score, depth, flags, move);
	//U64 smp_key = pos->posKey ^ smp_data;

	table->p_table[index].age = table->current_age;
	table->p_table[index].smp_data = smp_data;
	table->p_table[index].smp_key = pos->posKey ^ smp_data;
}

int probe_pv_move(const s_board* pos, const s_hashtable* table)
{
	const int index = pos->posKey % table->num_entries;

	const U64 test_key = pos->posKey ^ table->p_table[index].smp_data;

	ASSERT(index >= 0 && index <= table->num_entries - 1)

	if (table->p_table[index].smp_key == test_key)
	{
		return EXTRACT_MOVE(table->p_table[index].smp_data);
	}

	return nomove;
}

#ifdef DEBUG
void data_check(const int move)
{
	const int depth = rand() % MAXDEPTH;
	const int flag = rand() % 3;
	const int score = rand() % AB_BOUND;

	const U64 data = FOLD_DATA(score, depth, flag, move);
	printf("Orig: move:%s d:%d fl:%d sc:%d data:%llX\n", pr_move(move), depth, flag, score, data);
	printf("Check: move:%s d:%lld fl:%lld sc:%lld\n\n",
		pr_move(EXTRACT_MOVE(data)),
		EXTRACT_DEPTH(data),
		EXTRACT_FLAGS(data),
		EXTRACT_SCORE(data));
}

void temp_hash_test(const char* fen)
{
	s_board b[1];
	parse_fen(fen, b);

	s_movelist list[1];
	generate_all_moves(b, list);

	for (int move_num = 0; move_num < list->count; ++move_num)
	{
		if (!make_move(b, list->moves[move_num].move))
		{
			continue;
		}
		take_move(b);
		data_check(list->moves[move_num].move);
	}
}

void verify_entry_smp(void)
{
	/*
	U64 data = FOLD_DATA(entry->score, entry->depth, entry->flags, entry->move);
	U64 key = entry->posKey ^ data;

	if (data != entry->smp_data) { printf("data error"); exit(1);}
	if (key != entry->smp_key) { printf("smp_key error"); exit(1);}

	int move = EXTRACT_MOVE(data);
	int flag = EXTRACT_FLAGS(data);
	int score = EXTRACT_SCORE(data);
	int depth = EXTRACT_DEPTH(data);

	if (move != entry->move) { printf("move error"); exit(1);}
	if (flag != entry->flags) { printf("flags error"); exit(1);}
	if (score != entry->score) { printf("score error"); exit(1);}
	if (depth != entry->depth) { printf("depth error"); exit(1);}
	*/
}
#endif
