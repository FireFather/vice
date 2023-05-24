// uci.c

#include "stdio.h"
#include "defs.h"
#include "tinycthread.h"
#include "string.h"

enum
{
	inputbuffer = 400 * 6
};

thrd_t main_search_thread;

void uci_loop(s_board* pos, s_searchinfo* info)
{
	setbuf(stdin, NULL);
	setbuf(stdout, NULL);

	char line[inputbuffer];
	int mb = 64;
	parse_fen(START_FEN, pos);

	while (true)
	{
		memset(&line[0], 0, sizeof line);
		fflush(stdout);
		if (!fgets(line, inputbuffer, stdin))
			continue;

		if (line[0] == '\n')
			continue;
		if (!strncmp(line, "uci", 3))
		{
			printf("id name %s\n", NAME);
			printf("id author Bluefever\n");
			printf("option name Hash type spin default 64 min 4 max %d\n", max_hash);
			printf("option name Threads type spin default 1 min 1 max %d\n", maxthreads);
			printf("uciok\n");
		}
		if (!strncmp(line, "isready", 7))
		{
			printf("readyok\n");
			continue;
		}
		if (!strncmp(line, "ucinewgame", 10))
		{
			clear_hash_table(hash_table);
			parse_position("position startpos\n", pos);
		}
		if (!strncmp(line, "position", 8))
		{
			parse_position(line, pos);
		}
		else if (!strncmp(line, "go", 2))
		{
			//printf("Seen Go..\n");
			parse_go(line, info, pos, hash_table);
		}
		else if (!strncmp(line, "stop", 4))
		{
			join_search_thread(info);
		}
		else if (!strncmp(line, "quit", 4))
		{
			info->quit = true;
			join_search_thread(info);
			break;
		}
		else if (!strncmp(line, "setoption name Hash value ", 26))
		{
			sscanf(line, "%*s %*s %*s %*s %d", &mb);
			if (mb < 4) mb = 4;
			if (mb > max_hash) mb = max_hash;
			printf("info string Hash set to %d MB\n", mb);
			init_hash_table(hash_table, mb);
		}
		else if (!strncmp(line, "setoption name Threads value ", 29))
		{
			sscanf(line, "%*s %*s %*s %*s %d", &mb);
			if (mb < 1) mb = 1;
			if (mb > maxthreads) mb = maxthreads;
			printf("info string Threads set to %d MB\n", mb);
			info->thread_num = mb;
		}
		else if (!strncmp(line, "perft", 5))
		{
			perft_test(6, pos);
		}
		else if (!strncmp(line, "print", 5))
		{
			print_board(pos);
		}
#ifdef DEBUG
		else if (!strncmp(line, "debug", 4))
		{
			debug_analysis_test(pos, info, hash_table);
			break;
		}
#endif
		if (info->quit) break;
	}
}

void parse_go(const char* line, s_searchinfo* info, s_board* pos, s_hashtable* table)
{
	int depth = -1, movestogo = 30, movetime = -1;
	int time = -1, inc = 0;
	const char* ptr;
	info->timeset = false;

	if ((ptr = strstr(line, "infinite")))
	{
	}

	if ((ptr = strstr(line, "binc")) && pos->side == black)
	{
		inc = atoi(ptr + 5);
	}

	if ((ptr = strstr(line, "winc")) && pos->side == white)
	{
		inc = atoi(ptr + 5);
	}

	if ((ptr = strstr(line, "wtime")) && pos->side == white)
	{
		time = atoi(ptr + 6);
	}

	if ((ptr = strstr(line, "btime")) && pos->side == black)
	{
		time = atoi(ptr + 6);
	}

	if ((ptr = strstr(line, "movestogo")))
	{
		movestogo = atoi(ptr + 10);
	}

	if ((ptr = strstr(line, "movetime")))
	{
		movetime = atoi(ptr + 9);
	}

	if ((ptr = strstr(line, "depth")))
	{
		depth = atoi(ptr + 6);
	}

	if (movetime != -1)
	{
		time = movetime;
		movestogo = 1;
	}

	info->starttime = get_time_ms();
	info->depth = depth;

	if (time != -1)
	{
		info->timeset = true;
		time /= movestogo;
		time -= 50;
		info->stoptime = info->starttime + time + inc;
	}

	if (depth == -1)
	{
		info->depth = MAXDEPTH;
	}
	main_search_thread = launch_search_thread(pos, info, table);
}

void parse_position(const char* line_in, s_board* pos)
{
	line_in += 9;
	const char* ptr_char;

	if (strncmp(line_in, "startpos", 8) == 0)
	{
		parse_fen(START_FEN, pos);
	}
	else
	{
		ptr_char = strstr(line_in, "fen");
		if (ptr_char == NULL)
		{
			parse_fen(START_FEN, pos);
		}
		else
		{
			ptr_char += 4;
			parse_fen(ptr_char, pos);
		}
	}

	ptr_char = strstr(line_in, "moves");

	if (ptr_char != NULL)
	{
		ptr_char += 6;
		while (*ptr_char)
		{
			const int move = parse_move(ptr_char, pos);
			if (move == nomove) break;
			make_move(pos, move);
			pos->ply = 0;
			while (*ptr_char && *ptr_char != ' ') ptr_char++;
			ptr_char++;
		}
	}
}

thrd_t launch_search_thread(s_board* pos, s_searchinfo* info, s_hashtable* table)
{
	s_search_thread_data* p_search_data = malloc(sizeof(s_search_thread_data));

	p_search_data->original_position = pos;
	p_search_data->info = info;
	p_search_data->ttable = table;

	thrd_t th;
	thrd_create(&th, &search_position_thread, p_search_data);

	return th;
}

void join_search_thread(s_searchinfo* info)
{
	info->stopped = true;
	thrd_join(main_search_thread, NULL);
}
