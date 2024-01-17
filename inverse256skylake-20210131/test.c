#include <gmp.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <linux/perf_event.h>
#include "inverse256.h"

static long long cpucycles(void)
{
  static int rdpmcworks = 1;
  long long result;

  while (rdpmcworks) {
    static int fdperf = -1;
    static struct perf_event_mmap_page *buf = 0;
#ifdef THREADING
    unsigned int seq;
    long long index;
    long long offset;
#endif

    if (fdperf == -1) {
      static struct perf_event_attr attr;
      memset(&attr,0,sizeof attr);
      attr.type = PERF_TYPE_HARDWARE;
      attr.config = PERF_COUNT_HW_CPU_CYCLES;
      attr.exclude_kernel = 1;
      fdperf = syscall(__NR_perf_event_open, &attr, 0, -1, -1, 0);
      if (fdperf == -1) {
        rdpmcworks = 0;
        break;
      }
      buf = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ, MAP_SHARED, fdperf, 0);
      if (buf == 0) {
        rdpmcworks = 0;
        break;
      }
    }

#ifdef THREADING
    do {
      seq = buf->lock;
      asm volatile("" ::: "memory");
      index = buf->index;
      offset = buf->offset;
      asm volatile("rdpmc;shlq $32,%%rdx;orq %%rdx,%%rax"
        : "=a"(result) : "c"(index-1) : "%rdx");
      asm volatile("" ::: "memory");
    } while (buf->lock != seq);

    result += offset;
#else
    asm volatile("rdpmc;shlq $32,%%rdx;orq %%rdx,%%rax"
      : "=a"(result) : "c"(0) : "%rdx");
#endif

    result &= 0xffffffffffff;
    return result;
  }

  asm volatile(".byte 15;.byte 49;shlq $32,%%rdx;orq %%rdx,%%rax"
    : "=a" (result) ::  "%rdx");
  return result;
}

void gmp_import(mpz_t z,const unsigned char *s,unsigned long long slen)
{
  mpz_import(z,slen,-1,1,0,0,s);
}

int gmp_export(unsigned char *s,unsigned long long slen,mpz_t z)
{
  unsigned long long i;
  if (mpz_sizeinbase(z,256) > slen) return -1;
  for (i = 0;i < slen;++i) s[i] = 0;
  mpz_export(s,0,-1,1,0,0,z);
  return 0;
}

unsigned char two256[33] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
  1
} ;

mpz_t two256_gmp;
mpz_t x_gmp;
mpz_t y_gmp;
mpz_t xy_gmp;
mpz_t z_gmp;
mpz_t t_gmp;
mpz_t twop_gmp;

void doit(const unsigned char *x,mpz_t p_gmp,void (*inverse256)(unsigned char *,const unsigned char *))
{
  unsigned char y[32];
  unsigned char z[32];
  
  gmp_import(x_gmp,x,32);

  // gmp_printf("%Zx\n",x_gmp);

  inverse256(y,x);
  gmp_import(y_gmp,y,32);

  assert(mpz_cmp_ui(y_gmp,0) >= 0);
  assert(mpz_cmp(y_gmp,p_gmp) < 0);

  mpz_mul(xy_gmp,x_gmp,y_gmp);
  mpz_mod(xy_gmp,xy_gmp,p_gmp);

  if (mpz_cmp_ui(xy_gmp,1) != 0) {
    assert(mpz_cmp_ui(xy_gmp,0) == 0);
    mpz_mod(xy_gmp,x_gmp,p_gmp);
    assert(mpz_cmp_ui(xy_gmp,0) == 0);
  }

  inverse256(z,y);
  gmp_import(z_gmp,z,32);

  assert(mpz_cmp_ui(z_gmp,0) >= 0);
  assert(mpz_cmp(z_gmp,p_gmp) < 0);

  mpz_mul(xy_gmp,z_gmp,y_gmp);
  mpz_mod(xy_gmp,xy_gmp,p_gmp);

  if (mpz_cmp_ui(xy_gmp,1) != 0) {
    assert(mpz_cmp_ui(xy_gmp,0) == 0);
    mpz_mod(xy_gmp,y_gmp,p_gmp);
    assert(mpz_cmp_ui(xy_gmp,0) == 0);
  }

  mpz_set(z_gmp,x_gmp);
  while (mpz_cmp(z_gmp,p_gmp) >= 0) {
    mpz_sub(z_gmp,z_gmp,p_gmp);
    assert(gmp_export(z,32,z_gmp) == 0);
    inverse256(z,z);
    assert(memcmp(y,z,32) == 0);
  }

  mpz_set(z_gmp,x_gmp);
  for (;;) {
    mpz_add(z_gmp,z_gmp,p_gmp);
    if (mpz_cmp(z_gmp,two256_gmp) >= 0) break;
    assert(gmp_export(z,32,z_gmp) == 0);
    inverse256(z,z);
    assert(memcmp(y,z,32) == 0);
  }
}

long long t[64];
unsigned char x[32];

void bench(void)
{
  long long i,j;
  
  for (i = 0;i < 64;++i)
    t[i] = cpucycles();
  for (i = 63;i > 0;--i)
    t[i] -= t[i-1];

  printf("nothing");
  for (i = 1;i < 64;++i)
    printf(" %lld",t[i]);
  printf("\n");

  for (i = 0;i < 64;++i) {
    t[i] = cpucycles();
    inverse256_BTC_p(x,x);
  }
  for (i = 63;i > 0;--i)
    t[i] -= t[i-1];

  printf("cycles");
  for (i = 1;i < 64;++i)
    printf(" %lld",t[i]);
  printf("\n");

  for (i = 1;i < 64;++i)
    for (j = 1;j < i;++j)
      if (t[i] < t[j]) {
        long long ti = t[i];
        t[i] = t[j];
        t[j] = ti;
      }
  printf("sorted");
  for (i = 1;i < 64;++i)
    printf(" %lld",t[i]);
  printf("\n");
  
  fflush(stdout);
}

#define NUMPRIMES 4
struct {
  const char *name;
  void (*inverse256)(unsigned char *,const unsigned char *);
  unsigned char *modulus;
  mpz_t gmp;
} primes[NUMPRIMES] = {
  { "BTC_p", inverse256_BTC_p, inverse256_BTC_p_modulus },
  { "BTC_n", inverse256_BTC_n, inverse256_BTC_n_modulus },
  { "P256_p", inverse256_P256_p, inverse256_P256_p_modulus },
  { "P256_n", inverse256_P256_n, inverse256_P256_n_modulus },
} ;

int main(int argc, char *argv[])
{
  long long i,j,k;
  long long loop;
  uint64_t tt[2];
  char *tag = "";

  if (argc > 1) tag = argv[1];

  bench();
  
  mpz_init(two256_gmp);
  gmp_import(two256_gmp,two256,33);

  for (k = 0;k < NUMPRIMES;++k) {
    mpz_init(primes[k].gmp);
    gmp_import(primes[k].gmp,primes[k].modulus,32);
    gmp_printf("%smodulus %s = 0x%Zx\n",tag,primes[k].name,primes[k].gmp);
  }

  mpz_init(x_gmp);
  mpz_init(y_gmp);
  mpz_init(xy_gmp);
  mpz_init(z_gmp);
  mpz_init(twop_gmp);

  for (k = 0;k < NUMPRIMES;++k) {
    printf("%s%s checking 2000 integers near modulus\n",tag,primes[k].name);
    for (i = -1000;i < 1000;++i) {
      mpz_set_si(x_gmp,i);
      mpz_add(x_gmp,x_gmp,primes[k].gmp);
      mpz_mod(x_gmp,x_gmp,primes[k].gmp);
      assert(gmp_export(x,32,x_gmp) == 0);
      doit(x,primes[k].gmp,primes[k].inverse256);
    }
  }

  for (k = 0;k < NUMPRIMES;++k) {
    printf("%s%s checking 1000 integers near 2^256\n",tag,primes[k].name);
    for (i = -1000;i < 0; ++i) {
      mpz_set_si(x_gmp,i);
      mpz_add(x_gmp,x_gmp,two256_gmp);
      assert(gmp_export(x,32,x_gmp) == 0);
      doit(x,primes[k].gmp,primes[k].inverse256);
    }
  }

  for (k = 0;k < NUMPRIMES;++k) {
    printf("%s%s checking 2000 integers times 256 powers of 2\n",tag,primes[k].name);
    for (j=0; j<256; j++) {
      mpz_ui_pow_ui(twop_gmp,2,j);
      for (i = -1000;i < 1000;++i) {
        mpz_set_si(x_gmp,i);
        mpz_mul(x_gmp,x_gmp,twop_gmp);
        mpz_mod(x_gmp,x_gmp,primes[k].gmp);
        assert(gmp_export(x,32,x_gmp) == 0);
        doit(x,primes[k].gmp,primes[k].inverse256);
      }
    }
  }
  
  for (int a = 63; a >= 10; a--) {
    for (k = 0;k < NUMPRIMES;++k) {
      printf("%s%s checking low Hamming weight around 2^%d\n",tag,primes[k].name,a);
      for (int b = a-1; b >= 2; b--) {
        for (int s_b = -1; s_b <= 1; s_b+=2) {
          for (int c = b-1; c >= 1; c--) {
            for (int s_c = -1; s_c <= 1; s_c+=2) {
              for (int d = c-1; d >= 0; d--) {
                for (int s_d = -1; s_d <= 1; s_d+=2) {
                  tt[0] = (1ULL<<a)+s_b*(1ULL<<b)+s_c*(1ULL<<c)+s_d*(1ULL<<d);
                  tt[1] = 0; gmp_import(x_gmp,(void*)tt,16);
                  mpz_mod(x_gmp,x_gmp,primes[k].gmp);
                  gmp_export(x,32,x_gmp);
                  doit(x,primes[k].gmp,primes[k].inverse256);
                  mpz_neg(x_gmp,x_gmp);
                  mpz_mod(x_gmp,x_gmp,primes[k].gmp);
                  gmp_export(x,32,x_gmp);
                  doit(x,primes[k].gmp,primes[k].inverse256);
                }
              }
            }
          }
        }
      }
    }
  }

  printf("%schecking random integers from stdin\n",tag);
  for (loop = 0;;++loop) {
    if (!(loop&(loop-1))) {
      printf("%sloop %lld\n",tag,loop);
      fflush(stdout);
    }
    for (i = 0;i < 32;++i)
      x[i] = getchar();
    for (k = 0;k < NUMPRIMES;++k)
      doit(x,primes[k].gmp,primes[k].inverse256);
  }
}
