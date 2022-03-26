#ifndef CRYPTO_RANDOM_H
#define CRYPTO_RANDOM_H

//#include "crypto_main.h"
//#include "crys_rnd.h"
#include "board.h"
#include "stdbool.h"
uint32_t crypto_random_begin();
bool crypto_random_generate(uint8_t* buf, uint16_t bufsize);
bool crypto_random_generate_one_byte(uint8_t* buf, uint16_t lower_range, uint16_t upper_range);
void crypto_random_end();
//CRYS_RND_State_t* get_rnd_state_ptr();

#endif
