#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <gmp.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define BITS 256

void fermat_inversion(mpz_t t, const mpz_t z, const mpz_t p) {
    mpz_t z3, z15, t0, t1, t2, t3, t4, t5, temp;
    mpz_inits(z3, z15, t0, t1, t2, t3, t4, t5, temp, NULL);

    // z3 = z^2 * z
    mpz_pow_ui(z3, z, 2);
    mpz_mul(z3, z3, z);

    // z15 = z3^(2^2) * z3
    mpz_powm_ui(z15, z3, 4, p);
    mpz_mul(z15, z15, z3);

    // t0 = z15^(2^2) * z3
    mpz_powm_ui(t0, z15, 4, p);
    mpz_mul(t0, t0, z3);

    // t1 = t0^(2^6) * t0
    mpz_powm_ui(t1, t0, 64, p);
    mpz_mul(t1, t1, t0);

    // t2 = ((t1^(2^12) * t1)^(2^6)) * t0
    mpz_powm_ui(temp, t1, 4096, p);
    mpz_mul(temp, temp, t1);
    mpz_powm_ui(t2, temp, 64, p);
    mpz_mul(t2, t2, t0);

    // t3 = t2^2 * z3
    mpz_powm_ui(t3, t2, 2, p);
    mpz_mul(t3, t3, z3);

    // t4 = ((t3^(2^32) * z)^(2^96))
    mpz_powm_ui(temp, t3, 4294967296, p);
    mpz_mul(temp, temp, z);
    mpz_powm_ui(t4, temp, 2, p);
    for (int i = 0; i < 96; ++i) {
        mpz_powm_ui(t4, t4, 2, p);
    }

    // t5 = (t4^(2^32) * t3)^(2^32) * t3
    mpz_powm_ui(temp, t4, 4294967296, p);
    mpz_mul(temp, temp, t3);
    mpz_powm_ui(t5, temp, 4294967296, p);
    mpz_mul(t5, t5, t3);
    for (int i = 0; i < 32; ++i) {
        mpz_powm_ui(t5, t5, 2, p);
    }

    // t = (t5^(2^30) * t2)^(2^2) * z
    mpz_powm_ui(temp, t5, 1073741824, p);
    mpz_mul(temp, temp, t2);
    mpz_powm_ui(t, temp, 4, p);
    mpz_mul(t, t, z);

    mpz_clears(z3, z15, t0, t1, t2, t3, t4, t5, temp, NULL);
}

int main() {
    mpz_t z, t, p;
    unsigned char buf[BITS/8];
    int urandom = open("/dev/urandom", O_RDONLY);

    // Initialize GMP integers
    mpz_init(z);
    mpz_init(t);
    mpz_init_set_str(p, "115792089210356248762697446949407573529996955224135760342422259061068512044369", 10);

    // Read random number from /dev/random
    read(urandom, buf, BITS/8);
    mpz_import(z, BITS/8, 1, 1, 0, 0, buf);
    while(mpz_cmp_ui(z, 0) == 0 || mpz_cmp(z, p) >= 0) {
        read(urandom, buf, BITS/8);
        mpz_import(z, BITS/8, 1, 1, 0, 0, buf);
    }

    fermat_inversion(t, z, p);

    // Print result
    gmp_printf("z = %Zd\n", z);
    gmp_printf("t = %Zd\n", t);

    // Clean up
    mpz_clear(z);
    mpz_clear(t);
    mpz_clear(p);
    close(urandom);
    return 0;
}
