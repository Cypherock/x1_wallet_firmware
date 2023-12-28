#include "mpc_helpers.h"
#include "mpc_api.h"

#include "curves.h"
#include "ecdsa.h"
#include "bip32.h"

#include "ui_delay.h"

bool dkg_generate_signed_share_data(mpc_poc_group_info_t *group_info, 
                                    bool zero_polynomial,
                                    uint32_t *participants_indices,
                                    size_t participants_len,
                                    uint8_t *pub_key,
                                    bignum256 *secret_share,
                                    uint8_t *priv_key,
                                    mpc_poc_signed_share_data_t *signed_share_data) {

  const ecdsa_curve* curve = get_curve_by_name(SECP256K1_NAME)->params;

  // generate random polynomial
  uint8_t coeff_count = group_info->threshold;
  bignum256 *coeff = malloc(coeff_count * sizeof(bignum256));
  uint8_t rand_coeff[32] = {0};

  for (int i = 0; i < coeff_count; i++) {
    if (i == 0 && zero_polynomial) {
      bn_zero(&coeff[i]);
      continue;
    }

    random_generate(rand_coeff, 32);
    bn_read_be(rand_coeff, &coeff[i]);
  }

  // get my sequence index in the group
  uint32_t my_index = 0;
  if (!pub_key_to_index(group_info, pub_key, &my_index)) {
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_REQUEST);
    free(coeff);
    return false;
  }

  // create share data
  mpc_poc_share_data_t share_data = MPC_POC_SHARE_DATA_INIT_ZERO;
  share_data.index = my_index;
  share_data.data_count = group_info->total_participants - 1; 
  int ind = 0;

  for (int _ = 0; _ < participants_len; ++_) {
    int i = participants_indices[_];

    bignum256 x = {0};
    bn_read_uint32(i, &x);
    
    if (i == my_index) {
      evaluate_polynomial(curve, coeff, coeff_count, &x, secret_share);
      continue;
    }

    mpc_poc_share_t share = MPC_POC_SHARE_INIT_ZERO;
    share.index = i;

    bignum256 fx = {0};

    evaluate_polynomial(curve, coeff, coeff_count, &x, &fx);

    uint8_t fx_bytes[32] = {0};
    bn_write_be(&fx, fx_bytes);

    curve_point *cp = malloc(sizeof(curve_point));
    ecdsa_read_pubkey(curve, group_info->participants[i-1].pub_key, cp);

    curve_point *sk = malloc(sizeof(curve_point));

    bignum256 k;
    bn_read_be(priv_key, &k);

    point_multiply(curve, &k, cp, sk);

    uint8_t sk_bytes[33] = {0};

    sk_bytes[0] = 0x02 | (sk->y.val[0] & 0x01);
    bn_write_be(&sk->x, sk_bytes + 1);

    uint8_t sk_hash[32] = {0};
    Hasher hasher;
    hasher_Init(&hasher, HASHER_SHA2);
    hasher_Update(&hasher, sk_bytes, 33);
    hasher_Final(&hasher, sk_hash);

    if (mpc_aes_encrypt(fx_bytes, 32, share.enc_share, sk_hash) != 0) {
      mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_REQUEST);
      free(coeff);
      return false;
    }

    share.original_length = 32;

    share_data.data[ind] = share;
    ind++;
  }

  uint8_t signature[64] = {0};
  if (!mpc_sign_struct(&share_data, SHARE_DATA_BUFFER_SIZE, MPC_POC_SHARE_DATA_FIELDS, signature, priv_key)) {
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_REQUEST);
    free(coeff);
    return false;
  }

  signed_share_data->has_share_data = true;
  signed_share_data->share_data = share_data;
  memcpy(signed_share_data->signature, signature, 64);

  free(coeff);

  return true;
}


bool dkg_get_individual_public_key(mpc_poc_group_info_t *group_info,
                                   size_t participants_len,
                                   uint8_t *pub_key,
                                   uint8_t *priv_key,
                                   bignum256 *secret_share,
                                   mpc_poc_signed_share_data_t *share_data_list,
                                   size_t share_data_list_count,
                                   uint8_t *Qi,
                                   uint8_t *signature,
                                   uint32_t *my_index) {

  const ecdsa_curve* curve = get_curve_by_name(SECP256K1_NAME)->params;

  if (share_data_list_count != participants_len) {
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  if (!pub_key_to_index(group_info, pub_key, my_index)) {
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  for (int _ = 0; _ < participants_len; ++_) {
    mpc_poc_signed_share_data_t signed_share_data = share_data_list[_];

    if (signed_share_data.has_share_data == false) {
      mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_REQUEST);
      return false;
    }

    mpc_poc_share_data_t share_data = signed_share_data.share_data;

    uint8_t *participant_pub_key = malloc(33 * sizeof(uint8_t));
    if (index_to_pub_key(group_info, share_data.index, participant_pub_key) == false) {
      mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_REQUEST);
      return false;
    }

    if (!mpc_verify_struct_sig(&share_data, SHARE_DATA_BUFFER_SIZE, MPC_POC_SHARE_DATA_FIELDS, 
                               signed_share_data.signature, participant_pub_key)) {
      mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_REQUEST);
      return false;
    }

    if (share_data.data_count != participants_len) {
      mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_REQUEST);
      return false;
    }

    mpc_poc_share_t my_share;
    bool share_found = false;
    
    for (int j = 0; j < share_data.data_count; ++j) {
      if (share_data.data[j].index == *my_index) {
        share_found = true;
        my_share = share_data.data[j];
        break;
      }
    } 

    if (share_found == false) {
      mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_REQUEST);
      return false;
    }

    curve_point *cp = malloc(sizeof(curve_point));
    ecdsa_read_pubkey(curve, participant_pub_key, cp);

    curve_point *sk = malloc(sizeof(curve_point));

    bignum256 k;
    bn_read_be(priv_key, &k);

    point_multiply(curve, &k, cp, sk);

    uint8_t sk_bytes[33] = {0};

    sk_bytes[0] = 0x02 | (sk->y.val[0] & 0x01);
    bn_write_be(&sk->x, sk_bytes + 1);

    uint8_t sk_hash[32] = {0};
    Hasher hasher;
    hasher_Init(&hasher, HASHER_SHA2);
    hasher_Update(&hasher, sk_bytes, 33);
    hasher_Final(&hasher, sk_hash);

    uint8_t *dec_share = malloc(32 * sizeof(uint8_t));

    if (mpc_aes_decrypt(my_share.enc_share, 32, dec_share, sk_hash) != 0) {
      mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_REQUEST);
      return false;
    }

    bignum256 bn_share;
    bn_read_be(dec_share, &bn_share);

    bn_addmod(secret_share, &bn_share, &curve->order);
  }
  

  curve_point *pub_key_point = malloc(sizeof(curve_point));
  scalar_multiply(curve, secret_share, pub_key_point);

  Qi[0] = 0x02 | (pub_key_point->y.val[0] & 0x01);
  bn_write_be(&pub_key_point->x, Qi + 1);

  // sign the individual_pub_key
  if (mpc_sign_message(Qi, 33, signature, priv_key) != 0) {
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_REQUEST);
    free(pub_key_point);
    return false;
  }

  free(pub_key_point);

  return true;
}


bool dkg_get_group_public_key(mpc_poc_group_info_t *group_info,
                              uint8_t *pub_key,
                              uint8_t *priv_key,
                              bignum256 *secret_share,
                              mpc_poc_signed_public_key_t *signed_pub_key_list,
                              size_t signed_pub_key_list_count,
                              uint8_t *Qi,
                              mpc_poc_group_key_info_t *group_key_info,
                              uint8_t *signature) {

  const ecdsa_curve* curve = get_curve_by_name(SECP256K1_NAME)->params;

  if (signed_pub_key_list_count < group_info->threshold - 1) {
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  uint32_t my_index = 0;
  if (!pub_key_to_index(group_info, pub_key, &my_index)) {
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  uint8_t *participant_pub_key = malloc(33 * sizeof(uint8_t));

  const curve_point* points[group_info->threshold];
  uint32_t xcords[group_info->threshold];
  int ind = 0;

  for (int _ = 0; _ < signed_pub_key_list_count; ++_) {
    if (ind == group_info->threshold) {
        break;
    }

    mpc_poc_signed_public_key_t signed_pub_key = signed_pub_key_list[_];

    xcords[ind] = signed_pub_key.index;

    if (index_to_pub_key(group_info, signed_pub_key.index, participant_pub_key) == false) {
      mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_REQUEST);
      free(participant_pub_key);
      return false;
    }

    if (mpc_verify_signature(signed_pub_key.pub_key, 33, signed_pub_key.signature, participant_pub_key) == false) {
      mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                        ERROR_DATA_FLOW_INVALID_REQUEST);
      free(participant_pub_key);
      return false;
    }

    curve_point *cp = malloc(sizeof(curve_point));
    ecdsa_read_pubkey(curve, signed_pub_key.pub_key, cp);

    points[ind] = cp;
    ind++;
  }

  free(participant_pub_key);

  curve_point Qj;
  curve_point Qi_point;

  if (ecdsa_read_pubkey(curve, Qi, &Qi_point) == 0) {
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  if (ind != group_info->threshold) {
    points[ind] = &Qi_point;
    xcords[ind] = my_index;
    ind++;
  }

  if (ind != group_info->threshold) {
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                    ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  lagarange_exp_interpolate(curve, points, xcords, my_index, group_info->threshold, &Qj);

  if (!point_is_equal(&Qj, &Qi_point)) {
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_REQUEST);
    return false;
  }

  curve_point Q;
  lagarange_exp_interpolate(curve, points, xcords, 0, group_info->threshold, &Q);

  uint8_t *group_pub_key = malloc(33 * sizeof(uint8_t));

  group_pub_key[0] = 0x02 | (Q.y.val[0] & 0x01);
  bn_write_be(&Q.x, group_pub_key + 1);

  memcpy(group_key_info->group_pub_key, group_pub_key, 33);

  mpc_poc_share_t share = MPC_POC_SHARE_INIT_ZERO;
  share.index = my_index;
  share.original_length = 32;

  uint8_t fx_bytes[32] = {0};
  bn_write_be(secret_share, fx_bytes);

  if (mpc_aes_encrypt(fx_bytes, 32, share.enc_share, priv_key) != 0) {
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_REQUEST);
    free(group_pub_key);
    return false;
  }

  group_key_info->has_group_share = true;
  group_key_info->group_share = share;

  if (!mpc_sign_struct(group_key_info, GROUP_KEY_INFO_BUFFER_SIZE, MPC_POC_GROUP_KEY_INFO_FIELDS, signature, priv_key)) {
    mpc_send_error(ERROR_COMMON_ERROR_CORRUPT_DATA_TAG,
                      ERROR_DATA_FLOW_INVALID_REQUEST);
    free(group_pub_key);
    return false;
  }

  free(group_pub_key);

  return true;
}