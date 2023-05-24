/* NNUE wrapping functions */

// include headers
#include "./nnue/nnue.h"
#include "nnue_eval.h"

// init NNUE
void init_nnue(const char* filename)
{
	// call NNUE probe lib function
	nnue_init(filename);
}

// get NNUE score directly
int evaluate_nnue(const int player, int* pieces, int* squares)
{
	// call NNUE probe lib function
	return nnue_evaluate(player, pieces, squares);
}
