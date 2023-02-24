#ifndef inverse256_h
#define inverse256_h

#define inverse256_BTC_p inverse256_skylake_BTC_p
#define inverse256_BTC_n inverse256_skylake_BTC_n
#define inverse256_P256_p inverse256_skylake_P256_p
#define inverse256_P256_n inverse256_skylake_P256_n

extern void inverse256_BTC_p(unsigned char *,const unsigned char *);
extern void inverse256_BTC_n(unsigned char *,const unsigned char *);
extern void inverse256_P256_p(unsigned char *,const unsigned char *);
extern void inverse256_P256_n(unsigned char *,const unsigned char *);

extern unsigned char inverse256_BTC_p_modulus[32];
extern unsigned char inverse256_BTC_n_modulus[32];
extern unsigned char inverse256_P256_p_modulus[32];
extern unsigned char inverse256_P256_n_modulus[32];

#endif
