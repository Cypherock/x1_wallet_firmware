/**
 * Copyright (c) 2013-2014 Tomas Dzetkulic
 * Copyright (c) 2013-2014 Pavol Rusnak
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "rand.h"

#ifndef RAND_PLATFORM_INDEPENDENT

// #pragma message( \
//     "NOT SUITABLE FOR PRODUCTION USE! Replace random32() function with your
//     own secure code.")

// The following code is not supposed to be used in a production environment.
// It's included only to make the library testable.
// The message above tries to prevent any accidental use outside of the test
// environment.
//
// You are supposed to replace the random8() and random32() function with your
// own secure code. There is also a possibility to replace the random_buffer()
// function as it is defined as a weak symbol.

// [SEC-AUDIT BUG-28] The reference LCG below is explicitly "NOT SUITABLE FOR
// PRODUCTION". random32()/random_buffer() are instead backed by the hardware
// TRNG: random_generate() mixes the MCU RNG with the ATECC secure element and
// asserts on failure. These feed the ECDSA side-channel blinding in ecdsa.c;
// the signature output is unchanged (blinding cancels) - only the entropy
// source changes. See docs/SECURITY_AUDIT_BUGS.md.
void random_reseed(const uint32_t value) {
  (void)value;
}

uint32_t random32(void) {
  uint32_t r = 0;
  random_buffer((uint8_t *)&r, sizeof(r));
  return r;
}

#endif /* RAND_PLATFORM_INDEPENDENT */

//
// The following code is platform independent
//

// [SEC-AUDIT BUG-28] hardware TRNG source (defined in common/libraries/util).
extern void random_generate(uint8_t *arr, int len);

// random_generate() fills at most 32 bytes per call; a small pool amortises the
// TRNG calls so the blinding path (many small random32() draws) is not slowed
// excessively. Bytes are never reused.
static uint8_t s_trng_pool[32];
static size_t s_trng_used = sizeof(s_trng_pool);    // start empty -> refill

void random_buffer(uint8_t *buf, size_t len) {
  for (size_t i = 0; i < len; i++) {
    if (s_trng_used >= sizeof(s_trng_pool)) {
      random_generate(s_trng_pool, (int)sizeof(s_trng_pool));
      s_trng_used = 0;
    }
    buf[i] = s_trng_pool[s_trng_used++];
  }
}

uint32_t random_uniform(uint32_t n) {
  uint32_t x = 0, max = 0xFFFFFFFF - (0xFFFFFFFF % n);
  while ((x = random32()) >= max)
    ;
  return x / (max / n);
}

void random_permute(char *str, size_t len) {
  for (int i = len - 1; i >= 1; i--) {
    int j = random_uniform(i + 1);
    char t = str[j];
    str[j] = str[i];
    str[i] = t;
  }
}
