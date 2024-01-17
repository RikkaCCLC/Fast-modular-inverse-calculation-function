## Format-based inversion for SM2 && NIST

代码按照论文: ***Lightweight Implementations of NIST P-256 and SM2 ECC on 8-bit Resource-Constraint Embedded Device*** 给出的算法步骤进行编写.

### 1. 论文中算法描述:

### SM2

<img src="https://github.com/Mozartto/Extended-Euclidean-algorithm-and-fast-modular-inversion/assets/58902267/a86b8e6e-ce8c-44ed-a064-dfb37d8b1797" alt="image" style="zoom:50%;" />




### NIST P-256
<img src="https://github.com/Mozartto/Extended-Euclidean-algorithm-and-fast-modular-inversion/assets/58902267/f76c1ff5-7369-4e44-ba01-4b16bad53c1a" alt="image" style="zoom: 50%;" />


上述算法使用了加法链的思想.



### 2.代码编写

编写测试了四个代码文件, 分别是

1. P256_addChain_single.c
2. P256_addChain_All.c
3. SM2_addChain_single.c
4. SM2_addChain_All.c



#### 代码说明

以 `NIST-P256_addChain_All.c`为例:

`void format_inversion() `有三个传入参数:` mpz_t t,const mpz_t z, const mpz_t p`

p为模数, z为系统随机输入的大小在1~p-1之间的数, t 用于存放计算结果

```c
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
```

高次项使用循环来计算

其中z3, z15, t ~ t5分别与论文中的同名参数对应.

`main` 函数

```c
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

    // Enter loop for 5000 times to read z from /dev/random and calculate Fermat-based inversion
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

```

` #include <time.h> `添加c语言`time`函数库库

`clock_t start = clock(); `处为代码计时开始时间

`clock_t end = clock();`处为代码计时结束时间

` double time_elapsed = (double) (end - start) / CLOCKS_PER_SEC;`记录了5000次模逆所消耗的总时间。

main函数中的while循环首先打开系统random熵池进行读取随机数，然后检查参数z，p的正确性，并调用  `fermat_inversion(t, z, p);`函数进行模逆计算，其中t为存放结果的参数，z为随机数，p为模数。



`fread`函数用于读取随机数熵池中的二进制文件，这里用于读取z:

```c
size_t fread( void *buffer, size_t size, size_t count, FILE *stream );

void *buffer 参数 : 将文件中的二进制数据读取到该缓冲区中 ;

size_t size 参数 : 读取的 基本单元 字节大小 , 单位是字节 , 一般是 buffer 缓冲的单位大小 ;

如果 buffer 缓冲区是 char 数组 , 则该参数的值是 sizeof(char) ;
如果 buffer 缓冲区是 int 数组 , 则该参数的值是 sizeof(int) ;
size_t count 参数 : 读取的 基本单元 个数 ;

FILE *stream 参数 : 文件指针 ;

size_t 返回值 : 实际从文件中读取的 基本单元 个数 ; 读取的字节数是 基本单元数 * 基本单元字节大小 ;
```

这里没有使用返回值`size_t`，使用了`mpz_import`来为z进行赋值。

`urandom`为上述fread参数列表中的`FILE *stream`类型参数，可以理解为随机数生成池

代码测试5000次模逆计算所需的时间，CPU主频1.6GHZ，使用控制台输出的时间乘以1.6×10^9 再除以5000即可得到单次模逆所需的时钟周期数。
