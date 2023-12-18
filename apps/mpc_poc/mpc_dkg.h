#ifndef MPC_DKG_H
#define MPC_DKG_H

#include <mpc_poc/core.pb.h>
#include <stdint.h>

#include "mpc_context.h"

bool dkg_generate_signed_share_data(mpc_poc_group_info_t *group_info, 
                                    bool zero_polynomial,
                                    uint32_t *participants_indices,
                                    size_t participants_len,
                                    uint8_t *pub_key,
                                    bignum256 *secret_share,
                                    uint8_t *priv_key,
                                    mpc_poc_signed_share_data_t *signed_share_data);

bool dkg_get_individual_public_key(mpc_poc_group_info_t *group_info,
                                   size_t participants_len,
                                   uint8_t *pub_key,
                                   uint8_t *priv_key,
                                   bignum256 *secret_share,
                                   mpc_poc_signed_share_data_t *share_data_list,
                                   size_t share_data_list_count,
                                   uint8_t *Qi,
                                   uint8_t *signature,
                                   uint32_t *my_index);

bool dkg_get_group_public_key(mpc_poc_group_info_t *group_info,
                              uint8_t *pub_key,
                              uint8_t *priv_key,
                              bignum256 *secret_share,
                              mpc_poc_signed_public_key_t *signed_pub_key_list,
                              size_t signed_pub_key_list_count,
                              uint8_t *Qi,
                              mpc_poc_group_key_info_t *group_key_info,
                              uint8_t *signature);

#endif