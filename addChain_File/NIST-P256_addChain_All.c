#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <gmp.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#define BITS 256
#define P_HEX "ffffffff00000001000000000000000000000000ffffffffffffffffffffffff"

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
    for (int i = 0; i < 6; ++i) {
        mpz_powm_ui(t1, t0, 2, p);
    }
    mpz_mul(t1, t1, t0);

    // t2 = ((t1^(2^12) * t1)^(2^6)) * t0
    for (int i = 0; i < 12; ++i) {
        mpz_powm_ui(temp, t1, 2, p);
    }
    mpz_mul(temp, temp, t1);
    mpz_powm_ui(t2, temp, 64, p);
    mpz_mul(t2, t2, t0);

    // t3 = t2^2 * z3
    mpz_powm_ui(t3, t2, 2, p);
    mpz_mul(t3, t3, z3);

    // t4 = ((t3^(2^32) * z)^(2^96))
    for (int i = 0; i < 32; ++i) {
        mpz_powm_ui(temp, t3, 2, p);
    }

    mpz_mul(temp, temp, z);
    mpz_powm_ui(t4, temp, 2, p);
    for (int i = 0; i < 96; ++i) {
        mpz_powm_ui(t4, t4, 2, p);
    }

    // t5 = (t4^(2^32) * t3)^(2^32) * t3
    for (int i = 0; i < 32; ++i) {
        mpz_powm_ui(temp, t4, 2, p);
    }
    mpz_mul(temp, temp, t3);
    for (int i = 0; i < 96; ++i) {
        mpz_powm_ui(t5, temp, 2, p);
    }
    mpz_mul(t5, t5, t3);
    for (int i = 0; i < 32; ++i) {
        mpz_powm_ui(t5, t5, 2, p);
    }

    // t = (t5^(2^30) * t2)^(2^2) * z
    for (int i = 0; i < 30; ++i) {
        mpz_powm_ui(temp, t5, 2, p);
    }

    mpz_mul(temp, temp, t2);
    mpz_powm_ui(t, temp, 4, p);
    mpz_mul(t, t, z);

      // Print the result in hexadecimal format
    gmp_printf("z = %ZX\n", z);
    gmp_printf("t = %ZX\n\n", t);
}

int main() {
    // Initialize GMP variables
    mpz_t p, z, t;
    mpz_init(p);
    mpz_init(z);
    mpz_init(t);

    // Initialize random seed
    gmp_randstate_t state;
    gmp_randinit_default(state);
    gmp_randseed_ui(state, time(NULL));

    // Read p from a string
    mpz_set_str(p, P_HEX, 16);

    // Initialize buffer for reading from /dev/random
    unsigned char buf[BITS/8];

    // Start measuring time
    clock_t start = clock();

    int i = 5000;

    // Enter infinite loop to read z from /dev/random and calculate Fermat-based inversion
    while (i -- ) {
        // Read random number from /dev/random
        FILE* urandom = fopen("/dev/random", "r");
        fread(buf, 1, BITS/8, urandom);
        fclose(urandom);
        mpz_import(z, BITS/8, 1, 1, 0, 0, buf);
        while (mpz_cmp_ui(z, 0) == 0 || mpz_cmp(z, p) >= 0) {
            FILE* urandom = fopen("/dev/random", "r");
            fread(buf, 1, BITS/8, urandom);
            fclose(urandom);
            mpz_import(z, BITS/8, 1, 1, 0, 0, buf);
        }

        // Calculate Fermat-based inversion
        fermat_inversion(t, z, p);

        // Print the result in hexadecimal format
        gmp_printf("z = %ZX\n", z);
        gmp_printf("t = %ZX\n\n", t);
    }

    // End measuring time
    clock_t end = clock();

    // Calculate and print time elapsed
    double time_elapsed = (double) (end - start) / CLOCKS_PER_SEC;
    printf("Time elapsed: %f seconds\n", time_elapsed);

    // Clean up GMP variables
    mpz_clear(p);
    mpz_clear(z);
    mpz_clear(t);

    return 0;
}
