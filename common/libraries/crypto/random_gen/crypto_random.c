#include "crypto_random.h"
#include <stdlib.h>
#include "logger.h"
#include "app_error.h"

static bool begun = false;

uint32_t crypto_random_begin()
{
	BSP_RNG_Init();
	return 0;
}

void crypto_random_end()
{
	BSP_RNG_End();
}

bool crypto_random_generate(uint8_t* buf, uint16_t bufsize)
{
	uint32_t random_32bit;
	uint16_t i=0;
	uint32_t rng_status;
	// nothing to write for an empty buffer
	while(i<bufsize){
		// log the RNG status, not the result of the compare
		if((rng_status = BSP_RNG_Generate(&random_32bit)) != 0){
			LOG_ERROR("err:%08X\n", RNG_MCU_ERROR_BASE | rng_status);
			return false;
		}
//TODO: Utilize complete 32 bit random from MCU
		buf[i++]=random_32bit%256;
	}
	return true;
}
