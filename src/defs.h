#pragma once

#include "stdlib.h"
#include "tinycthread.h"

//#define DEBUG

enum
{
	max_hash = 1024
};

#ifndef DEBUG
#define ASSERT(n)
#else
#define ASSERT(n) \
if(!(n)) { \
printf("%s - Failed",#n); \
printf("On %s ",__DATE__); \
printf("At %s ",__TIME__); \
printf("In File %s ",__FILE__); \
printf("At Line %d\n",__LINE__); \
exit(1);}
#endif

typedef unsigned long long U64;

#define NAME "Vice 1.2 NN"

enum
{
	brd_sq_num = 120
};

enum
{
	maxgamemoves = 2048,
	maxpositionmoves = 256,
	MAXDEPTH = 64,
	maxthreads = 32
};

#define START_FEN  "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define FINE_70 "8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - -"
#define WAC_2 "8/7p/5k2/5p2/p1p2P2/Pr1pPK2/1P1R3P/8 b - -"
#define LCT_1 "r3kb1r/3n1pp1/p6p/2pPp2q/Pp2N3/3B2PP/1PQ2P2/R3K2R w KQkq -"

enum
{
	INF_BOUND = 32000,
	AB_BOUND = 30000
};

#define ISMATE (AB_BOUND - MAXDEPTH)

enum { EMPTY, wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK };

enum { file_a, file_b, file_c, file_d, file_e, file_f, file_g, file_h, file_none };

enum { rank_1, rank_2, rank_3, rank_4, rank_5, rank_6, rank_7, rank_8, rank_none };

enum { white, black, both };

enum
{
	a1 = 21,
	b1,
	c1,
	d1,
	e1,
	f1,
	g1,
	h1,
	a2 = 31,
	b2,
	c2,
	d2,
	e2,
	f2,
	g2,
	h2,
	a3 = 41,
	b3,
	c3,
	d3,
	e3,
	f3,
	g3,
	h3,
	a4 = 51,
	b4,
	c4,
	d4,
	e4,
	f4,
	g4,
	h4,
	a5 = 61,
	b5,
	c5,
	d5,
	e5,
	f5,
	g5,
	h5,
	a6 = 71,
	b6,
	c6,
	d6,
	e6,
	f6,
	g6,
	h6,
	a7 = 81,
	b7,
	c7,
	d7,
	e7,
	f7,
	g7,
	h7,
	a8 = 91,
	b8,
	c8,
	d8,
	e8,
	f8,
	g8,
	h8,
	no_sq,
	OFFBOARD
};

enum { false, true };

enum { wkca = 1, wqca = 2, bkca = 4, bqca = 8 };

typedef struct
{
	int move;
	int score;
} s_move;

typedef struct
{
	s_move moves[maxpositionmoves];
	int count;
} s_movelist;

enum { hfnone, hfalpha, hfbeta, hfexact };

typedef struct
{
	int age;
	U64 smp_data;
	U64 smp_key;
} s_hashentry;

typedef struct
{
	s_hashentry* p_table;
	int num_entries;
	int new_write;
	int over_write;
	int hit;
	int cut;
	int current_age;
} s_hashtable;

typedef struct
{
	int move;
	int castle_perm;
	int en_pas;
	int fifty_move;
	U64 pos_key;
} s_undo;

typedef struct
{
	int pieces[brd_sq_num];
	U64 pawns[3];

	int king_sq[2];

	int side;
	int enPas;
	int fifty_move;

	int ply;
	int his_ply;

	int castlePerm;

	U64 posKey;

	int pce_num[13];
	int big_pce[2];
	int maj_pce[2];
	int min_pce[2];
	int material[2];

	s_undo history[maxgamemoves];

	// piece list
	int p_list[13][10];

	int pv_array[MAXDEPTH];

	int search_history[13][brd_sq_num];
	int search_killers[2][MAXDEPTH];
} s_board;

typedef struct
{
	int starttime;
	int stoptime;
	int depth;
	int timeset;
	int movestogo;

	long nodes;

	int quit;
	int stopped;

	float fh;
	float fhf;
	int null_cut;

	int thread_num;
} s_searchinfo;

typedef struct
{
	s_searchinfo* info;
	s_board* original_position;
	s_hashtable* ttable;
} s_search_thread_data;

typedef struct
{
	s_board* pos;
	s_searchinfo* info;
	s_hashtable* ttable;

	int thread_number;
	int depth;
	int best_move;
} s_search_worker_data;

/* GAME MOVE */
/*
0000 0000 0000 0000 0000 0111 1111 -> From 0x7F
0000 0000 0000 0011 1111 1000 0000 -> To >> 7, 0x7F
0000 0000 0011 1100 0000 0000 0000 -> Captured >> 14, 0xF
0000 0000 0100 0000 0000 0000 0000 -> EP 0x40000
0000 0000 1000 0000 0000 0000 0000 -> Pawn Start 0x80000
0000 1111 0000 0000 0000 0000 0000 -> Promoted Piece >> 20, 0xF
0001 0000 0000 0000 0000 0000 0000 -> Castle 0x1000000
*/

#define FROMSQ(m) ((m) & 0x7F)
#define TOSQ(m) (((m)>>7) & 0x7F)
#define CAPTURED(m) (((m)>>14) & 0xF)
#define PROMOTED(m) (((m)>>20) & 0xF)

enum
{
	mflagep = 0x40000,
	mflagps = 0x80000,
	mflagca = 0x1000000
};

enum
{
	mflagcap = 0x7C000,
	mflagprom = 0xF00000
};

enum
{
	nomove = 0
};

/* MACROS */

#define FR2_SQ(f,r) ( (21 + (f) ) + ( (r) * 10 ) )
#define SQ64(sq120) (Sq120ToSq64[(sq120)])
#define SQ120(sq64) (Sq64ToSq120[(sq64)])
#define POP(b) pop_bit(b)
#define CNT(b) count_bits(b)
#define CLRBIT(bb,sq) ((bb) &= ClearMask[(sq)])
#define SETBIT(bb,sq) ((bb) |= SetMask[(sq)])

#define IS_BQ(p) (PieceBishopQueen[(p)])
#define IS_RQ(p) (PieceRookQueen[(p)])
#define IS_KN(p) (PieceKnight[(p)])
#define IS_KI(p) (PieceKing[(p)])

#define MIRROR64(sq) (Mirror64[(sq)])

/* GLOBALS */

extern int Sq120ToSq64[brd_sq_num];
extern int Sq64ToSq120[64];
extern U64 SetMask[64];
extern U64 ClearMask[64];
extern U64 PieceKeys[13][120];
extern U64 SideKey;
extern U64 CastleKeys[16];
extern char pce_char[];
extern char side_char[];
extern char rank_char[];
extern char file_char[];

extern int piece_big[13];
extern int piece_maj[13];
extern int piece_min[13];
extern int PieceVal[13];
extern int piece_col[13];
extern int piece_pawn[13];

extern int FilesBrd[brd_sq_num];
extern int RanksBrd[brd_sq_num];

extern int PieceKnight[13];
extern int PieceKing[13];
extern int PieceRookQueen[13];
extern int PieceBishopQueen[13];
extern int piece_slides[13];

extern int Mirror64[64];

extern U64 file_bb_mask[8];
extern U64 rank_bb_mask[8];

extern U64 black_passed_mask[64];
extern U64 white_passed_mask[64];
extern U64 isolated_mask[64];

extern s_hashtable hash_table[1];

/* FUNCTIONS */

// init.c
extern void all_init(void);

// bitboards.c
extern int pop_bit(U64* bb);
extern int count_bits(U64 b);

// hashkeys.c
extern U64 generate_pos_key(const s_board* pos);

// board.c
extern void reset_board(s_board* pos);
extern int parse_fen(const char* fen, s_board* pos);
extern void print_board(const s_board* pos);
extern void update_lists_material(s_board* pos);
extern int check_board(const s_board* pos);
extern void mirror_board(s_board* pos);

// attack.c
extern int sq_attacked(int sq, int side, const s_board* pos);

// io.c
extern char* pr_move(int move);
extern char* pr_sq(int sq);
extern void print_move_list(const s_movelist* list);
extern int parse_move(const char* ptr_char, const s_board* pos);

//validate.c
extern int sq_on_board(int sq);
extern int side_valid(int side);
extern int file_rank_valid(int fr);
extern int piece_valid_empty(int pce);
extern int piece_valid(int pce);
extern void mirror_eval_test(s_board* pos);
extern int sq_is120(int sq);
extern int pce_valid_empty_offbrd(int pce);
extern int move_list_ok(const s_movelist* list, const s_board* pos);
extern void debug_analysis_test(s_board* pos, s_searchinfo* info, s_hashtable* table);

// movegen.c
extern void generate_all_moves(const s_board* pos, s_movelist* list);
extern void generate_all_caps(const s_board* pos, s_movelist* list);
extern int move_exists(s_board* pos, int move);
extern void init_mvv_lva(void);

// makemove.c
extern int make_move(s_board* pos, int move);
extern void take_move(s_board* pos);
extern void make_null_move(s_board* pos);
extern void take_null_move(s_board* pos);

// perft.c
extern void perft_test(int depth, s_board* pos);

// search.c
extern void search_position(s_board* pos, s_searchinfo* info, s_hashtable* table);
extern int search_position_thread(void* data);

// misc.c
extern int get_time_ms(void);

// pvtable.c
extern void temp_hash_test(const char* fen);
extern void init_hash_table(s_hashtable* table, int mb);
extern void store_hash_entry(const s_board* pos, s_hashtable* table, int move, int score, int flags, int depth);
extern int probe_hash_entry(const s_board* pos, s_hashtable* table, int* move, int* score, int alpha, int beta,
                            int depth);
extern int probe_pv_move(const s_board* pos, const s_hashtable* table);
extern int get_pv_line(int depth, s_board* pos, const s_hashtable* table);
extern void clear_hash_table(s_hashtable* table);

// evaluate.c
extern int eval_position(const s_board* pos);

// uci.c
extern void uci_loop(s_board* pos, s_searchinfo* info);
extern void parse_go(const char* line, s_searchinfo* info, s_board* pos, s_hashtable* table);
extern void parse_position(const char* line_in, s_board* pos);
extern thrd_t launch_search_thread(s_board* pos, s_searchinfo* info, s_hashtable* table);
extern void join_search_thread(s_searchinfo* info);

