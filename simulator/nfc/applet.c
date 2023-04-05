#include "applet.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#define PN532_FILE "applet_comm.bin"

#define MAX_BUFFER_LEN 270
#define MAX_CARDS 4
#define MAX_WALLETS 4

#define APDU_BASE_OFFSET 8

#define CARD_FILE_NAME "sim_cards.bin"

typedef struct _wallet {
  uint8_t is_set;
  uint8_t name[16];
  uint8_t wallet_id[32];
  uint8_t password[32];
  uint8_t share[64];
  uint8_t checksum[4];
  uint8_t key[32];
  uint8_t benef_key[32];
  uint8_t benef_iv[32];
  uint8_t wallet_info;
  uint8_t mnemonics_count;
  uint8_t min_shares;
  uint8_t total_shares;
  uint8_t xcor;
  uint8_t is_locked;
} SimWallet;

typedef struct {
  SimWallet wallets[MAX_WALLETS];
} SimCard;

static SimCard cards[MAX_CARDS];
static uint8_t family_id[5] = {0xa1, 0xa2, 0xa3, 0xa4, 0x00};
static uint8_t card_number = 1;
static uint8_t version[6] = {0x01, 0x04, 0x01, 0x02, 0x03, 0x04};

static uint8_t applet_select_apdu[] =
    {0x00, 0xa4, 0x04, 0x00, 0x05, 0x01, 0x02, 0x03, 0x04, 0x05};
static uint8_t applet_select_resp[] = {0xb0,
                                       0x06,
                                       0x01,
                                       0x04,
                                       0x01,
                                       0x02,
                                       0x03,
                                       0x04,
                                       0xb1,
                                       0x05,
                                       0xa1,
                                       0xa2,
                                       0xa3,
                                       0xa4,
                                       0x00};

static int get_available_slot();
static int add_wallet(uint8_t id, uint8_t *buffer);
static int retrieve_wallet(uint8_t *buffer, uint8_t *out_buffer, uint16_t *len);
static int delete_wallet(uint8_t *buffer);
static void process();
static uint8_t adafruit_pn532_cs_complement_calc(uint8_t current_sum);
static void write_response(uint8_t *p_cmd, uint8_t cmd_len);
static uint8_t prepare_wallet_list(uint8_t *buffer);

static void init_cards() {
  FILE *file = fopen(CARD_FILE_NAME, "rb");
  errno = 0;
  if (!file) {
    file = fopen(CARD_FILE_NAME, "wb");
    if (!file)
      return;
    fwrite(&cards, 1, sizeof(cards), file);
  } else {
    file = fopen(CARD_FILE_NAME, "rb");
    if (!file)
      return;
    fread(&cards, 1, sizeof(cards), file);
  }
  if (errno)
    perror("");
  fclose(file);
}

static void save() {
  FILE *file = fopen(CARD_FILE_NAME, "wb");
  if (!file)
    return;
  fwrite(&cards, 1, sizeof(cards), file);
  if (errno)
    perror("");
  fclose(file);
}

ret_code_t applet_read(uint8_t *buffer, uint8_t size) {
  FILE *file = fopen(PN532_FILE, "rb");
  if (!file)
    return STM_ERROR_INTERNAL;
  errno = 0;
  fread(buffer, 1, size, file);
  fclose(file);
  if (errno) {
    perror("");
    return STM_ERROR_INTERNAL;
  }    // for (int i = 0; i < 33; i++) printf("%02X ", buffer[i]); printf("\n");
  return STM_SUCCESS;
}

ret_code_t applet_write(uint8_t *buffer, uint8_t size) {
  init_cards();
  FILE *file = fopen(PN532_FILE, "wb");
  if (!file)
    return STM_ERROR_INTERNAL;
  errno = 0;
  uint32_t count = fwrite(buffer, 1, size, file);
  fclose(file);
  if (errno) {
    perror("");
    return STM_ERROR_INTERNAL;
  }
  process();    // process the written command
  save();
  return STM_SUCCESS;
}

static void process() {
  family_id[4] = card_number;
  uint8_t buffer[MAX_BUFFER_LEN];
  uint8_t out_buffer[MAX_BUFFER_LEN];
  uint16_t off = 0, id, response;

  FILE *file = fopen(PN532_FILE, "rb");
  if (!file)
    return;
  errno = 0;
  uint16_t len = fread(buffer, MAX_BUFFER_LEN, 1, file);
  fclose(file);
  if (errno) {
    perror("");
    return;
  }

  switch (buffer[APDU_BASE_OFFSET + OFFSET_INS]) {
    case 0xa4:    // applet select
      if (memcmp(buffer + APDU_BASE_OFFSET + OFFSET_CLA,
                 applet_select_apdu,
                 sizeof(applet_select_apdu)) != 0) {
        off = 0;
        out_buffer[off++] = 0x69;
        out_buffer[off++] = 0x83;
        break;
      }
      card_number = (card_number % MAX_CARDS) + 1;
      memcpy(out_buffer, applet_select_resp, sizeof(applet_select_resp));
      off = sizeof(applet_select_resp);
      out_buffer[off - 1] = card_number;    // put correct number as this
                                            // program cycles b/w 1-4 cards
      out_buffer[off++] = 0x90;
      out_buffer[off++] = 0x00;
      break;

    case 0xc1:    // add wallet
      id = get_available_slot();
      off = 0;
      if (id < 0 || id > MAX_WALLETS) {
        out_buffer[off++] = 0x6a;
        out_buffer[off++] = 0x84;
      } else {
        response = add_wallet(id, buffer + APDU_BASE_OFFSET);
        out_buffer[off++] = ((response >> 8) & 0x00ff);
        out_buffer[off++] = (response & 0x00ff);
      }
      break;

    case 0xc2:    // retrieve wallet
      response = retrieve_wallet(buffer + APDU_BASE_OFFSET, out_buffer, &off);
      out_buffer[off++] = ((response >> 8) & 0x00ff);
      out_buffer[off++] = (response & 0x00ff);
      break;

    case 0xc3:    // delete wallet
      response = delete_wallet(buffer + APDU_BASE_OFFSET);
      out_buffer[off++] = ((response >> 8) & 0x00ff);
      out_buffer[off++] = (response & 0x00ff);
      break;

    case 0xc4:    // get all wallets (sync with card)
      off = prepare_wallet_list(out_buffer + 1);
      out_buffer[0] = off / 56;
      if (off) {
        out_buffer[off++] = 0x90;
        out_buffer[off++] = 0x00;
      } else {
        out_buffer[off++] = 0x6a;
        out_buffer[off++] = 0x83;
      }
      break;
  }
  write_response(out_buffer, off);
}

static int get_available_slot() {
  for (int i = 0; i < MAX_WALLETS; i++) {
    if (!cards[card_number - 1].wallets[i].is_set)
      return i;
  }
  return -1;
}

static int add_wallet(uint8_t id, uint8_t *buffer) {
  SimWallet *wallet = &cards[card_number - 1].wallets[id];
  memcpy(wallet->name, buffer + 7, sizeof(wallet->name));
  memcpy(wallet->password, buffer + 25, sizeof(wallet->password));
  memcpy(wallet->share, buffer + 65, sizeof(wallet->share));
  memcpy(wallet->checksum, buffer + 131, sizeof(wallet->checksum));
  memcpy(wallet->key, buffer + 141, sizeof(wallet->key));
  memcpy(wallet->benef_key, buffer + 175, sizeof(wallet->benef_key));
  memcpy(wallet->benef_iv, buffer + 193, sizeof(wallet->benef_iv));
  memcpy(wallet->wallet_id, buffer + 211, sizeof(wallet->wallet_id));
  wallet->xcor = buffer[58];
  wallet->mnemonics_count = buffer[60];
  wallet->total_shares = buffer[62];
  wallet->min_shares = buffer[136];
  wallet->wallet_info = buffer[138];
  for (int i = 0; i < MAX_WALLETS; i++) {
    SimWallet *curr_wallet = &cards[card_number - 1].wallets[i];
    if (curr_wallet->is_set) {
      if (memcmp(wallet->name, curr_wallet->name, sizeof(wallet->name)) == 0)
        return 0x6a80;
      if (memcmp(wallet->wallet_id,
                 curr_wallet->wallet_id,
                 sizeof(wallet->wallet_id)) == 0)
        return 0x6a80;
    }
  }
  wallet->is_set = 1;
  return 0x9000;
}

static int find_wallet(uint8_t *buffer) {
  for (int i = 0; i < 33; i++)
    printf("%02X ", buffer[i]);
  printf("\n");
  for (int i = 0; i < MAX_WALLETS; i++) {
    SimWallet *wallet = &cards[card_number - 1].wallets[i];
    if (wallet->is_set &&
        memcmp(buffer + 7, wallet->name, sizeof(wallet->name)) == 0) {
      if (WALLET_IS_PIN_SET(wallet->wallet_info))
        if (memcmp(buffer + 25, wallet->password, sizeof(wallet->password)) ==
            0)
          return i;
        else {
          // TODO: handle wrong PIN attempt
          return 0x6c01;
        }
      else
        return i;
    }
  }
  return -1;
}

static int retrieve_wallet(uint8_t *buffer,
                           uint8_t *out_buffer,
                           uint16_t *len) {
  int id = find_wallet(buffer);
  uint16_t size = 0;
  if (id < 0 || id > MAX_WALLETS)
    return 0x6a83;

  // prepare wallet
  SimWallet *wallet = &cards[card_number - 1].wallets[id];
  out_buffer[size++] = 0xe2;
  out_buffer[size++] = wallet->xcor;
  out_buffer[size++] = 0xe3;
  out_buffer[size++] = wallet->mnemonics_count;
  out_buffer[size++] = 0xe4;
  out_buffer[size++] = wallet->total_shares;
  out_buffer[size++] = 0xe5;
  out_buffer[size++] = sizeof(wallet->share);
  memcpy(out_buffer + size, wallet->share, sizeof(wallet->share));
  size += sizeof(wallet->share);
  out_buffer[size++] = 0xe6;
  out_buffer[size++] = sizeof(wallet->checksum);
  memcpy(out_buffer + size, wallet->checksum, sizeof(wallet->checksum));
  size += sizeof(wallet->checksum);
  out_buffer[size++] = 0xe7;
  out_buffer[size++] = wallet->min_shares;
  out_buffer[size++] = 0xe8;
  out_buffer[size++] = wallet->wallet_info;
  out_buffer[size++] = 0xe9;
  out_buffer[size++] = sizeof(wallet->key);
  memcpy(out_buffer + size, wallet->key, sizeof(wallet->key));
  size += sizeof(wallet->key);
  out_buffer[size++] = 0xec;
  out_buffer[size++] = sizeof(wallet->wallet_id);
  memcpy(out_buffer + size, wallet->wallet_id, sizeof(wallet->wallet_id));
  size += sizeof(wallet->wallet_id);

  if (len)
    *len = size;
  return 0x9000;
}

static int delete_wallet(uint8_t *buffer) {
  int id = find_wallet(buffer);
  if (id < 0 || id > MAX_WALLETS)
    return 0x6a83;
  cards[card_number - 1].wallets[id].is_set = 0;
  return 0x9000;
}

static uint8_t prepare_wallet_list(uint8_t *buffer) {
  uint8_t len = 0;
  for (int i = 0; i < MAX_WALLETS; i++) {
    SimWallet *wallet = &cards[card_number - 1].wallets[i];
    if (wallet->is_set) {
      buffer[len++] = 0xd4;
      buffer[len++] = wallet->is_locked;
      buffer[len++] = 0xe0;
      buffer[len++] = sizeof(wallet->name);
      memcpy(buffer + len, wallet->name, sizeof(wallet->name));
      len += sizeof(wallet->name);
      buffer[len++] = 0xe8;
      buffer[len++] = wallet->wallet_info;
      buffer[len++] = 0xec;
      buffer[len++] = sizeof(wallet->wallet_id);
      memcpy(buffer + len, wallet->wallet_id, sizeof(wallet->wallet_id));
      len += sizeof(wallet->wallet_id);
    }
  }
  return len;
}

static uint8_t adafruit_pn532_cs_complement_calc(uint8_t current_sum) {
  return ~current_sum + 1;
}

static void write_response(uint8_t *p_cmd, uint8_t cmd_len) {
  ret_code_t err_code;
  uint8_t checksum;
  uint8_t buffer[MAX_BUFFER_LEN] = {0};

  // Compose header part of the command frame.
  buffer[0] = PN532_PREAMBLE;
  buffer[1] = PN532_STARTCODE1;
  buffer[2] = PN532_STARTCODE2;
  buffer[3] = cmd_len + 3;    // Data length + TFI byte.
  buffer[4] = adafruit_pn532_cs_complement_calc(cmd_len + 3);
  buffer[5] = PN532_PN532TOHOST;
  buffer[6] = PN532_COMMAND_INDATAEXCHANGE + 1;
  buffer[7] = 0x00;

  // Copy the payload data.
  memcpy(buffer + HEADER_SEQUENCE_LENGTH + 2, p_cmd, cmd_len);

  // Calculate checksum.
  checksum = PN532_PN532TOHOST + PN532_COMMAND_INDATAEXCHANGE + 1;
  for (uint8_t i = 0; i < cmd_len; i++) {
    checksum += p_cmd[i];
  }
  checksum = adafruit_pn532_cs_complement_calc(checksum);

  // Compose checksum part of the command frame.
  buffer[HEADER_SEQUENCE_LENGTH + cmd_len + 2] = checksum;
  buffer[HEADER_SEQUENCE_LENGTH + cmd_len + 3] = PN532_POSTAMBLE;

  FILE *file = fopen(PN532_FILE, "wb");
  if (!file)
    return;
  errno = 0;
  fseek(file, 1, SEEK_SET);
  uint32_t count = fwrite(buffer, 1, cmd_len + PN532_FRAME_OVERHEAD + 2, file);
  fclose(file);
  if (errno) {
    perror("");
    return;
  }
  applet_read(buffer, cmd_len + PN532_FRAME_OVERHEAD + 2);
}