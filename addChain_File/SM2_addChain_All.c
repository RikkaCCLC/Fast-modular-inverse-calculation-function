#include <stdio.h>
#include <stdint.h>
#include <gmp.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#define p "0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF"

// Fermat 模逆算法计算给定值 z 在模 p 下的逆元 t
void fermat_invert(mpz_t t, mpz_t z, mpz_t p_mpz) {
    mpz_t z3, z15, t0, t1, t2, t3, t4, t5;
    mpz_init(z3);
    mpz_init(z15);
    mpz_init(t0);
    mpz_init(t1);
    mpz_init(t2);
    mpz_init(t3);
    mpz_init(t4);
    mpz_init(t5);

    mpz_mul(z3, z, z);
    mpz_mul(z3, z3, z);
    mpz_powm_ui(z15, z3, 4, p_mpz);
    mpz_powm_ui(t0, z15, 4, p_mpz);

    mpz_powm_ui(t1, t0, 64, p_mpz);

    // 循环计算 t1 的 2 的次方
    for (int i = 0; i < 6; ++i) {
        mpz_powm_ui(t1, t1, 2, p_mpz);
    }

    mpz_mul(t1, t1, t0);

    mpz_powm_ui(t2, t1, 2, p_mpz);

    // 循环计算 t2 的 2 的次方
    for (int i = 0; i < 12; ++i) {
        mpz_powm_ui(t2, t2, 2, p_mpz);
    }

    mpz_mul(t2, t2, t1);
    mpz_mul(t2, t2, z15);

    mpz_powm_ui(t3, t2, 2, p_mpz);
    mpz_mul(t3, t3, z3);

    mpz_powm_ui(t4, t3, 2, p_mpz);

    // 循环计算 t4 的 2 的次方
    for (int i = 0; i < 32; ++i) {
        mpz_powm_ui(t4, t4, 2, p_mpz);
    }

    mpz_mul(t4, t4, t3);
    mpz_powm_ui(t4, t4, 96, p_mpz);

    mpz_powm_ui(t5, t4, 32, p_mpz);
    mpz_mul(t5, t5, t3);

    // 循环计算 t5 的 2 的次方
    for (int i = 0; i < 32; ++i) {
        mpz_powm_ui(t5, t5, 2, p_mpz);
    }

    // 循环计算 t5 的 2 的次方
    for (int i = 0; i < 2; ++i) {
        mpz_powm_ui(t5, t5, 2, p_mpz);
    }

    mpz_mul(t, t5, z);
    mpz_sub(t, p_mpz, t);

    // 清理 mpz_t 变量
    mpz_clear(z3);
    mpz_clear(z15);
    mpz_clear(t0);
    mpz_clear(t1);
    mpz_clear(t2);
    mpz_clear(t3);
    mpz_clear(t4);
    mpz_clear(t5);
}


int main() {
    mpz_t p_mpz, z, t;
    mpz_init_set_str(p_mpz, p, 0);
    mpz_init(z);
    mpz_init(t);

    int randomData = open("/dev/urandom", O_RDONLY);
    if (randomData < 0) {
        perror("Error: failed to open /dev/urandom");
        return -1;
    }

    // Start measuring time
    clock_t start = clock();

    int i = 5000;

    // 生成 1 到 p-1 之间的随机数
    gmp_randstate_t rand_state;
    gmp_randinit_default(rand_state);
    gmp_randseed_ui(rand_state, time(NULL));

    while (i--) {
        mpz_urandomm(z, rand_state, p_mpz);
        gmp_printf("z = %Zx\n", z);

        // 计算 z 在模 p 下的逆元
        fermat_invert(t, z, p_mpz);
        gmp_printf("t = %Zx\n\n", t);
    }

    gmp_randclear(rand_state);

    // End measuring time
    clock_t end = clock();
    // Calculate and print time elapsed
    double time_elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Time elapsed: %f seconds\n", time_elapsed);

    close(randomData);
    mpz_clear(p_mpz);
    mpz_clear(z);
    mpz_clear(t);

    return 0;
}

