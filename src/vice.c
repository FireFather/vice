// vice.c

#include "stdio.h"
#include "defs.h"
#include "stdlib.h"
#include "string.h"

int main(void)
{
	printf("id name %s\n", NAME);
	printf("id author Bluefever\n");

	all_init();
	s_searchinfo info[1];
	info->thread_num = 1;
	hash_table->p_table = NULL;
	init_hash_table(hash_table, 64);
	s_board pos[1];

	uci_loop(pos, info);

	free(hash_table->p_table);
	return 0;
}
