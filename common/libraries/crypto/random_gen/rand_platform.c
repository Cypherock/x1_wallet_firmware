/**
 * @file    rand_platform.c
 * @author  Cypherock X1 Team
 * @brief   Hardware-TRNG-backed random32()/random_reseed() for device builds.
 * @details When RAND_PLATFORM_INDEPENDENT is defined (device firmware only, set
 *          in utilities/cmake/firmware/firmware.cmake), the insecure reference
 *          LCG in rand.c is excluded and these implementations are linked
 *          instead. random_buffer()/random_uniform() in rand.c are platform
 *          independent and draw from random32(), so they become secure too.
 * @copyright Copyright (c) 2024 HODL TECH PTE LTD
 * <br/> You may obtain a copy of license at <a href="https://mitcc.org/"
 * target=_blank>https://mitcc.org/</a>
 */

#include "rand.h"

// Regression guard: a device build (not the simulator) must exclude the
// insecure LCG by defining RAND_PLATFORM_INDEPENDENT. Fail the build otherwise.
#if (USE_SIMULATOR == 0) && !defined(RAND_PLATFORM_INDEPENDENT)
#error \
    "Device build must define RAND_PLATFORM_INDEPENDENT so the insecure LCG in rand.c is excluded"
#endif

#ifdef RAND_PLATFORM_INDEPENDENT

#include <stddef.h>
#include <stdint.h>

// Hardware TRNG entry point (common/libraries/util/utils.c): mixes the MCU RNG
// with the ATECC secure element and asserts on failure.
extern void random_generate(uint8_t *arr, int len);

void random_reseed(const uint32_t value) {
  (void)value;
}

uint32_t random32(void) {
  // Amortise the TRNG calls with a small pool; bytes are never reused.
  static uint8_t pool[32];
  static size_t used = sizeof(pool);    // start empty -> refill on first use

  uint32_t r = 0;
  for (size_t i = 0; i < sizeof(r); i++) {
    if (used >= sizeof(pool)) {
      random_generate(pool, (int)sizeof(pool));
      used = 0;
    }
    ((uint8_t *)&r)[i] = pool[used++];
  }
  return r;
}

#endif /* RAND_PLATFORM_INDEPENDENT */
