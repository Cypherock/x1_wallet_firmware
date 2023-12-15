#ifndef MPC_HELPERS_H
#define MPC_HELPERS_H

#include <mpc_poc/core.pb.h>
#include "ui_core_confirm.h"
#include "ecdsa.h"
#include "aes/aes.h"
#include "bignum.h"

#define ENTITY_INFO_BUFFER_SIZE 512
#define GROUP_INFO_BUFFER_SIZE 2048
#define SHARE_DATA_BUFFER_SIZE 2048
#define GROUP_KEY_INFO_BUFFER_SIZE 256

int mpc_sign_message(const uint8_t *message, size_t message_len, uint8_t *sig, const uint8_t *priv_key);
bool mpc_verify_signature(const uint8_t *message, size_t message_len, const uint8_t *sig, const uint8_t *pub_key);

bool mpc_sign_struct(const void *src_struct, size_t buffer_size,
                    const pb_msgdesc_t *fields, uint8_t *sig, 
                    const uint8_t *priv_key);

bool mpc_verify_struct_sig(const void *src_struct, size_t buffer_size, 
                          const pb_msgdesc_t *fields, const uint8_t *sig, 
                          const uint8_t *pub_key);

void bytes_to_hex(const uint8_t *data, size_t data_len, char *out, size_t out_len);
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
void evaluate_polynomial(const ecdsa_curve* curve,
                         const bignum256* coeff,
                         const uint8_t coeff_count,
                         const bignum256* x,
                         bignum256* fx);

int mpc_aes_encrypt(const uint8_t *data, size_t original_data_len, uint8_t *out, const uint8_t *key);
int mpc_aes_decrypt(const uint8_t *data, size_t original_data_len, uint8_t *out, const uint8_t *key);

void evaluate_exp_lagarange_term(const ecdsa_curve* curve,
                                        const curve_point* point,
                                        const uint64_t x_cord,
                                        const uint64_t interpolate_point,
                                        const uint64_t threshold,
                                        curve_point* result);


void lagarange_exp_interpolate(const ecdsa_curve* curve,
                               const curve_point** points,
                               const uint32_t* x_cords,
                               const uint32_t interpolate_point,
                               const uint32_t threshold,
                               curve_point* result);

bool index_to_pub_key(const mpc_poc_group_info_t *group_info, uint32_t index, uint8_t *pub_key);
bool pub_key_to_index(const mpc_poc_group_info_t *group_info, const uint8_t *pub_key, uint32_t *index);

#endif