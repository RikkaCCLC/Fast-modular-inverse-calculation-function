#include <stdint.h>
#include "inverse256.h"

extern void inverse256_skylake_asm(const unsigned char *,unsigned char *,const int64_t *);

/* To set up the table for an arbitrary prime of size 256 or less 
   Copy the first 20 entries of the table verbatim, these are the
   constants 2^30-1, 2^33, 2^63, 2^63-2^33, and 2^29;

   compute the prime radix 2^64, and enter in positions 20..23.

   compute the prime radix 2^30, and enter the 9 limbs in positions
   24, 28, ... 56; finally, replace position 60 with 1/p mod 2^30.

   Note: the prime expansion needs all limbs between 0 and 2^30.

   We can write a specific multiplication and reduction routine for
   the Bitcoin prime because it is so sparse, but the difference in
   the timings for the 25519 prime and for this routine is about 5%
   so we have decided against it. */

/* This is the Bitcoin prime */
static const __attribute__((aligned(32)))
int64_t t_BTC_p[64]={
    0x3FFFFFFFLL, 0x3FFFFFFFLL, 0x3FFFFFFFLL, 0x3FFFFFFFLL,
    0x200000000LL, 0x200000000LL, 0x200000000LL, 0x200000000LL,
    0x8000000000000000LL, 0x8000000000000000LL,
    0x8000000000000000LL, 0x8000000000000000LL,
    0X7FFFFFFE00000000LL, 0X7FFFFFFE00000000LL,
    0X7FFFFFFE00000000LL, 0X7FFFFFFE00000000LL,
    0x20000000LL, 0x20000000LL, 0x20000000LL, 0x20000000LL,
    0xfffffffefffffc2fULL, 0xffffffffffffffffULL,
    0xffffffffffffffffULL, 0xffffffffffffffffULL,
    0x03ffffc2fLL, 0LL, 0LL, 0LL,
    0x03ffffffbLL, 0LL, 0LL, 1LL,
    0x03fffffffLL, 0LL, 0LL, 0LL,
    0x03fffffffLL, 0LL, 0LL, 0LL,
    0x03fffffffLL, 0LL, 0LL, 0LL,
    0x03fffffffLL, 0LL, 0LL, 0LL,
    0x03fffffffLL, 0LL, 0LL, 0LL,
    0x03fffffffLL, 0LL, 0LL, 0LL,
    0x00000ffffLL, 0LL, 0LL, 0LL,
    0xd838091dd2253531ULL, 0LL, 0LL, 0LL};

unsigned char inverse256_BTC_p_modulus[32] = {
  0x2f,0xfc,0xff,0xff,0xfe,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
} ;

void inverse256_BTC_p(unsigned char *out,const unsigned char *in)
{
  inverse256_skylake_asm(in,out,t_BTC_p);
}

/* This is the Bitcoin curve order prime */
static const __attribute__((aligned(32)))
int64_t t_BTC_n[64]={
    0x3FFFFFFFLL, 0x3FFFFFFFLL, 0x3FFFFFFFLL, 0x3FFFFFFFLL,
    0x200000000LL, 0x200000000LL, 0x200000000LL, 0x200000000LL,
    0x8000000000000000LL, 0x8000000000000000LL,
    0x8000000000000000LL, 0x8000000000000000LL,
    0X7FFFFFFE00000000LL, 0X7FFFFFFE00000000LL,
    0X7FFFFFFE00000000LL, 0X7FFFFFFE00000000LL,
    0x20000000LL, 0x20000000LL, 0x20000000LL, 0x20000000LL,
    0xbfd25e8cd0364141ULL, 0xbaaedce6af48a03bULL,
    0xfffffffffffffffeULL, 0xffffffffffffffffULL,
    0x010364141LL, 0LL, 0LL, 0LL,
    0x03f497a33LL, 0LL, 0LL, 1LL,
    0x0348a03bbLL, 0LL, 0LL, 0LL,
    0x02bb739abLL, 0LL, 0LL, 0LL,
    0x03ffffebaLL, 0LL, 0LL, 0LL,
    0x03fffffffLL, 0LL, 0LL, 0LL,
    0x03fffffffLL, 0LL, 0LL, 0LL,
    0x03fffffffLL, 0LL, 0LL, 0LL,
    0x00000ffffLL, 0LL, 0LL, 0LL,
    0x4b0dff665588b13fULL, 0LL, 0LL, 0LL};

unsigned char inverse256_BTC_n_modulus[32] = {
  0x41,0x41,0x36,0xd0,0x8c,0x5e,0xd2,0xbf,
  0x3b,0xa0,0x48,0xaf,0xe6,0xdc,0xae,0xba,
  0xfe,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
} ;

void inverse256_BTC_n(unsigned char *out,const unsigned char *in)
{
  inverse256_skylake_asm(in,out,t_BTC_n);
}

/* This is the P-256 curve order prime */
static const __attribute__((aligned(32)))
int64_t t_P256_n[64]={
    0x3FFFFFFFLL, 0x3FFFFFFFLL, 0x3FFFFFFFLL, 0x3FFFFFFFLL,
    0x200000000LL, 0x200000000LL, 0x200000000LL, 0x200000000LL,
    0x8000000000000000LL, 0x8000000000000000LL,
    0x8000000000000000LL, 0x8000000000000000LL,
    0X7FFFFFFE00000000LL, 0X7FFFFFFE00000000LL,
    0X7FFFFFFE00000000LL, 0X7FFFFFFE00000000LL,
    0x20000000LL, 0x20000000LL, 0x20000000LL, 0x20000000LL,
    0xf3b9cac2fc632551ULL, 0xbce6faada7179e84ULL,
    0xffffffffffffffffULL, 0xffffffff00000000ULL,
    0x03c632551LL, 0LL, 0LL, 0LL,
    0x00ee72b0bLL, 0LL, 0LL, 1LL,
    0x03179e84fLL, 0LL, 0LL, 0LL,
    0x039beab69LL, 0LL, 0LL, 0LL,
    0x03fffffbcLL, 0LL, 0LL, 0LL,
    0x03fffffffLL, 0LL, 0LL, 0LL,
    0x000000fffLL, 0LL, 0LL, 0LL,
    0x03fffc000LL, 0LL, 0LL, 0LL,
    0x00000ffffLL, 0LL, 0LL, 0LL,
    0xccd1c8aaee00bc4fULL, 0LL, 0LL, 0LL};

unsigned char inverse256_P256_n_modulus[32] = {
  0x51,0x25,0x63,0xfc,0xc2,0xca,0xb9,0xf3,
  0x84,0x9e,0x17,0xa7,0xad,0xfa,0xe6,0xbc,
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0x00,0x00,0x00,0x00,0xff,0xff,0xff,0xff,
} ;

void inverse256_P256_n(unsigned char *out,const unsigned char *in)
{
  inverse256_skylake_asm(in,out,t_P256_n);
}

/* This is the P-256 curve Solinas prime */
static const __attribute__((aligned(32)))
int64_t t_P256_p[64]={
    0x3FFFFFFFLL, 0x3FFFFFFFLL, 0x3FFFFFFFLL, 0x3FFFFFFFLL,
    0x200000000LL, 0x200000000LL, 0x200000000LL, 0x200000000LL,
    0x8000000000000000LL, 0x8000000000000000LL,
    0x8000000000000000LL, 0x8000000000000000LL,
    0X7FFFFFFE00000000LL, 0X7FFFFFFE00000000LL,
    0X7FFFFFFE00000000LL, 0X7FFFFFFE00000000LL,
    0x20000000LL, 0x20000000LL, 0x20000000LL, 0x20000000LL,
    0xffffffffffffffffULL, 0x00000000ffffffffULL,
    0x0000000000000000ULL, 0xffffffff00000001ULL,
    0x03fffffffLL, 0LL, 0LL, 0LL,
    0x03fffffffLL, 0LL, 0LL, 1LL,
    0x03fffffffLL, 0LL, 0LL, 0LL,
    0x00000003fLL, 0LL, 0LL, 0LL,
    0x000000000LL, 0LL, 0LL, 0LL,
    0x000000000LL, 0LL, 0LL, 0LL,
    0x000001000LL, 0LL, 0LL, 0LL,
    0x03fffc000LL, 0LL, 0LL, 0LL,
    0x00000ffffLL, 0LL, 0LL, 0LL,
    0x0000000000000001ULL, 0LL, 0LL, 0LL};

unsigned char inverse256_P256_p_modulus[32] = {
  0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
  0xff,0xff,0xff,0xff,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x01,0x00,0x00,0x00,0xff,0xff,0xff,0xff,
} ;

void inverse256_P256_p(unsigned char *out,const unsigned char *in)
{
  inverse256_skylake_asm(in,out,t_P256_p);
}
