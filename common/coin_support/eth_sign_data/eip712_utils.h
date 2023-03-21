#ifndef EIP712_UTILS
#define EIP712_UTILS
#include "abi.h"
#include "eip712.pb.h"

#define BUFFER_SIZE      1024
#define HASH_SIZE        32
#define DYNAMIC_BYTES_ID "bytes"

enum EIP712_STATUS_CODES {
  EIP712_OK = 0,
  EIP712_MEMORY_LIMIT_EXCEEDED,
  EIP712_MEMORY_ALLOCATION_FAILED,
  EIP712_ABI_ENCODING_FAILED,
  EIP712_INVALID_DATA,
  EIP712_ERROR,
};

typedef struct queue_node {
  TypedDataStruct_TypedDataNode *tree_node;
  struct queue_node *next;
  char *prefix;
} queue_node;

typedef struct queue {
  queue_node *front;
  queue_node *rear;
  int count;
} queue;

queue_node *new_queue_node(TypedDataStruct_TypedDataNode *tree_node,
                           const char *prefix);

queue *create_queue();

int is_empty(queue *q);

void enqueue(queue *q,
             TypedDataStruct_TypedDataNode *tree_node,
             const char *prefix);

queue_node *dequeue(queue *q);

void fill_string_with_data(const TypedDataStruct_TypedDataNode *data_node,
                           char *output,
                           const size_t output_size);

int hash_struct(const TypedDataStruct_TypedDataNode *data_node,
                uint8_t *output);

int encode_data(const TypedDataStruct_TypedDataNode *data_node,
                uint8_t *output,
                const size_t output_size,
                size_t *bytes_written);

#endif /* EIP712_UTILS */