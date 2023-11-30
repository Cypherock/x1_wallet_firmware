#ifndef MPC_HELPERS_H
#define MPC_HELPERS_H

#include <mpc_poc/core.pb.h>
#include "ui_core_confirm.h"

#define ENTITY_INFO_BUFFER_SIZE 512
#define GROUP_INFO_BUFFER_SIZE 2048

int mpc_sign_message(const uint8_t *message, size_t message_len, uint8_t *sig, const uint8_t *priv_key);
bool mpc_verify_signature(const uint8_t *message, size_t message_len, const uint8_t *sig, const uint8_t *pub_key);
void bytes_to_hex(const uint8_t *data, size_t data_len, char *out, size_t out_len);
// int fetch_pub_key_from_flash(const uint8_t *wallet_id, uint8_t *pub_key);
// int fetch_priv_key_from_flash(const uint8_t *wallet_id, uint8_t *priv_key);
void priv_key_from_seed(const uint8_t *seed, uint8_t *priv_key);
void pub_key33_from_priv_key(const uint8_t *priv_key, uint8_t *pub_key);

void display_msg_on_screen(const char *msg);
void stop_msg_display();
void mpc_init_screen();

void mpc_delay_scr_init(const char message[], const uint32_t delay_in_ms);
bool mpc_core_confirmation(const char *body, ui_core_rejection_cb *reject_cb);
bool mpc_core_scroll_page(const char *title,
                      const char *body,
                      ui_core_rejection_cb *reject_cb);

int construct_mpc_key(const pb_byte_t* wallet_id, uint8_t* priv_key);

int initiate_application(const pb_byte_t* wallet_id, uint8_t* priv_key, uint8_t* pub_key);

#endif