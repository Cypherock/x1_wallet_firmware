#ifndef EIP712_UTILS
#define EIP712_UTILS
#include <evm/core.pb.h>

#include "abi.h"

#define BUFFER_SIZE 1024
#define HASH_SIZE 32
#define DYNAMIC_BYTES_ID "bytes"

typedef enum EIP712_STATUS_CODES {
  EIP712_OK = 0,
  EIP712_MEMORY_LIMIT_EXCEEDED,
  EIP712_MEMORY_ALLOCATION_FAILED,
  EIP712_ABI_ENCODING_FAILED,
  EIP712_INVALID_DATA,
  EIP712_ERROR,
} eip712_status_codes_e;

typedef struct queue_node {
  evm_sign_typed_data_node_t *tree_node;
  struct queue_node *next;
  char *prefix;
} queue_node;

typedef struct queue {
  queue_node *front;
  queue_node *rear;
  int count;
} queue;

queue_node *new_queue_node(evm_sign_typed_data_node_t *tree_node,
                           const char *prefix);

queue *create_queue();

int is_empty(queue *q);

void enqueue(queue *q,
             evm_sign_typed_data_node_t *tree_node,
             const char *prefix);

queue_node *dequeue(queue *q);

void fill_string_with_data(const evm_sign_typed_data_node_t *data_node,
                           char *output,
                           const size_t output_size);

int hash_struct(const evm_sign_typed_data_node_t *data_node, uint8_t *output);

int encode_data(const evm_sign_typed_data_node_t *data_node,
                uint8_t *output,
                const size_t output_size,
                size_t *bytes_written);

#endif /* EIP712_UTILS */