#pragma once
#if !defined(_MSC_VER)
#define NNUE_EMBEDDED
#endif

#define NNUE_FILE "nn.bin"

void init_nnue(const char* filename);
int evaluate_nnue(int player, int* pieces, int* squares);
