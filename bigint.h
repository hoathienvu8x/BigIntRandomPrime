#ifndef _BIGINT_H
#define _BIGINT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define BigInt_Size 256
#define BigInt_DigitBits 8
#define BigInt_Base 256

typedef uint8_t bigint[BigInt_Size];

int bigint_equal(bigint a, bigint b);

/* a < b */
int bigint_less(bigint a, bigint b);

/* a <= b */
int bigint_less_or_equal(bigint a, bigint b);

/* a == 0 */
int bigint_is_zero(bigint a);

/* a & 1 == 0 */
int bigint_is_even(bigint a);

/* a & 1 == 1 */
int bigint_is_odd(bigint a);

/* a = 0 */
void bigint_zero(bigint a);

/* a = 1 */
void bigint_one(bigint a);

/* a = value */
void bigint_set(bigint a, size_t value);

/* a = b */
void bigint_set_bigint(bigint a, bigint b);

/* b = b + a */
void bigint_add_bigint(bigint a, bigint b);

/* b = b + a */
void bigint_add_int(uint8_t a, bigint b);

/* b = b - a */
void bigint_sub_bigint(bigint a, bigint b);

void bigint_sub_int(uint8_t a, bigint b);

/* a = a << 8*sh (base 256) */
void bigint_shl(bigint a, size_t sh);

/* a = a >> 8*sh (base 256) */
void bigint_shr(bigint a, size_t sh);

/* a = a << sh */
void bigint_shbl(bigint a, size_t sh);

/* a = a >> sh */
void bigint_shbr(bigint a, size_t sh);

/* b = b * a */
void bigint_mul_int(uint8_t a, bigint b);

/* p = a * b */
void bigint_mul_bigint(bigint a, bigint b, bigint p);

/* b / a = q, b % a = r */
void bigint_div_bigint(bigint a, bigint b, bigint q, bigint r);

/* b % a = r */
void bigint_mod_bigint2(bigint a, bigint b, bigint r);

void bigint_mod_bigint(bigint a, bigint b, bigint r);

/* b % a = r */
void bigint_mod_int(size_t a, bigint b, size_t *r);

/* generate random number a */
void bigint_rand(bigint a);

/* generate random number a < b */
void bigint_rand_range(bigint a, bigint b);

void bigint_rand_prime(bigint a, int nbits);

/* r = a^x mod n */
void bigint_pow_mod(bigint a, bigint x, bigint n, bigint r);

void bigint_print_format(char * pref, bigint x, int printBytes);

void bigint_print(bigint a);

void bigint_set_trials(size_t trials);

int bigint_is_prime(bigint a);

#endif /* _BIGINT_H */
