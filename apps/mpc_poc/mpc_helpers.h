#ifndef MPC_HELPERS_H
#define MPC_HELPERS_H

#include <mpc_poc/core.pb.h>

#define ENTITY_INFO_BUFFER_SIZE 256

void mpc_sign_message(const uint8_t *message, size_t message_len, uint8_t *sig, const uint8_t *priv_key);
bool mpc_verify_signature(const uint8_t *message, size_t message_len, const uint8_t *sig, const uint8_t *pub_key);
void bytes_to_hex(const uint8_t *data, size_t data_len, char *out, size_t out_len);
int fetch_pub_key_from_flash(const uint8_t *wallet_id, uint8_t *pub_key);
int fetch_priv_key_from_flash(const uint8_t *wallet_id, uint8_t *priv_key);
void priv_key_from_seed(const uint8_t *seed, uint8_t *priv_key);
void pub_key33_from_priv_key(const uint8_t *priv_key, uint8_t *pub_key);

#endif