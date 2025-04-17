#include "bigint.h"

#define bigint_min(a, b) (a < b) ? a : b
#define bigint_max(a, b) (a > b) ? a : b

static bigint bigint_cache[BigInt_Base+1];
static int bigint_cache_empty = 1;
static bigint bigint_lastnum;
static size_t bigint_trials = 256;

void bigint_set_trials(size_t trials) {
  bigint_trials = trials == 0 ? 3 : trials;
}

int bigint_equal(bigint a, bigint b) {
  size_t i = 0;
  for (i = 0; i < BigInt_Size; i++) {
    if (a[i] != b[i]) {
      return 0;
    }
  }
  return 1;
}
/* a < b */
int bigint_less(bigint a, bigint b) {
  size_t i = 0;
  for (i = BigInt_Size - 1; ; i--) {
    if (a[i] < b[i]) return 1;
    if (a[i] > b[i]) return 0;
    if (i == 0) break;
  }
  return 0;
}
/* a <= b */
int bigint_less_or_equal(bigint a, bigint b) {
  size_t i=0;
  for (i = BigInt_Size - 1; ; i--) {
    if (a[i] < b[i]) return 1;
    if (a[i] > b[i]) return 0;
    if (i == 0) break;
  }
  return 1;
}
/* a == 0 */
int bigint_is_zero(bigint a) {
  size_t i = 0;
  for (i = 0; i < BigInt_Size; ++i) {
    if (a[i] != 0) return 0;
  }
  return 1;
}
/* a & 1 == 0 */
int bigint_is_even(bigint a) {
  return (a[0] & 1) == 0;
}
/* a & 1 == 1 */
int bigint_is_odd(bigint a) {
  return (a[0] & 1) == 1;
}
/* a = 0 */
void bigint_zero(bigint a) {
  memset(a, 0, BigInt_Size);
}
/* a = 1 */
void bigint_one(bigint a) {
  bigint_zero(a);
  a[0] = 1;
}
/* a = value */
void bigint_set(bigint a, size_t value) {
  size_t i = 0;
  bigint_zero(a);
  while (value > 0 && i < BigInt_Size) {
    a[i] = value % BigInt_Base;
    value /= BigInt_Base;
    i++;
  }
}
/* a = b */
void bigint_set_bigint(bigint a, bigint b) {
  bigint_zero(a);
  memcpy(a, b, BigInt_Size);
}
/* b = b + a */
void bigint_add_bigint( bigint a, bigint b) {
  uint16_t x = 0;
  size_t i = 0;
  for (i = 0; i < BigInt_Size; i++) {
    x = (uint16_t) b[i] + a[i] + x;
    b[i] = x % BigInt_Base;
    x /= BigInt_Base;
  }
}
/* b = b + a */
void bigint_add_int(uint8_t a, bigint b) {
  uint16_t x = a;
  size_t i = 0;
  for (i = 0; i < BigInt_Size; i++) {
    x = (uint16_t) b[i] + x;
    b[i] = x % BigInt_Base;
    x /= BigInt_Base;
    if (x == 0) break;
  }
}
/* b = b - a */
void bigint_sub_bigint( bigint a, bigint b) {
  uint16_t r = 0;
  size_t i = 0;
  for (i = 0; i < BigInt_Size; i++) {
    if (b[i] < a[i] + r) {
      b[i] = r = BigInt_Base + b[i] - a[i] - r;
      r = 1;
    } else {
      b[i] -= r + a[i];
      r = 0;
    }
  }
}
void bigint_sub_int( uint8_t a, bigint b) {
  uint16_t r = a;
  size_t i = 0;
  for (i = 0; i < BigInt_Size; i++) {
    if (b[i] < r) {
      b[i] = r = BigInt_Base + b[i] - r;
      r = 1;
    } else {
      b[i] -= r;
      r = 0;
    }
  }
}
/* a = a << 8*sh (base 256) */
void bigint_shl(bigint a, size_t sh) {
  size_t i = 0;
  if (sh == 0) return;
  sh = bigint_min(BigInt_Size, sh);
  for (i = BigInt_Size - 1; i >= sh; i--) {
    a[i] = a[i - sh];
  }

  for (i = 0; i < sh; i++) {
    a[i] = 0;
  }
}
/* a = a >> 8*sh (base 256) */
void bigint_shr(bigint a, size_t sh) {
  size_t i = 0;
  if (sh == 0) return;
  sh = bigint_min(BigInt_Size, sh);
  for (i = 0; i < BigInt_Size - sh; i++) {
    a[i] = a[i + sh];
  }

  for (i = 0; i < sh; i++) {
    a[BigInt_Size - 1 - i] = 0;
  }
}
/* a = a << sh */
void bigint_shbl(bigint a, size_t sh) {
  uint16_t mask = BigInt_Base - (1 << (BigInt_DigitBits - sh));
  uint8_t bits0 = 0, bits1 = 0;
  size_t i = 0;

  if (sh == 0) return;

  bigint_shl(a, sh / BigInt_DigitBits);
  sh = sh % BigInt_DigitBits;

  for (i = 0; i < BigInt_Size; ++i) {
    bits1 = a[i] & mask;
    a[i] <<= sh;
    a[i] |= bits0 >> (BigInt_DigitBits - sh);
    bits0 = bits1;
  }
}
/* a = a >> sh */
void bigint_shbr(bigint a, size_t sh) {
  uint16_t mask = (1 << sh) - 1;
  uint8_t bits0 = 0, bits1 = 0;
  size_t i = 0;

  if (sh == 0) return;

  bigint_shr(a, sh / BigInt_DigitBits);
  sh = sh % BigInt_DigitBits;

  for (i = BigInt_Size - 1;; --i) {
    bits1 = a[i] & mask;
    a[i] >>= sh;
    a[i] |= bits0 << (BigInt_DigitBits - sh);
    bits0 = bits1;
    if (i == 0) break;
  }
}
/* b = b * a */
void bigint_mul_int( uint8_t a, bigint b) {
  uint16_t r = 0;
  size_t i = 0;
  for (i = 0; i < BigInt_Size; ++i) {
    r += (uint16_t) a * b[i];
    b[i] = r % BigInt_Base;
    r /= BigInt_Base;
  }
}
/* p = a * b */
void bigint_mul_bigint( bigint a,  bigint b, bigint p) {
  bigint t;
  size_t i = 0;

  bigint_zero(p);
  for (i = 0; i < BigInt_Size; ++i) {
    memcpy(t, b, BigInt_Size);
    bigint_mul_int(a[i], t);
    bigint_shl(t, i);
    bigint_add_bigint(t, p);
  }
}
/* b / a = q, b % a = r */
void bigint_div_bigint( bigint a,  bigint b, bigint q, bigint r) {
  bigint t;
  size_t i = 0;
  uint8_t k = 0;

  bigint_zero(q);
  bigint_zero(r);
  bigint_zero(t);

  for (i = BigInt_Size - 1;; --i) {
    bigint_shl(r, 1);
    bigint_add_int(b[i], r);
    if (bigint_less_or_equal(a, r)) {
      bigint_zero(t);
      k = 0;
      do {
        ++k;
        bigint_add_bigint(a, t);
      } while (bigint_less_or_equal(t, r));

      q[i] = k - 1;
      bigint_sub_bigint(a, t);
      bigint_sub_bigint(t, r);
    } else {
      q[i] = 0;
    }
    if (i == 0) break;
  }
}

/* b % a = r */
void bigint_mod_bigint2( bigint a,  bigint b, bigint r) {
  bigint q;
  bigint_div_bigint(a,b,q,r);
}
void bigint_mod_bigint(bigint a, bigint b, bigint r) {
  bigint t;
  size_t i, k;
  int k0, k1, m;

  if (bigint_less(b, a)) {
    bigint_set_bigint(r,b);
    return;
  }

  bigint_zero(r);

  bigint_set_bigint(bigint_lastnum, a);
  if (bigint_cache_empty || bigint_equal(a, bigint_lastnum) == 0) {
    bigint_zero(t);

    for (k = 0; k <= BigInt_Base; k++) {
      bigint_set_bigint(bigint_cache[k], t);
      bigint_add_bigint(a, t);
    }
    bigint_set_bigint(bigint_lastnum, a);
  }

  for (i = BigInt_Size - 1; ; --i) {
    bigint_shl(r,1);
    bigint_add_int(b[i], r);

    if (bigint_less_or_equal(a, r)) {
      if (bigint_less_or_equal(bigint_cache[BigInt_Base - 1], r)) {
        bigint_sub_bigint(bigint_cache[BigInt_Base - 1], r);
      } else {
        k0 = 0;
        k1 = BigInt_Base;
        while (1) {
          m = (k0 + k1) / 2;
          if (bigint_less_or_equal(bigint_cache[m], r)) {
            k0 = m;
          } else {
            k1 = m;
          }
          if (k1 == k0 + 1) break;
        }
        bigint_sub_bigint(bigint_cache[k0], r);
      }
    }
    if (i == 0) break;
  }
}
/* b % a = r */
void bigint_mod_int(size_t a,  bigint b, size_t *r) {
  size_t i = 0;
  *r = 0;
  for (i = BigInt_Size - 1;; --i) {
    *r = (*r * BigInt_Base + b[i]) % a;
    if (i == 0) break;
  }
}

/* generate random number a */
void _bigint_rand(bigint a, int prime) {
  size_t i = 0;
  srand(time(0));
  for (i = 0; i < BigInt_Size; i++) {
    if (i == 0) {
      a[i] = (rand() % 9) + '1'; /* No leading zeros */
    } else {
      a[i] = rand() % BigInt_Base;
    }
  }
  if (prime) {
    a[BigInt_Size - 1] = (rand() % 5) * 2 + '1'; /* Last digit is odd */
  }
}
void bigint_rand(bigint a) {
  _bigint_rand(a, 0);
}
/* generate random number a < b */
void _bigint_rand_range(bigint a,  bigint b, int prime) {
  bigint t, q;

  _bigint_rand(a, prime);
  bigint_set_bigint(t, a);
  bigint_div_bigint(b, t, q, a);
}
void bigint_rand_range(bigint a,  bigint b) {
  _bigint_rand_range(a, b, 0);
}
void bigint_rand_prime(bigint a, int nbits) {
  static int smallPrimes[] = {
      2,   3,   5,   7,  11,  13,  17,  19,  23,  29,  31,  37,  41,  43,
     47,  53,  59,  61,  67,  71,  73,  79,  83,  89,  97, 101, 103, 107,
    109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181,
    191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263,
    269, 271, 277, 281, 283, 293, 307, 311, 313, 317, 331, 337, 347, 349,
    353, 359, 367, 373, 379, 383, 389, 397, 401, 409, 419, 421, 431, 433,
    439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503, 509, 521,
    523, 541,  -1
  };
  int i, do_fast = 0;
  bigint n_lo, n_hi, n;
  size_t p, r, pmod[541];

  srand(time(0));

  bigint_one(n_lo);
  bigint_one(n_hi);

  bigint_shbl(n_lo, nbits - 1);
  bigint_shbl(n_hi, nbits);

  bigint_sub_bigint(n_lo, n_hi);

  bigint_zero(n);

  bigint_set_trials(bigint_max(10, nbits / 16));

  while (1) {
    if (bigint_is_zero(n)) {
      _bigint_rand_range(n, n_hi, 1);
      bigint_add_bigint(n_lo, n);

      if (bigint_is_even(n)) {
        bigint_add_int(1, n);
      }

      for (i = 0; smallPrimes[i] > 0; i++) {
        p = (size_t)smallPrimes[i];
        bigint_mod_int(p, n, &r);
        pmod[i] = r;
      }
    }

    do_fast = 0;
    while (1) {
      for (i = 0; smallPrimes[i] > 0; ++i) {
        if (pmod[i] == 0) {
          do_fast = 0;
          break;
        }
      }
      if (do_fast == 0) {
        bigint_add_int(2, n);
        for (i = 0; smallPrimes[i] > 0; i++) {
          p = (size_t)smallPrimes[i];
          pmod[i] += 2;
          if (pmod[i] >= p) {
            pmod[i] -= p;
          }
        }
        do_fast = 1;
        continue;
      } else {
        break;
      }
      if (do_fast == 0) break;
    }
    if (bigint_is_prime(n)) {
      bigint_set_bigint(a, n);
      break;
    }
    bigint_add_int(2, n);
    for (i = 0; smallPrimes[i] > 0; ++i) {
      p = (size_t)smallPrimes[i];
      pmod[i] += 2;
      if (pmod[i] >= p) {
        pmod[i] -= p;
      }
    }
  }
}
/* r = a^x mod n */
void bigint_pow_mod(bigint a, bigint x,  bigint n, bigint r) {
  bigint t;

  bigint_one(r);
  bigint_zero(t);

  while (bigint_is_zero(x) == 0) {
    if (bigint_is_odd(x)) {
      bigint_mul_bigint(a, r, t);
      bigint_mod_bigint(n, t, r);
    }
    bigint_shbr(x, 1);
    bigint_mul_bigint(a, a, t);
    bigint_mod_bigint(n, t, a);
  }
}
void bigint_print_format(char * pref, bigint x, int printBytes) {
  size_t n = 0, i;
  bigint _10, q, r;
  char str[4096];

  for (n = BigInt_Size - 1; ; --n) {
    if (x[n] != 0) break;
    if (n == 0) break;
  }

  if (printBytes) {
    printf(" - %16s : ", pref);
    for (i = 0; i <= n; ++i) {
      printf("%3d ", x[i]);
    }
    printf("\n");
  }

  bigint_set(_10, 10);
  memset(str, 0, sizeof(str));

  n = 0;
  if (printBytes) {
    printf("   %16s : ", "Decimal");
  } else {
    printf(" - %16s : ", pref);
  }
  while (bigint_is_zero(x) == 0) {
    bigint_div_bigint(_10, x, q, r);
    bigint_set_bigint(x,q);
    str[n++] = '0' + r[0];
  }

  for (i = n - 1; ; --i) {
    printf("%c", str[i]);
    if (i == 0) break;
  }
  printf("\n");
}
void bigint_print(bigint a) {
  size_t i;
  for (i = BigInt_Size - 1; ; i--) {
    printf("%3d ",a[i]);
    if (i == 0) break;
  }
  printf("\n\n");
}
int bigint_is_prime(bigint n) {
  bigint _1, n_1, m, d, t, r, a, x, x2;
  bigint _2, _4, _max, g;
  size_t s = 0, i, j;

  if (bigint_is_even(n)) return 0;

  bigint_one(_1);
  bigint_set_bigint(n_1, n);
  bigint_sub_bigint(_1, n_1);

  bigint_set_bigint(m, n_1);
  bigint_set(_2, 2);
  bigint_set(_4, 4);

  while (bigint_is_even(m)) {
    ++s;
    bigint_shbr(m, 1);
  }

  bigint_set_bigint(m, n_1);
  bigint_set_bigint(t, _1);
  bigint_shbl(t, s);
  bigint_div_bigint(t, m, d, r);

  for (i = 0; i < bigint_trials; i++) {
    bigint_set_bigint(_max, n);
    bigint_sub_bigint(_4, _max);
    bigint_rand_range(a, _max);
    bigint_add_bigint(_2, a);

    bigint_pow_mod(a, d, n, x);
    if (bigint_equal(x, _1) || bigint_equal(x, n_1)) {
      continue;
    }

    for (j = 0; j < s - 1; j++) {
      bigint_mul_bigint(x, x, x2);
      bigint_mod_bigint(n, x2, g);
      bigint_set_bigint(x, g);

      if (bigint_equal(x, _1)) return 0;

      if (bigint_equal(x, n_1)) break;
    }

    if (bigint_equal(x, n_1) == 0) {
      return 0;
    }
  }

  return 1;
}
