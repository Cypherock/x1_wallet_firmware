#include "mpc_helpers.h"
#include "mpc_api.h"

#include "ecdsa.h"
#include "curves.h"
#include "bip32.h"
#include "coin_specific_data.h"

#include "ui_delay.h"
#include "ui_events_priv.h"
#include "ui_core_confirm.h"

#include "reconstruct_wallet_flow.h"

#include <stdio.h>
#include "aes/aes.h"

int mpc_sign_message(const uint8_t *message, size_t message_len, uint8_t *sig, const uint8_t *priv_key) {
    uint8_t pby;
    const ecdsa_curve *curve = get_curve_by_name(SECP256K1_NAME)->params;
    return ecdsa_sign(curve, HASHER_SHA2D, priv_key, message, message_len, sig, &pby, NULL);
}

bool mpc_verify_signature(const uint8_t *message, size_t message_len, const uint8_t *sig, const uint8_t *pub_key) {
    const ecdsa_curve *curve = get_curve_by_name(SECP256K1_NAME)->params;
    return (ecdsa_verify(curve, HASHER_SHA2D, pub_key, sig, message, message_len) == 0);
}

void bytes_to_hex(const uint8_t *data, size_t data_len, char *out, size_t out_len) {
    char *ptr = out;
    size_t remaining = out_len;
    size_t num_written;

    for (size_t i = 0; i < data_len; ++i) {
        // Each byte will produce two hexadecimal characters
        num_written = snprintf(ptr, remaining, "%02X", data[i]);

        if (num_written >= remaining) {
            // Not enough space in the buffer, stop writing
            break;
        }

        ptr += num_written; // Move the pointer forward
        remaining -= num_written; // Decrease the remaining space
    }

    // Ensure the output is null-terminated
    if (out_len > 0) {
        out[out_len - 1] = '\0';
    }
}

void priv_key_from_seed(const uint8_t *seed, uint8_t *priv_key) {
    const size_t SEED_LEN = 64;
    uint8_t sha256_hash[32];

    Hasher hasher;
    hasher_Init(&hasher, HASHER_SHA2);  

    hasher_Update(&hasher, seed, SEED_LEN);
    hasher_Final(&hasher, sha256_hash);

    for (int i = 0; i < 32; i++) {
        priv_key[i] = sha256_hash[i];
    }
}

void pub_key33_from_priv_key(const uint8_t *priv_key, uint8_t *pub_key) {
    const ecdsa_curve *curve = get_curve_by_name(SECP256K1_NAME)->params;
    ecdsa_get_public_key33(curve, priv_key, pub_key);
}

void display_msg_on_screen(const char *msg) {
  static lv_obj_t *instruction;
  lv_obj_clean(lv_scr_act());

  instruction = lv_label_create(lv_scr_act(), NULL);

  ui_paragraph(
      instruction,
      msg,
      LV_LABEL_ALIGN_CENTER);    // Creates task to print text on screen
  lv_obj_align(instruction, NULL, LV_ALIGN_CENTER, 0, 0);
  lv_label_set_text(instruction, msg);

  lv_task_handler();
}

void stop_msg_display() {
  if (ui_mark_event_over)
    (*ui_mark_event_over)();
}

void mpc_init_screen() {
    display_msg_on_screen("MPC application is running...\nCheck CLI for more info.");
}

void mpc_delay_scr_init(const char message[], const uint32_t delay_in_ms) {
    stop_msg_display();
    delay_scr_init(message, delay_in_ms);
    mpc_init_screen();
}

bool mpc_core_confirmation(const char *body, ui_core_rejection_cb *reject_cb) {
    stop_msg_display();
    bool status = core_confirmation(body, reject_cb);
    mpc_init_screen();

    return status;
}

bool mpc_core_scroll_page(const char *title,
                      const char *body,
                      ui_core_rejection_cb *reject_cb) {
    stop_msg_display();
    bool status = core_scroll_page(title, body, reject_cb);
    mpc_init_screen();

    return status;
}

int construct_mpc_key(const pb_byte_t* wallet_id, uint8_t* priv_key) {
    bool status = false;

    uint8_t seed[64] = {0};

    char msg[100] = "";
    char wallet_name[NAME_SIZE] = "";

    if (!get_wallet_name_by_id(wallet_id, (uint8_t *)wallet_name, mpc_send_error)) {
      return status;
    }

    snprintf(msg, sizeof(msg), "Use the MPC key of the %s wallet?", wallet_name);

    if (!mpc_core_confirmation(msg, mpc_send_error)) {
        return status;
    }

    stop_msg_display();
    if (!reconstruct_seed(wallet_id, &seed[0], mpc_send_error)) {
      memzero(seed, sizeof(seed));
      return status;
    }
    mpc_init_screen();

    priv_key_from_seed(seed, priv_key);
    memzero(seed, sizeof(seed));

    status = true;
    return status;
}

int initiate_application(const pb_byte_t* wallet_id, uint8_t* priv_key, uint8_t* pub_key) {
    bool status = false;
    mpc_init_screen();
    
    if (!construct_mpc_key(wallet_id, priv_key)) {
        return status;
    }

    pub_key33_from_priv_key(priv_key, pub_key);

    status = true;
    return status;
}

void evaluate_polynomial(const ecdsa_curve* curve,
                         const bignum256* coeff,
                         const uint8_t coeff_count,
                         const bignum256* x,
                         bignum256* fx) {
    assert(curve != NULL && coeff != NULL && x != NULL && fx != NULL);
    bignum256 term = {0}, x_pow_i = {0};

    bn_one(&x_pow_i);
    bn_zero(fx);

    for (int i = 0; i <= coeff_count; i++) {
        // fx += ( ai * (x ^ (i+1)) )
        bn_copy(&coeff[i], &term);
        bn_multiply(&x_pow_i, &term, &curve->order);
        bn_addmod(fx, &term, &curve->order);

        // calculate next power of x (i.e. x_pow_i = x_pow_i * x)
        bn_multiply(x, &x_pow_i, &curve->order);
    }
}

void pad_data(uint8_t *padded_data, const uint8_t *data, int original_len, int padded_len) {
    memcpy(padded_data, data, original_len);
    memset(padded_data + original_len, 0, padded_len - original_len);
}

int mpc_aes_encrypt(const uint8_t *data, size_t original_data_len, uint8_t *out, const uint8_t *key) {
    size_t padded_len = ((original_data_len + 15) / 16) * 16;
    uint8_t *padded_data = malloc(padded_len);
    pad_data(padded_data, data, original_data_len, padded_len);

    uint8_t iv[16]; // AES block size is 16 bytes
    memset(iv, 0, sizeof(iv));

    aes_encrypt_ctx enc_ctx;

    // Initialize encryption context with the key
    if (aes_encrypt_key256(key, &enc_ctx) != EXIT_SUCCESS) {
        free(padded_data);
        return 1;
    }

    // Encrypt the data
    return aes_cbc_encrypt(padded_data, out, padded_len, iv, &enc_ctx);
}

int mpc_aes_decrypt(const uint8_t *data, size_t original_data_len, uint8_t *out, const uint8_t *key) {
    size_t padded_len = ((original_data_len + 15) / 16) * 16;
    uint8_t *dec_buf = malloc(padded_len);

    uint8_t iv[16]; // AES block size is 16 bytes
    memset(iv, 0, sizeof(iv));

    aes_decrypt_ctx dec_ctx;

    // Initialize decryption context with the key
    if (aes_decrypt_key256(key, &dec_ctx) != EXIT_SUCCESS) {
        return 1;
    }

    // Decrypt the data
    if (aes_cbc_decrypt(data, dec_buf, padded_len, iv, &dec_ctx) != EXIT_SUCCESS) {
        return 1;
    }

    // Copy the original data to the output buffer
    memcpy(out, dec_buf, original_data_len);

    free(dec_buf);
    return 0;
}
