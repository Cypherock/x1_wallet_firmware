#include "session_utils.h"
#include "controller_level_four.h"

uint8_t session_key_derv_data[12] = {0};

void derive_public_key(Session *session) {
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
}

bool verify_session_signature(Session *session, uint8_t *payload, uint16_t
payload_length, uint8_t *buffer) {

    uint8_t hash[32] = {0};
    sha256_Raw(payload, payload_length, hash);

    uint8_t
        status = ecdsa_verify_digest(&nist256p1,
                                     session->public_key,
                                     buffer,
                                     hash);

    return status;
}

void append_signature(uint8_t *payload, uint16_t payload_length, Message
*message) {
    uint8_t hash[32] = {0};
    sha256_Raw(payload, payload_length, hash);
    auth_data_t signed_data = atecc_sign(hash);
    memcpy(message->signature, signed_data.signature, SIGNATURE_SIZE);
    memcpy(message->postfix1, signed_data.postfix1, POSTFIX1_SIZE);
    memcpy(message->postfix2, signed_data.postfix2, POSTFIX2_SIZE);
}

void session_pre_init(Session *session, Message *session_pre_init_details) {
    random_generate(session->device_random, DEVICE_RANDOM_SIZE);

    derive_public_key(session);

    get_device_serial();
    memcpy(session->device_id, atecc_data.device_serial, DEVICE_SERIAL_SIZE);

    session_pre_init_details->message = (uint8_t *) malloc
        (DEVICE_RANDOM_SIZE + DEVICE_SERIAL_SIZE);
    session_pre_init_details->message_size = DEVICE_RANDOM_SIZE +
        DEVICE_SERIAL_SIZE;

    memcpy(session_pre_init_details->message,
           session->device_random,
           DEVICE_RANDOM_SIZE);
    memcpy(session_pre_init_details->message + DEVICE_RANDOM_SIZE,
           session->device_id,
           DEVICE_SERIAL_SIZE);

    append_signature(session_pre_init_details->message,
                     DEVICE_RANDOM_SIZE + DEVICE_SERIAL_SIZE,
                     session_pre_init_details);
}

void byte_array_to_session_message(uint8_t *data_array, uint16_t msg_size,
                                   Message *msg) {
    msg->message_size = msg_size - SIGNATURE_SIZE;
    msg->message = (uint8_t *) malloc(msg->message_size);
    memcpy(msg->message, data_array, msg->message_size);
    memcpy(msg->signature, data_array + msg->message_size,
           SIGNATURE_SIZE);
}

uint8_t session_message_to_byte_array(Message msg, uint8_t *data_array) {
    uint8_t data_array_size = 0;
    memcpy(data_array, msg.message, msg.message_size);
    data_array_size += msg.message_size;
    memcpy(data_array + data_array_size, msg.signature, SIGNATURE_SIZE);
    data_array_size += SIGNATURE_SIZE;
    memcpy(data_array + data_array_size, msg.postfix1, POSTFIX1_SIZE);
    data_array_size += POSTFIX1_SIZE;
    memcpy(data_array + data_array_size, msg.postfix2, POSTFIX2_SIZE);
    data_array_size += POSTFIX2_SIZE;

    return data_array_size;
}

bool session_init(Session *session, Message *session_init_details) {

    memcpy(session->session_id,
           session_init_details->message,
           SESSION_ID_SIZE);

    uint8_t payload[SESSION_ID_SIZE + DEVICE_RANDOM_SIZE +
        DEVICE_SERIAL_SIZE];
    size_t payload_length = 0;
    memcpy(payload, session->session_id, SESSION_ID_SIZE);
    payload_length += SESSION_ID_SIZE;
    memcpy(payload + payload_length, session->device_random,
           DEVICE_RANDOM_SIZE);
    payload_length += DEVICE_RANDOM_SIZE;
    memcpy(payload + payload_length, session->device_id,
           DEVICE_SERIAL_SIZE);
    payload_length += DEVICE_SERIAL_SIZE;

    if (!verify_session_signature(session, payload,
                                  payload_length,
                                  session_init_details->signature)) {
        return false;
    }

    if (memcmp(session_init_details->message + SESSION_ID_SIZE,
               session->device_id, DEVICE_SERIAL_SIZE) != 0) {
        return false;
    }

    session->session_age = bendian_byte_to_dec(session_init_details->message +
                                                   SESSION_ID_SIZE +
                                                   DEVICE_SERIAL_SIZE,
                                               sizeof(session->session_age));

    return true;
}
