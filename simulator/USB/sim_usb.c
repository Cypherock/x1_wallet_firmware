#include "sim_usb.h"

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef _WIN32
#define TEMP_ENV_VAR "TEMP"
#define SIM_USB_RX_FILE_NAME "cypherock_device_in.bin"
#define SIM_USB_TX_FILE_NAME "cypherock_device_out.bin"
#else
#define SIM_USB_RX_FILE_NAME "/tmp/cypherock_device_in.bin"
#define SIM_USB_TX_FILE_NAME "/tmp/cypherock_device_out.bin"
#endif

static FILE *rx_file = NULL;
static FILE *tx_file = NULL;
static uint8_t rec_buffer[80];
volatile uint8_t rec_counter = 0;
static pthread_t ptid;

static void check_for_usb_data();
static int8_t SIM_Receive_FS(const uint8_t *Buf, const uint32_t *Len);
static uint32_t seek_pos = 0;

void SIM_USB_DEVICE_Init() {
  char file_name[250];

  errno = 0;

#ifdef _WIN32
  snprintf(file_name,
           sizeof(file_name),
           "%s\\%s",
           getenv(TEMP_ENV_VAR),
           SIM_USB_RX_FILE_NAME);
#else
  snprintf(file_name, sizeof(file_name), "%s", SIM_USB_RX_FILE_NAME);
#endif

  rx_file = fopen(file_name, "wb");
  if (errno) {
    perror("rx file");
    return;
  }
  fclose(rx_file);

#ifdef _WIN32
  snprintf(file_name,
           sizeof(file_name),
           "%s\\%s",
           getenv(TEMP_ENV_VAR),
           SIM_USB_TX_FILE_NAME);
#else
  snprintf(file_name, sizeof(file_name), "%s", SIM_USB_TX_FILE_NAME);
#endif
  tx_file = fopen(file_name, "wb");
  fclose(tx_file);
  if (errno)
    perror("tx file");

  rx_file = NULL;
  tx_file = NULL;
}

void SIM_Transmit_FS(uint8_t *data, uint8_t size) {
  char file_name[250];

#ifdef _WIN32
  snprintf(file_name,
           sizeof(file_name),
           "%s\\%s",
           getenv(TEMP_ENV_VAR),
           SIM_USB_TX_FILE_NAME);
#else
  snprintf(file_name, sizeof(file_name), "%s", SIM_USB_TX_FILE_NAME);
#endif

  errno = 0;
  tx_file = fopen(file_name, "ab");
  if (ftell(tx_file) > 1048576) {    // size of tx is >1MB erase all
    fclose(tx_file);
    tx_file = fopen(file_name, "wb");
  }
  if (errno)
    perror("ERROR (usb transmit)");
  if (!tx_file)
    return;
  errno = 0;
  fwrite(data, sizeof(uint8_t), size, tx_file);
  if (errno)
    perror("ERROR (usb transmit)");
  fclose(tx_file);
  tx_file = NULL;
}

static int8_t SIM_Receive_FS(const uint8_t *Buf, const uint32_t *Len) {
  // TODO
  return (USBD_OK);
  /* USER CODE END 6 */
}

static void check_for_usb_data() {
  FILE *file;
  char file_name[250];

#ifdef _WIN32
  snprintf(file_name,
           sizeof(file_name),
           "%s\\%s",
           getenv(TEMP_ENV_VAR),
           SIM_USB_RX_FILE_NAME);
#else
  snprintf(file_name, sizeof(file_name), "%s", SIM_USB_RX_FILE_NAME);
#endif

  uint8_t buffer[2048];

  file = fopen(file_name, "rb");
  fseek(file, seek_pos, SEEK_SET);
  errno = 0;
  uint32_t count = fread(buffer, sizeof(uint8_t), COMM_HEADER_SIZE, file);
  if (count == COMM_HEADER_SIZE) {
    count = fread(buffer + COMM_HEADER_SIZE,
                  sizeof(uint8_t),
                  buffer[DATA_SIZE_INDEX],
                  file) +
            COMM_HEADER_SIZE;
    seek_pos += count;
    printf("%d\n", count);
    for (int i = 0; i < count; i++)
      printf("%02X", buffer[i]);
    printf("\n");
    if (errno)
      perror("ERROR (rx file)");
    else
      SIM_Receive_FS(buffer, &count);
  }
  fclose(file);
  if (seek_pos > 1048576) {    // size of rx is >1MB erase all
    file = fopen(file_name, "wb");
    if (file)
      fclose(file);
  }
}

void usbsim_continue_loop() {
  check_for_usb_data();
}