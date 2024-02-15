
#include "sim_usb.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/uhid.h>
#include <poll.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

static unsigned char rdesc[] = {
    0x06, 0xa0, 0xff,    // USAGE PAGE: Vendor 1
    0x09, 0x01,          // USAGE: Vendor 1
    0xa1, 0x01,          // Collection Application
    0x15, 0x00,          // Logical Minimum
    0x26, 0xff, 0x00,    // Logical Maximum
    0x75, 0x08,          // Report Size
    0x95, 0x40,          // Report Count
    0x09, 0x01,          // Usage: Vendor 1
    0x81, 0x08,          // Input (Data, Arr, Abs, Wrap)
    0x95, 0x40,          // Report Count
    0x09, 0x01,          // Usage: Vendor 1
    0x91, 0x08,          // Output (Data, Arr, Abs, Wrap)
    0x95, 0x01,          // Report Count
    0x09, 0x01,          // Usage: Vendor 1
    0xB1, 0x02,          // Feature
    0xC0                 // End collection
};

__u8 tx_buffer[192];
__u16 data_size = 0;

static int uhid_write(int fd, const struct uhid_event *ev) {
  ssize_t ret;

  ret = write(fd, ev, sizeof(*ev));
  if (ret < 0) {
    fprintf(stderr, "Cannot write to uhid: %m\n");
    return -errno;
  } else if (ret != sizeof(*ev)) {
    fprintf(
        stderr, "Wrong size written to uhid: %ld != %lu\n", ret, sizeof(ev));
    return -EFAULT;
  } else {
    return 0;
  }
}

static int create(int fd) {
  struct uhid_event ev;

  memset(&ev, 0, sizeof(ev));
  ev.type = UHID_CREATE;
  strcpy((char *)ev.u.create.name, "CYPHEROCK X1 WALLET");
  // strcpy((char*)ev.u.create.phys, "CYPHEROCK X1 WALLET");
  strcpy((char *)ev.u.create.uniq, "1122334455");
  ev.u.create.rd_data = rdesc;
  ev.u.create.rd_size = sizeof(rdesc);
  ev.u.create.bus = BUS_USB;
  ev.u.create.vendor = 0x3503;
  ev.u.create.product = 0x0103;
  ev.u.create.version = 0;
  ev.u.create.country = 0;

  return uhid_write(fd, &ev);
}

static void destroy(int fd) {
  struct uhid_event ev;

  memset(&ev, 0, sizeof(ev));
  ev.type = UHID_DESTROY;

  uhid_write(fd, &ev);
}

static void handle_output(struct uhid_event *ev) {
  // fprintf(stdout, "output.rtype = %d\n", ev->u.output.rtype);
  // print_hex_array("output.data", ev->u.output.data, ev->u.output.size);
  comm_packet_parser(
      &ev->u.output.data[1], ev->u.output.size - 1, COMM_LIBUSB__HID);
}

static int event(int fd) {
  struct uhid_event ev;
  ssize_t ret;

  memset(&ev, 0, sizeof(ev));
  ret = read(fd, &ev, sizeof(ev));
  if (ret == 0) {
    fprintf(stderr, "Read HUP on uhid-cdev\n");
    return -EFAULT;
  } else if (ret < 0) {
    fprintf(stderr, "Cannot read uhid-cdev: %m\n");
    return -errno;
  } else if (ret != sizeof(ev)) {
    fprintf(stderr,
            "Invalid size read from uhid-dev: %ld != %lu\n",
            ret,
            sizeof(ev));
    return -EFAULT;
  }

  switch (ev.type) {
    case __UHID_LEGACY_CREATE:
      fprintf(stdout, "__UHID_LEGACY_CREATE from uhid-dev\n");
      break;
    case UHID_DESTROY:
      fprintf(stdout, "UHID_DESTROY from uhid-dev\n");
      break;
    case UHID_START:
      fprintf(stdout, "UHID_START from uhid-dev\n");
      break;
    case UHID_STOP:
      fprintf(stdout, "UHID_STOP from uhid-dev\n");
      break;
    case UHID_OPEN:
      fprintf(stdout, "UHID_OPEN from uhid-dev\n");
      break;
    case UHID_CLOSE:
      fprintf(stdout, "UHID_CLOSE from uhid-dev\n");
      break;
    case UHID_OUTPUT:
      handle_output(&ev);
      break;
    case __UHID_LEGACY_OUTPUT_EV:
      fprintf(stdout, "__UHID_LEGACY_OUTPUT_EV from uhid-dev\n");
      break;
    case __UHID_LEGACY_INPUT:
      fprintf(stdout, "__UHID_LEGACY_INPUT from uhid-dev\n");
      break;
    case UHID_GET_REPORT:
      fprintf(stdout, "UHID_GET_REPORT from uhid-dev\n");
      break;
    case UHID_GET_REPORT_REPLY:
      fprintf(stdout, "UHID_GET_REPORT_REPLY from uhid-dev\n");
      break;
    case UHID_CREATE2:
      fprintf(stdout, "UHID_CREATE2 from uhid-dev\n");
      break;
    case UHID_INPUT2:
      fprintf(stdout, "UHID_INPUT2 from uhid-dev\n");
      break;
    case UHID_SET_REPORT:
      fprintf(stdout, "UHID_SET_REPORT from uhid-dev\n");
      break;
    case UHID_SET_REPORT_REPLY:
      fprintf(stdout, "UHID_SET_REPORT_REPLY from uhid-dev\n");
      break;
    default:
      fprintf(stderr, "Invalid event from uhid-dev: %u\n", ev.type);
  }

  return 0;
}

static int send_data(int fd) {
  struct uhid_event ev;

  memset(&ev, 0, sizeof(ev));
  ev.type = UHID_INPUT;
  ev.u.input.size = data_size;
  memcpy(&ev.u.input.data[0], tx_buffer, data_size);
  data_size = 0;
  return uhid_write(fd, &ev);
}

static void *start_hid_device(void *arg) {
  int fd;
  const char *path = "/dev/uhid";
  struct pollfd pfds[2];
  int ret;
  struct termios state;

  if (arg != NULL) {
    path = arg;
  }

  fd = open(path, O_RDWR | O_CLOEXEC);
  if (fd < 0) {
    fprintf(stderr, "Cannot open uhid-cdev %s: %m\n", path);
    return NULL;
  }

  ret = create(fd);
  if (ret) {
    close(fd);
    return NULL;
  }

  pfds[0].fd = fd;
  pfds[0].events = POLLIN | POLLPRI | POLLOUT;

  while (1) {
    ret = poll(pfds, 1, -1);
    if (ret < 0) {
      fprintf(stderr, "Cannot poll for fds: %m\n");
      break;
    }
    if (pfds[0].revents & POLLHUP) {
      fprintf(stderr, "Received HUP on uhid-cdev\n");
      break;
    }

    if ((pfds[0].revents & POLLIN) || (pfds[0].revents & POLLPRI)) {
      ret = event(fd);
      if (ret)
        break;
    }

    if ((pfds[0].revents & POLLOUT) && (data_size > 0)) {
      ret = send_data(fd);
      if (ret)
        printf("Failed to send data (status = %d)", ret);
    }
    usleep(3000);
  }

  fprintf(stderr, "Destroy uhid device\n");
  destroy(fd);
  return NULL;
}

void SIM_USB_DEVICE_Init() {
  pthread_t tid;    // Thread ID
  int result;

  // Create a new thread, passing the function myFunction as the thread's entry
  // point
  result = pthread_create(&tid, NULL, start_hid_device, "device");
  if (result != 0) {
    perror("pthread_create");
  }
}

void SIM_Transmit_FS(uint8_t *data, uint8_t size) {
  if (size < 128) {
    memcpy(tx_buffer, data, size);
    data_size = size;
  }
}