#ifndef EIP712_UTILS
#define EIP712_UTILS
#include "./simple.pb.h"
#include "abi.h"

#define BUFFER_SIZE      1024
#define HASH_SIZE        32
#define DYNAMIC_BYTES_ID "bytes"

enum {
  EIP712_OK = 0,
  EIP712_MEMORY_LIMIT_EXCEEDED,
  EIP712_MEMORY_ALLOCATION_FAILED,
  EIP712_ABI_ENCODING_FAILED,
  EIP712_INVALID_DATA,
  EIP712_ERROR,
} EIP712_STATUS_CODES;

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

queue_node *new_queue_node(TypedDataStruct_TypedDataNode *tree_node, const char *prefix) {
  queue_node *temp = (queue_node *)cy_malloc(sizeof(queue_node));
  temp->tree_node  = tree_node;
  temp->prefix     = cy_malloc(strlen(prefix) + 1);
  strcpy(temp->prefix, prefix);
  temp->next = NULL;
  return temp;
}

queue *create_queue() {
  queue *q = (queue *)cy_malloc(sizeof(queue));
  q->front = q->rear = NULL;
  q->count           = 0;
  return q;
}

int is_empty(queue *q) {
  return (q->count == 0);
}

void enqueue(queue *q, TypedDataStruct_TypedDataNode *tree_node, const char *prefix) {
  queue_node *temp = new_queue_node(tree_node, prefix);
  q->count++;
  if (q->rear == NULL) {
    q->front = q->rear = temp;
    return;
  }
  q->rear->next = temp;
  q->rear       = temp;
}

queue_node *dequeue(queue *q) {
  if (is_empty(q))
    return NULL;
  queue_node *temp = q->front;
  q->front         = q->front->next;
  if (q->front == NULL)
    q->rear = NULL;
  q->count--;
  return temp;
}

void fill_string_with_data(const TypedDataStruct_TypedDataNode *data_node, char *output, const size_t output_size) {
  memzero(output, output_size);
  char buffer[BUFFER_SIZE] = {0};
  switch (data_node->type) {
    case TypedDataStruct_TypedDataNode_Eip712DataType_ARRAY:
    case TypedDataStruct_TypedDataNode_Eip712DataType_STRUCT:
      snprintf(buffer, sizeof(buffer), "Contains %ld elements", data_node->size);
      break;
    case TypedDataStruct_TypedDataNode_Eip712DataType_UINT:
    case TypedDataStruct_TypedDataNode_Eip712DataType_INT:
    case TypedDataStruct_TypedDataNode_Eip712DataType_BOOL:
      snprintf(buffer, sizeof(buffer), "%d", *(data_node->data->bytes));
      break;
    case TypedDataStruct_TypedDataNode_Eip712DataType_STRING:
      snprintf(buffer, data_node->data->size +1, "%s", data_node->data->bytes);
      break;
    case TypedDataStruct_TypedDataNode_Eip712DataType_BYTES:
    case TypedDataStruct_TypedDataNode_Eip712DataType_ADDRESS:
    default:
      snprintf(buffer, sizeof(buffer), "0x");
      byte_array_to_hex_string(data_node->data->bytes, data_node->data->size, buffer+2, sizeof(buffer)-1);
      break;
  }
  snprintf(output, output_size, "%s: %s", data_node->struct_name, buffer);
}
int hash_struct(const TypedDataStruct_TypedDataNode *data_node, uint8_t *output);
int encode_data(const TypedDataStruct_TypedDataNode *data_node,
                uint8_t *output,
                const size_t output_size,
                size_t *bytes_written) {
  if (output_size < HASH_SIZE)
    return EIP712_MEMORY_LIMIT_EXCEEDED;
  switch (data_node->type) {
    {
      case TypedDataStruct_TypedDataNode_Eip712DataType_UINT: {
        uint8_t abi_status = Abi_Encode(Abi_uint256_e, data_node->size, data_node->data->bytes, output);
        if (abi_status != ABI_PROCESS_COMPLETE) {
          LOG_ERROR("ABI:%d", abi_status);
          printf("ABI:%d", abi_status);
          return EIP712_ABI_ENCODING_FAILED;
        }
        *bytes_written += HASH_SIZE;
      } break;
      case TypedDataStruct_TypedDataNode_Eip712DataType_INT: {
        uint8_t abi_status = Abi_Encode(Abi_int256_e, data_node->size, data_node->data->bytes, output);
        if (abi_status != ABI_PROCESS_COMPLETE) {
          LOG_ERROR("ABI:%d", abi_status);
          printf("ABI:%d", abi_status);
          return EIP712_ABI_ENCODING_FAILED;
        }
        *bytes_written += HASH_SIZE;
      } break;
      case TypedDataStruct_TypedDataNode_Eip712DataType_BOOL: {
        uint8_t abi_status = Abi_Encode(Abi_bool_e, data_node->size, data_node->data->bytes, output);
        if (abi_status != ABI_PROCESS_COMPLETE) {
          LOG_ERROR("ABI:%d", abi_status);
          printf("ABI:%d", abi_status);
          return EIP712_ABI_ENCODING_FAILED;
        }
        *bytes_written += HASH_SIZE;
      } break;
      case TypedDataStruct_TypedDataNode_Eip712DataType_ADDRESS: {
        uint8_t abi_status = Abi_Encode(Abi_address_e, data_node->size, data_node->data->bytes, output);
        if (abi_status != ABI_PROCESS_COMPLETE) {
          LOG_ERROR("ABI:%d", abi_status);
          printf("ABI:%d", abi_status);
          return EIP712_ABI_ENCODING_FAILED;
        }
        *bytes_written += HASH_SIZE;
      } break;
      case TypedDataStruct_TypedDataNode_Eip712DataType_BYTES: {
        if (strncmp(data_node->struct_name, DYNAMIC_BYTES_ID, sizeof(DYNAMIC_BYTES_ID)) != 0) {
          uint8_t abi_status = Abi_Encode(Abi_bytes_e, data_node->size, data_node->data->bytes, output);
          if (abi_status != ABI_PROCESS_COMPLETE) {
            LOG_ERROR("ABI:%d", abi_status);
            printf("ABI:%d", abi_status);
            return EIP712_ABI_ENCODING_FAILED;
          }
          *bytes_written += HASH_SIZE;
          break;
        }
      }
      case TypedDataStruct_TypedDataNode_Eip712DataType_STRING:
        keccak_256(data_node->data->bytes, data_node->data->size, output);
        *bytes_written += HASH_SIZE;
        break;
      case TypedDataStruct_TypedDataNode_Eip712DataType_ARRAY: {
            size_t result_size = data_node->children_count * HASH_SIZE;
      uint8_t *result = malloc(result_size);
      if (result == NULL)
        return EIP712_MEMORY_ALLOCATION_FAILED;
      memzero(result, result_size);
      int status = EIP712_OK;
      size_t dummy = 0;
      for (int i = 0; i < data_node->children_count; i++)
      {
        if (data_node->children[i].type == TypedDataStruct_TypedDataNode_Eip712DataType_STRUCT)
          status = hash_struct(&data_node->children[i], result + i * HASH_SIZE);
        else
          status = encode_data(&data_node->children[i], result + i * HASH_SIZE, HASH_SIZE, &dummy);
        if (status != EIP712_OK)
        {
          free(result);
          return status;
        }
      }
      print_hex_array("EncodeData Array PreHash", result, result_size);
      keccak_256(result, result_size, output);
      *bytes_written += HASH_SIZE;
      free(result);
      print_hex_array("EncodeData Array", output, HASH_SIZE);
      } break;
      case TypedDataStruct_TypedDataNode_Eip712DataType_STRUCT: {
        size_t result_size = data_node->children_count * HASH_SIZE;
        if (result_size > output_size)
          return EIP712_MEMORY_LIMIT_EXCEEDED;
        uint8_t *result = malloc(result_size);
        if (result == NULL)
          return EIP712_MEMORY_ALLOCATION_FAILED;
        memzero(result, result_size);
        int status   = EIP712_OK;
        size_t dummy = 0;
        for (int i = 0; i < data_node->children_count; i++) {
          if (data_node->children[i].type == TypedDataStruct_TypedDataNode_Eip712DataType_STRUCT)
            status = hash_struct(&data_node->children[i], result + i * HASH_SIZE);
          else
            status = encode_data(&data_node->children[i], result + i * HASH_SIZE, HASH_SIZE, &dummy);

          if (status != EIP712_OK) {
            free(result);
            return status;
          }
        }
        memcpy(output, result, result_size);
        *bytes_written += result_size;
        free(result);
      } break;

      default:
        break;
    }
  }
  print_hex_array("EncodeData Inside", output, *bytes_written);
  return EIP712_OK;
}

int hash_struct(const TypedDataStruct_TypedDataNode *data_node, uint8_t *output) {
  if (data_node->type != TypedDataStruct_TypedDataNode_Eip712DataType_STRUCT)
    return EIP712_INVALID_DATA;
  size_t data_size = HASH_SIZE + (data_node->children_count + 1) * HASH_SIZE;
  size_t used_size = 0;
  uint8_t *data    = malloc(data_size);
  memcpy(data, data_node->type_hash->bytes, data_node->type_hash->size);
  int status =
      encode_data(data_node, data + data_node->type_hash->size, (data_size - data_node->type_hash->size), &used_size);
  printf("status = %d\n", status);
  print_hex_array("Pre Hash", data, data_size);
  keccak_256(data, used_size, output);
  free(data);
  return status;
}
#endif /* EIP712_UTILS */