// init.c

#include <stdio.h>

#include "defs.h"
#include "stdlib.h"

#include "nnue_eval.h"

#define RAND_64 	((U64)rand() | \
					(U64)rand() << 15 | \
					(U64)rand() << 30 | \
					(U64)rand() << 45 | \
					((U64)rand() & 0xf) << 60 )

int Sq120ToSq64[brd_sq_num];
int Sq64ToSq120[64];

U64 SetMask[64];
U64 ClearMask[64];

U64 PieceKeys[13][120];
U64 SideKey;
U64 CastleKeys[16];

int FilesBrd[brd_sq_num];
int RanksBrd[brd_sq_num];

U64 file_bb_mask[8];
U64 rank_bb_mask[8];

U64 black_passed_mask[64];
U64 white_passed_mask[64];
U64 isolated_mask[64];

void init_eval_masks(void)
{
	int sq;

	for (sq = 0; sq < 8; ++sq)
	{
		file_bb_mask[sq] = 0ULL;
		rank_bb_mask[sq] = 0ULL;
	}

	for (int r = rank_8; r >= rank_1; r--)
	{
		for (int f = file_a; f <= file_h; f++)
		{
			sq = r * 8 + f;
			file_bb_mask[f] |= 1ULL << sq;
			rank_bb_mask[r] |= 1ULL << sq;
		}
	}

	for (sq = 0; sq < 64; ++sq)
	{
		isolated_mask[sq] = 0ULL;
		white_passed_mask[sq] = 0ULL;
		black_passed_mask[sq] = 0ULL;
	}

	for (sq = 0; sq < 64; ++sq)
	{
		int tsq = sq + 8;

		while (tsq < 64)
		{
			white_passed_mask[sq] |= 1ULL << tsq;
			tsq += 8;
		}

		tsq = sq - 8;
		while (tsq >= 0)
		{
			black_passed_mask[sq] |= 1ULL << tsq;
			tsq -= 8;
		}

		if (FilesBrd[SQ120(sq)] > file_a)
		{
			isolated_mask[sq] |= file_bb_mask[FilesBrd[SQ120(sq)] - 1];

			tsq = sq + 7;
			while (tsq < 64)
			{
				white_passed_mask[sq] |= 1ULL << tsq;
				tsq += 8;
			}

			tsq = sq - 9;
			while (tsq >= 0)
			{
				black_passed_mask[sq] |= 1ULL << tsq;
				tsq -= 8;
			}
		}

		if (FilesBrd[SQ120(sq)] < file_h)
		{
			isolated_mask[sq] |= file_bb_mask[FilesBrd[SQ120(sq)] + 1];

			tsq = sq + 9;
			while (tsq < 64)
			{
				white_passed_mask[sq] |= 1ULL << tsq;
				tsq += 8;
			}

			tsq = sq - 7;
			while (tsq >= 0)
			{
				black_passed_mask[sq] |= 1ULL << tsq;
				tsq -= 8;
			}
		}
	}
}

void init_files_ranks_brd(void)
{
	for (int index = 0; index < brd_sq_num; ++index)
	{
		FilesBrd[index] = OFFBOARD;
		RanksBrd[index] = OFFBOARD;
	}

	for (int rank = rank_1; rank <= rank_8; ++rank)
	{
		for (int file = file_a; file <= file_h; ++file)
		{
			const int sq = FR2_SQ(file, rank);
			FilesBrd[sq] = file;
			RanksBrd[sq] = rank;
		}
	}
}

void init_hash_keys(void)
{
	int index;
	for (index = 0; index < 13; ++index)
	{
		for (int index2 = 0; index2 < 120; ++index2)
		{
			PieceKeys[index][index2] = RAND_64;
		}
	}
	SideKey = RAND_64;
	for (index = 0; index < 16; ++index)
	{
		CastleKeys[index] = RAND_64;
	}
}

void init_bit_masks(void)
{
	int index;

	for (index = 0; index < 64; index++)
	{
		SetMask[index] = 0ULL;
		ClearMask[index] = 0ULL;
	}

	for (index = 0; index < 64; index++)
	{
		SetMask[index] |= 1ULL << index;
		ClearMask[index] = ~SetMask[index];
	}
}

void init_sq120_to64(void)
{
	int index;
	int sq64 = 0;
	for (index = 0; index < brd_sq_num; ++index)
	{
		Sq120ToSq64[index] = 65;
	}

	for (index = 0; index < 64; ++index)
	{
		Sq64ToSq120[index] = 120;
	}

	for (int rank = rank_1; rank <= rank_8; ++rank)
	{
		for (int file = file_a; file <= file_h; ++file)
		{
			const int sq = FR2_SQ(file, rank);
			ASSERT(sq_on_board(sq))
			Sq64ToSq120[sq64] = sq;
			Sq120ToSq64[sq] = sq64;
			sq64++;
		}
	}
}

void all_init(void)
{
	init_sq120_to64();
	init_bit_masks();
	init_hash_keys();
	init_files_ranks_brd();
	init_eval_masks();
	init_mvv_lva();
	init_nnue(NNUE_FILE);
}
