#include <stdio.h>
#include "bigint.h"

#define min(a, b) (a < b) ? a : b
#define max(a, b) (a > b) ? a : b

int main(int argc, char ** argv) {
  int nbits = 512;
  bigint prime;

  printf("Usage: %s nbits\n", argv[0]);
  if (argc > 1) {
    nbits = max(8, atoi(argv[1]));
    nbits = min(1024, nbits);
  }

  printf("Searching for %d-bit prime ...\n", nbits);
  bigint_rand_prime(prime, nbits);
  printf("Found prime p:\n");
  /* bigint_print(prime); */
  bigint_print_format("p", prime, 1);

  return 0;
}
