#ifndef HUFFMAN_HUFFMAN_H
#define HUFFMAN_HUFFMAN_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

int huffman_encode_file(FILE* in, FILE* out);

int huffman_decode_file(FILE* in, FILE* out);

int huffman_encode_memory(const unsigned char* bufin,
			  uint32_t bufinlen,
			  unsigned char** pbufout,
			  uint32_t* pbufoutlen);

int huffman_decode_memory(const unsigned char* bufin,
			  uint32_t bufinlen,
			  unsigned char** bufout,
			  uint32_t* pbufoutlen);

#define MAX_SYMBOLS 256
typedef struct huffman_node_tag
{
	unsigned char isLeaf;
	unsigned long count;
	struct huffman_node_tag* parent;

	union
	{
		struct
		{
			struct huffman_node_tag *zero, *one;
		};
		unsigned char symbol;
	};
} huffman_node;
typedef struct huffman_code_tag
{
	/* The length of this code in bits. */
	unsigned long numbits;

	/* The bits that make up this code. The first
	   bit is at position 0 in bits[0]. The second
	   bit is at position 1 in bits[0]. The eighth
	   bit is at position 7 in bits[0]. The ninth
	   bit is at position 0 in bits[1]. */
	unsigned char* bits;
} huffman_code;
typedef huffman_node* SymbolFrequencies[MAX_SYMBOLS];
typedef huffman_code* SymbolEncoder[MAX_SYMBOLS];

uint32_t get_freq(SymbolFrequencies sf, uint32_t freq[MAX_SYMBOLS]);
int print_table_encoder(SymbolEncoder *se, uint32_t symbol_count);
int print_table_freq(uint32_t freq[MAX_SYMBOLS]);
int print_nice(SymbolFrequencies sf, uint32_t symbol_count);
bool read_code_table(FILE* in, huffman_node** rootOut, unsigned int* dataBytesOut);
int shuffman_encode_memory(const SymbolEncoder *se, const unsigned char* bufin,
			   uint32_t bufinlen, unsigned char** pbufout,
			   uint32_t* pbufoutlen);
int shuffman_decode_memory(huffman_node *root, const unsigned char* bufin,
			   uint32_t bufinlen, unsigned char** pbufout,
			   uint32_t* pbufoutlen);
void build_symbol_encoder(huffman_node* subtree, SymbolEncoder* pSF);
void free_encoder(SymbolEncoder* pSE);
void free_huffman_tree(huffman_node* subtree);

#endif
