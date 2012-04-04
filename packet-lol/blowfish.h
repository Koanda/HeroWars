#include "define.h"
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define CIPHER_ALGO_BLOWFISH	 4  /* blowfish 128 bit key */
#define BLOWFISH_BLOCKSIZE 8
#define BLOWFISH_ROUNDS 16

#define ERROR_SELFTEST_FAILED 1
#define ERROR_WEAK_KEY 2


typedef unsigned int u32;
typedef unsigned char byte;

typedef struct {
    u32 s0[256];
    u32 s1[256];
    u32 s2[256];
    u32 s3[256];
    u32 p[BLOWFISH_ROUNDS+2];
} BLOWFISH_context;

int bf_setkey( BLOWFISH_context *c, byte *key, unsigned keylen);
void encrypt_block( BLOWFISH_context *bc, byte *outbuf, byte *inbuf );
void decrypt_block( BLOWFISH_context *bc, byte *outbuf, byte *inbuf );