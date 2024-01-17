 #include <stdio.h>
#include <stdint.h>
#include <gmp.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define p "0xFFFFFFFEFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF00000000FFFFFFFFFFFFFFFF"

//Fermat-based inversion for SM2,
//use the “addition chain” to minimize the number of multiplications in the iteration of inversion computation

// Fermat 模逆算法计算给定值 z 在模 p 下的逆元 t
void fermat_invert(mpz_t t, mpz_t z, mpz_t p_mpz) {

    // 初始化多个 mpz_t 变量
    mpz_t z3, z15, t0, t1, t2, t3, t4, t5;
    mpz_init(z3);
    mpz_init(z15);
    mpz_init(t0);
    mpz_init(t1);
    mpz_init(t2);
    mpz_init(t3);
    mpz_init(t4);
    mpz_init(t5);


     // 计算 z^3 和 z^15
    mpz_mul(z3, z, z);
    mpz_mul(z3, z3, z);
    mpz_powm_ui(z15, z3, 4, p_mpz);
    mpz_powm_ui(t0, z15, 4, p_mpz);


    // 迭代计算 t0 到 t5
    mpz_powm_ui(t1, t0, 64, p_mpz);
    mpz_powm_ui(t2, t1, 4096, p_mpz);
    mpz_mul(t2, t2, t1);
    mpz_powm_ui(t2, t2, 64, p_mpz);
    mpz_mul(t3, t2, t2);
    mpz_mul(t3, t3, z);
    mpz_powm_ui(t4, t3, 4294967296, p_mpz);
    mpz_mul(t4, t4, t3);
    mpz_powm_ui(t4, t4, 8, p_mpz);
    mpz_mul(t4, t4, z15);
    mpz_powm_ui(t5, t4, 4294967296, p_mpz);
    mpz_mul(t5, t5, t3);
    mpz_powm_ui(t5, t5, 2147483648, p_mpz);
    mpz_mul(t5, t5, t3);
    mpz_powm_ui(t5, t5, 2147483648, p_mpz);
    mpz_mul(t5, t5, t3);
    mpz_powm_ui(t5, t5, 16, p_mpz);
    mpz_mul(t, t5, z);
    mpz_sub(t, p_mpz, t);

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

    int randomData = open("/dev/random", O_RDONLY);
    if (randomData < 0) {
        perror("Error: failed to open /dev/random");
        return -1;
    }
    unsigned char randChar;
    int bytesRead = 0;
    while (bytesRead < sizeof(randChar)) {
        int result = read(randomData, &randChar + bytesRead, sizeof(randChar) - bytesRead);
        if (result < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("Error: failed to read from /dev/random");
            return -1;
        }
        bytesRead += result;
    }
    close(randomData);

    mpz_import(z, 1, -1, sizeof(randChar), 0, 0, &randChar);
gmp_printf("z = %Zx\n", z);

fermat_invert(t, z, p_mpz);
gmp_printf("t = %Zx\n", t);

mpz_clear(p_mpz);
mpz_clear(z);
mpz_clear(t);

return 0;
}
