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
	do{
		if(rng_status = BSP_RNG_Generate(&random_32bit) != 0){
			LOG_ERROR("err:%08X\n", RNG_MCU_ERROR_BASE | rng_status);
			return false;
		}
//TODO: Utilize complete 32 bit random from MCU
		buf[i++]=random_32bit%256;
	}while(i<bufsize);
	return true;
}

/**
 * [in/out] buf The address where the saved random number should be saved.
 * [in] lower_range Random number should not be less than this number but can be equal to it.
 * [in] upper_range Random number should not be greater than this number but can be equal to it.
 */

bool crypto_random_generate_one_byte(uint8_t* buf, uint16_t lower_range, uint16_t upper_range)
{


    return true;
}

