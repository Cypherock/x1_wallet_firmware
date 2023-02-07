#include "session_utils.h"

bool verify_session_digest(uint8_t *payload, uint16_t payload_length,
                           uint8_t *buffer) {

    uint8_t hash[32] = {0};
    sha256_Raw(payload, payload_length, hash);
    uint8_t session_key_derv_data[12] = {0};
    HDNode session_node;
    uint32_t index;
    char xpub[112] = {'\0'};

    base58_encode_check(get_card_root_xpub(), FS_KEYSTORE_XPUB_LEN,
                        nist256p1_info.hasher_base58, xpub, 112);
    hdnode_deserialize_public((char *) xpub,
                              0x0488b21e,
                              NIST256P1_NAME,
                              &session_node,
                              NULL);

    index = read_be(session_key_derv_data);
    hdnode_public_ckd(&session_node, index);

    index = read_be(session_key_derv_data + 4);
    hdnode_public_ckd(&session_node, index);

    index = read_be(session_key_derv_data + 8);
    hdnode_public_ckd(&session_node, index);

    uint8_t
        status = ecdsa_verify_digest(&nist256p1,
                                     session_node.public_key,
                                     buffer,
                                     hash);

    return status;
}